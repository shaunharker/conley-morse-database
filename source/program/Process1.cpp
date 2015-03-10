/*
 *  Process1.cpp
 */

#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <exception>
#include <vector>

#include "boost/shared_ptr.hpp"
#include "bProcesoost/thread.hpp"
#include "boost/chrono/chrono_io.hpp"

#include "database/program/Configuration.h"
#include "database/program/Process1.h"
#include "database/program/jobs/Clutching_Graph_Job.h"
#include "database/structures/Database.h"

#include "Model.h"

void Process1::command_line ( int argcin, char * argvin [] ) {
  argc = argcin;
  argv = argvin;
  model . initialize ( argc, argv );
}

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void Process1::initialize ( void ) {
  // Load configuration
  std::cout << "Process1::initialize. Attempting to load configuration.\n";
  config . loadFromFile ( argv[1] );
  std::cout << "Process1::initialize. Loaded configuration.\n";
  
  // Create save file
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.p1" );
  std::ofstream save_file ( (filestring + appendstring) . c_str () );
  save_file . close ();

  // Checkpoint/Progress variable initialization
  time_of_last_checkpoint_ = clock();
  time_of_last_progress_report_ = clock ();
  progress_bar_ = 0;
  num_jobs_ = 0;
  num_jobs_sent_ = 0;
  checkpoint_timer_running_ = false;

  // output variables
  output_flush_begin_ = 0;
  output_flush_end_ = 0;
  output_flush_size_ = 0;
  output_flush_pos_ = 0;

  // Construct Parameter Space
  std::cout << "Process1::initialize. Obtaining parameter space.\n";
  parameter_space_ = model . parameterSpace ();
  if ( not parameter_space_ ) {
    throw std::logic_error ( "Unable to obtain parameter space from model.\n");
  }
  std::cout << "Process1::initialize. Serializing parameter space.\n";
  database . insert ( parameter_space_ );

  // Count number of patches
  std::cout << "Process1::initialize. Iterating through patches.\n";
  size_t num_calc = 0;
  while ( 1 ) {
    boost::shared_ptr<ParameterPatch> p = parameter_space_ -> patch ();
    if ( not p ) {
      throw std::logic_error("Error. Process1::initialize. Unable to obtain patch from parameter space.\n");
    }
    if ( p -> empty () ) break;
    ++ num_jobs_;
    num_calc += p -> vertices . size ();
  }
  
  // Output to the user about the upcoming database calculation
  std::cout << "Process1 initialized. \n";
  std::cout << "  There are " << parameter_space_ -> size () << " parameters.\n";
  std::cout << "  There are " << num_jobs_ << " jobs.\n";
  std::cout << "  Within those jobs, there are " << num_calc << " parameter box calculations to be done.\n";
  std::cout << "  On average, a single parameter box calculation will be done " << 
    (double) num_calc  / (double) parameter_space_ -> size ()  << " times.\n";
}

/* * * * * * * * * * * */
/* prepare definition  */
/* * * * * * * * * * * */
int Process1::prepare ( Message & job ) {
  using namespace chomp;
  
  if ( progress_bar_ == num_jobs_ ) return 1; // nothing to compute

  if ( not checkpoint_timer_running_  || num_jobs_sent_ == num_jobs_ ) {
    job << (uint64_t) 0; // Checkpoint timer job
    checkpoint_timer_running_ = true;
    return 0;
  } else {
    job << (uint64_t) 1; // Conley Job
  }

  // Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;
  std::cout << "Process1::prepare: Preparing job " << job_number << "\n";
  
  // Obtain patch 
  boost::shared_ptr<ParameterPatch> patch = parameter_space_ -> patch ();
  
  // prepare the message with the job to be sent
  job << job_number;
  job << patch;
  job << config.PHASE_SUBDIV_INIT;
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0;
}
 
/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */

struct ClutchingJobWorkThread {
  Message * result;
  const Message * job;
  bool * computed;
  const Model * model;
  ClutchingJobWorkThread
            ( Message * result, 
              const Message * job,
              bool * computed,
              const Model * model ) 
  : result(result), job(job), computed(computed), model(model) {}

  void operator () ( void ) {
    try {
      Clutching_Graph_Job ( result , *job, *model );
      *computed = true;
    } catch ( std::logic_error& e ) {
      throw e;
    } catch ( ... /* boost::thread_interrupted& */) {
      *computed = false;
    }
  }
};


void Process1::work_job_helper ( Message & result, const Message & job ) const {
  std::cout << "Process1::work. Normal Job detected.\n";
  result << (uint64_t) 1;
  // Read Job Number
  size_t job_number;
  job >> job_number;
  result << job_number;
  // Perform work
  bool computed;
  ClutchingJobWorkThread cj ( &result, &job, &computed, &model );
  boost::thread t(cj);
  if ( not t . try_join_for ( boost::chrono::seconds( 3600 ) ) ) {
    t.interrupt();
    t.join();
  }
  if ( not computed ) {
    result << Database ();
  }
}

