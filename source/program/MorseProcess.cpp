/*
 *  MorseProcess.cpp
 */

#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <exception>
#include <vector>

#include <memory>
#include "boost/thread.hpp"
#include "boost/chrono/chrono_io.hpp"

#include "database/program/Configuration.h"
#include "database/program/MorseProcess.h"
#include "database/program/jobs/Clutching_Graph_Job.h"
#include "database/structures/Database.h"

#include "Model.h"

void MorseProcess::command_line ( int argcin, char * argvin [] ) {
  argc = argcin;
  argv = argvin;
  model . initialize ( argc, argv );
}

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void MorseProcess::initialize ( void ) {
  // Load configuration
  std::cout << "MorseProcess::initialize. Attempting to load configuration.\n";
  config . loadFromFile ( argv[1] );
  std::cout << "MorseProcess::initialize. Loaded configuration.\n";
  
  // Checkpoint/Progress variable initialization
  time_of_last_checkpoint_ = clock();
  time_of_last_progress_report_ = clock ();
  progress_bar_ = 0;
  num_jobs_ = 0;
  num_jobs_sent_ = 0;
  checkpoint_timer_running_ = false;

  // Construct Parameter Space
  std::cout << "MorseProcess::initialize. Obtaining parameter space.\n";
  parameter_space_ = model . parameterSpace ();
  if ( not parameter_space_ ) {
    throw std::logic_error ( "Unable to obtain parameter space from model.\n");
  }
  std::cout << "MorseProcess::initialize. Serializing parameter space.\n";
  database . insert ( parameter_space_ );

  // Count number of patches
  std::cout << "MorseProcess::initialize. Iterating through patches.\n";
  size_t num_calc = 0;
  while ( 1 ) {
    std::shared_ptr<ParameterPatch> p = parameter_space_ -> patch ();
    if ( not p ) {
      throw std::logic_error("Error. MorseProcess::initialize. Unable to obtain patch from parameter space.\n");
    }
    if ( p -> empty () ) break;
    ++ num_jobs_;
    num_calc += p -> vertices . size ();
  }
  
  // Output to the user about the upcoming database calculation
  std::cout << "MorseProcess initialized. \n";
  std::cout << "  There are " << parameter_space_ -> size () << " parameters.\n";
  std::cout << "  There are " << num_jobs_ << " jobs.\n";
  std::cout << "  Within those jobs, there are " << num_calc << " parameter box calculations to be done.\n";
  std::cout << "  On average, a single parameter box calculation will be done " << 
    (double) num_calc  / (double) parameter_space_ -> size ()  << " times.\n";
}

/* * * * * * * * * * * */
/* prepare definition  */
/* * * * * * * * * * * */
int MorseProcess::prepare ( Message & job ) {
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
  std::cout << "MorseProcess::prepare: Preparing job " << job_number << "\n";
  
  // Obtain patch 
  std::shared_ptr<ParameterPatch> patch = parameter_space_ -> patch ();
  
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
/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void MorseProcess::work ( Message & result, const Message & job ) const {
  // Read Job type
  uint64_t job_type;
  job >> job_type;
  switch ( job_type ) {
  case 0:
    // Checkpoint timer job
    std::cout << "MorseProcess::work. Checkpoint timer job detected.\n";
    boost::this_thread::sleep( boost::posix_time::seconds(15) );
    result << (uint64_t) 0;
    break;
  case 1:
    std::cout << "MorseProcess::work. Normal Job detected.\n";
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
    break;
  }
  std::cout << "MorseProcess::work. Job complete.\n";
}

/* * * * * * * * * * */
/* accept definition */
/* * * * * * * * * * */
void MorseProcess::accept(const Message &result) {
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
    Database job_database;
    result >> job_number;
    result >> job_database;
    // Merge the results
    database . merge ( job_database );
    ++ progress_bar_;
    std::cout << "MorseProcess::read: Received result " 
      << job_number << "\n";
  }

  if ( (float)(current_time - time_of_last_progress_report_ ) / (float)CLOCKS_PER_SEC > 1.0f ) {
    progressReport ();
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void MorseProcess::finalize ( void ) {
  std::cout << "MorseProcess::finalize \n";
  checkpoint ();
}

void MorseProcess::checkpoint ( void ) {
  std::cout << "MorseProcess::checkpoint\n";
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.raw" );
  database . save ( (filestring + appendstring) . c_str () );
  time_of_last_checkpoint_ = clock();
}

void MorseProcess::progressReport ( void ) {
  std::cout << "MorseProcess::progressReport. " << progress_bar_ << " / " << num_jobs_ << "\n";
  std::ofstream progress_file ( "progress.txt" );
  progress_file << "Morse Process Progress: " << progress_bar_ << " / " << num_jobs_ << "\n";
  progress_file . close ();
  time_of_last_progress_report_ = clock();
}