void Process1::work_job ( Message & result, const Message & job ) const {
  Message job_copy ( job );
  Message sub_result;
  work_job_helper ( sub_result, job_copy );

  // Read Job Number
  size_t job_number;
  boost::shared_ptr<ParameterPatch> patch;
  Database database;
  job >> job_number;
  job >> patch;
  sub_result >> database;

  // Create result

  // First create a data structure to tell us which clutching relations
  // give us identity between Morse graphs
  std::unordered_set <std::pair<uint64_t, uint64_t>> identity_pairs;
  {
    std::unordered_map < uint64_t, uint64_t > param_to_mgr;
    for ( const ParameterRecord & pr : database . parameter_records () ) {
      uint64_t morsegraph_index = pr . morsegraph_index;
      param_to_mgr [ pr . parameter_index ] = (int64_t) morsegraph_index;
    }

    for ( const ClutchingRecord & cr : database . clutch_records () ) {
      uint64_t pi1 = cr . parameter_index_1;
      uint64_t pi2 = cr . parameter_index_2;
      if ( param_to_mgr . count ( pi1 ) == 0 ||
           param_to_mgr . count ( pi2 ) == 0 ) {
        continue;
      }

      if ( is_identity ( morsegraph_data_ [ param_to_mgr [ pi1 ] ], 
                         morsegraph_data_ [ param_to_mgr [ pi2 ] ],
                         bg_data_ [ cr . bg_index ] ) ) {
        identity_pairs . insert ( std::make_pair ( cr . pi1, cr . pi2 ) )
      }
    }
  }
  // Now create a vector of bits in order of adjacencies given in "patch"
  // so we report a 0 when there is not an identity map and 1 when there is
  std::vector<bool> bits;
  for ( std::pair<uint64_t, uint64_t> pair : patch -> edges ) {
    if ( identity_pairs . count ( pair ) ) {
      bits . push_back ( 1 );
    } else {
      bits . push_back ( 0 );
    }
  }

  // Return the result
  result << job_number;
  result << bits;
}

void Process1::work_timer ( Message & result, const Message & job ) const {
  // Checkpoint timer job
  std::cout << "Process1::work. Checkpoint timer job detected.\n";
  boost::this_thread::sleep( boost::posix_time::seconds(15) );
  result << (uint64_t) 0;
}

void Process1::work ( Message & result, const Message & job ) const {
  // Read Job type
  uint64_t job_type;
  job >> job_type;
  switch ( job_type ) {
  case 0:
    work_timer ( result, job );
    break;
  case 1:
    work_job ( result, job );
    break;
  }
  std::cout << "Process1::work. Job complete.\n";
}

/* * * * * * * * * * */
/* accept definition */
/* * * * * * * * * * */
void Process1::accept(const Message &result) {
  clock_t current_time = clock ();
  /// Read the results from the result message
  uint64_t result_type;
  result >> result_type;
  if ( result_type == 0 ) {
    // Checkpoint Timer finished.
    checkpoint_timer_running_ = false;
    // Save a checkpoint
    if ( (float)(current_time - time_of_last_checkpoint_ ) 
      / (float)CLOCKS_PER_SEC > 3600.0f ) {
      checkpoint ();
      progressReport ();
    }
  } else {
    // Accepting result of normal job.
    // Read the results from the result message
    size_t job_number;
    std::vector<bool> job_data;

    result >> job_number;
    result >> job_data;

    // Handle the results
    output_data_ [ job_number ] = job_data;

    // Update how many results can be contiguously be flushed to disk
    while ( output_data_ . count ( output_flush_end_ ) ) {
      output_flush_size_ += output_data_ [ output_flush_end_ ] . size ();
      ++ output_flush_end_;
    }
    
    // Check if ready to flush
    if ( output_flush_size_ > 33554432 ) { // 4 megabytes
      checkpoint ();
    }


    ++ progress_bar_;
    std::cout << "Process1::read: Received result " 
      << job_number << "\n";
  }

  if ( (float)(current_time - time_of_last_progress_report_ ) / (float)CLOCKS_PER_SEC > 1.0f ) {
    progressReport ();
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void Process1::finalize ( void ) {
  std::cout << "Process1::finalize \n";
  checkpoint ();
}

void Process1::checkpoint ( void ) {
  std::cout << "Process1::checkpoint\n";

  // Prepare write buffer
  size_t begin_bit_position = output_flush_pos_;
  size_t end_bit_position = output_flush_pos_ + output_flush_size_;
  size_t begin_byte_position = begin_bit_position / 8;
  size_t end_byte_position = end_bit_position / 8;
  
  output_flush_pos_ = end_bit_position;

  std::vector<char> write_buffer ( end_byte_position - begin_byte_position + 1, 0 );

  // Flush data into write buffer
  size_t write_byte = begin_byte_position;
  char write_bit = 1 << (begin_bit_position % 8);
  while ( output_flush_begin_ != output_flush_end_ ) {
    std::vector<bool> const& data = output_data_ [ output_flush_begin_ ];
    for ( bool bit : data ) {
      if ( bit ) {
        write_buffer [ write_byte ] |= write_bit;
      }
      write_bit <<= 1;
      if ( write_bit == 0 ) { 
        write_bit = 1;
        write_byte ++;
      }
    }
    ++ output_flush_begin_;
  }
  output_flush_size_ = 0;

  // Open the file
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.p1" );
  std::fstream outfile;
  outfile . open ( (filestring + appendstring) . c_str (), 
                   std::ios::out | std::ios::in | std::ios::binary);
 
  // Read last byte and include it in flush_buffer (so it isn't overwritten)
  outfile . seekg ( begin_byte_position );
  char c;
  outfile >> c;
  write_buffer [ 0 ] |= c;

  // Write buffer to file
  outfile . seekp ( begin_byte_position );
  outfile . write ( &write_buffer[0], write_buffer . size () );

  // Close file
  outfile . close ();
  time_of_last_checkpoint_ = clock();
}

void Process1::progressReport ( void ) {
  std::cout << "Process1::progressReport. " << progress_bar_ << " / " << num_jobs_ << "\n";
  std::ofstream progress_file ( "progress.txt" );
  progress_file << "Process1 Progress: " << progress_bar_ << " / " << num_jobs_ << "\n";
  progress_file . close ();
  time_of_last_progress_report_ = clock();
}
