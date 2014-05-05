/*
 *  ConleyProcess.cpp
 */

//#define SNF_DEBUG
#include <iostream>
#include <fstream>
#include <limits>
#include <ctime>

#include "boost/foreach.hpp"

#include <boost/thread.hpp>
#include <boost/chrono/chrono_io.hpp>

#include "database/structures/Database.h"
#include "database/program/Configuration.h"
#include "database/program/ConleyProcess.h"
#include "database/program/jobs/Conley_Index_Job.h"
#include "database/structures/UnionFind.hpp"

#ifdef USE_SDSL
#include "database/structures/SuccinctGrid.h"
#endif
#include "database/structures/PointerGrid.h"
#include "database/structures/UniformGrid.h"
#include "database/structures/EdgeGrid.h"
 
#include "chomp/Rect.h"

#include "ModelMap.h"

#ifndef GRIDCHOICE
#define GRIDCHOICE PointerGrid
#endif

/*  in Conley_Morse_Database.cpp
#include <boost/serialization/export.hpp>
#ifdef USE_SDSL
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
*/

void ConleyProcess::command_line ( int argcin, char * argvin [] ) {
  argc = argcin;
  argv = argvin;
  model . initialize ( argc, argv );
}

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void ConleyProcess::initialize ( void ) {
  using namespace chomp;
  time_of_last_progress_report_ = clock ();

  // LOAD DATABASE
  num_jobs_sent_ = 0;
  std::cout << "ConleyProcess::initialize ()\n";
  
  std::cout << "Attempting to load configuration...\n";
  config . loadFromFile ( argv[1] );
  std::cout << "Loaded configuration.\n";
  
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.mdb" );
  database . load ( (filestring + appendstring) . c_str () );

  num_incc_ = database . INCC_Records () . size ();
  finished_ . resize ( num_incc_, false );
  attempts_ . resize ( num_incc_, 0 );
  num_finished_ = 0;
  current_incc_ = 0;

  checkpoint_timer_running_ = false;
}

/* * * * * * * * * * */
/* write definition  */
/* * * * * * * * * * */
int ConleyProcess::prepare ( Message & job ) {
  using namespace chomp;

  if ( num_finished_ == num_incc_ ) return 1; // Code 1: No more jobs.

  if ( not checkpoint_timer_running_ ) {
    job << (uint64_t) 0; // Checkpoint timer job
    checkpoint_timer_running_ = true;
    return 0;
  } else {
    job << (uint64_t) 1; // Conley Job
  }

  std::cout << " ConleyProcess::prepare\n";
  std::cout << " num_jobs_sent_ = " << num_jobs_sent_ << "\n";
  
  while ( finished_ [ current_incc_ ] ) {
    ++ current_incc_;
    if ( current_incc_ == num_incc_ ) current_incc_ = 0;
  }

  size_t attempt = attempts_ [ current_incc_ ] ++;
  uint64_t incc = current_incc_;
  uint64_t pi;
  uint64_t ms;

  const INCC_Record & incc_record = database . INCC_Records () [ incc ];
  
  if ( attempt < incc_record . smallest_reps . size () ) {
    // Find a small representative
    std::set<std::pair<uint64_t, std::pair<uint64_t, uint64_t> > >::iterator it;
    it = incc_record . smallest_reps . begin ();
    std::advance ( it, attempt );
    std::pair < uint64_t, uint64_t > pair = it -> second;
    pi = pair . first;
    ms = pair . second;
  } else {
    // Find a random representative
    uint64_t incc_size = database . incc_sizes () [ incc ];
    bool chose_representative = false;
    while ( not chose_representative ) {
      BOOST_FOREACH ( uint64_t inccp, incc_record . inccp_indices ) {
        const INCCP_Record & inccp_record = 
         database . INCCP_Records () [ inccp ];
        uint64_t cs = inccp_record . cs_index;
        if ( database . csData () [ cs ] . vertices . size () != 1 ) continue;
        const MGCCP_Record & mgccp_record = 
         database . MGCCP_Records () [ inccp_record . mgccp_index ];
        uint64_t mgccp_size = mgccp_record . parameter_indices . size ();
        if ( rand () % incc_size >= mgccp_size ) continue;
        
        pi = mgccp_record . parameter_indices [ rand () % mgccp_size ];
        ms =  database . csData () [ cs ] . vertices [ 0 ];
        chose_representative = true;
        break;
      }
    }
  }

  size_t job_number = num_jobs_sent_;
  boost::shared_ptr<Parameter> parameter = 
    database . parameter_space () . parameter ( pi );
  job << job_number;
  job << incc;
  job << parameter;
  job << ms;
  job << config.PHASE_SUBDIV_INIT;
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;
  job << model;
  //job << config.PHASE_BOUNDS;
  //job << config.PHASE_PERIODIC;

  std::cout << "Preparing conley job " << job_number 
            << " with parameter = " << *parameter << "  and  ms = (" <<  ms << ")\n";
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0; // Code 0: Job was sent.
}

/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void ConleyProcess::work ( Message & result, const Message & job ) const {
  uint64_t job_type;
  job >> job_type;
  switch ( job_type ) {
    case 0:
      // Checkpoint timer job
      std::cout << "ConleyProcess::work. Checkpoint timer job detected.\n";
      boost::this_thread::sleep( boost::posix_time::seconds(60) );
      result << (uint64_t) 0;
      break;
    case 1:
      std::cout << "ConleyProcess::work. Normal job detected.\n";

      result << (uint64_t) 1;
      Conley_Index_Job < GRIDCHOICE > ( &result , job );
      break;
  }
  std::cout << "ConleyProcess::work. Job complete.\n";

}

/* * * * * * * * * */
/* read definition */
/* * * * * * * * * */
void ConleyProcess::accept (const Message &result) {
  clock_t current_time = clock ();
  /// Read the results from the result message
  uint64_t result_type;
  result >> result_type;
  if ( result_type == 0 ) {
    // Checkpoint Timer finished.
    checkpoint_timer_running_ = false;
    if ( (float)(current_time - time_of_last_checkpoint_ ) 
      / (float)CLOCKS_PER_SEC > 3600.0f ) {
      checkpoint ();
      progressReport ();
    }
  } else {
  // Accepting result of normal job.
    size_t job_number;
    int error_code;
    uint64_t incc;
    CI_Data job_result;
    result >> job_number;
    result >> error_code;
    result >> incc;
    result >> job_result;

    if ( error_code == 0 && not finished_[incc] ) { 
      database . insert ( incc, job_result );
      finished_ [ incc ] = true;
      ++ num_finished_;
    } else if ( error_code == 1 && not finished_[incc] ) {
      // partial answer, do not mark as finished but include result
      database . insert ( incc, job_result );
    }
    std::cout << "ConleyProcess::accept: Received result " 
            << job_number << "\n";
  }

  if ( (float)(current_time - time_of_last_progress_report_ ) / (float)CLOCKS_PER_SEC > 1.0f ) {
    progressReport ();
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void ConleyProcess::finalize ( void ) {
  std::cout << "ConleyProcess::finalize ()\n";
  checkpoint ();
}


void ConleyProcess::checkpoint ( void ) {
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.cmdb" );
  database . save ( (filestring + appendstring) . c_str () );
  time_of_last_checkpoint_ = clock ();
}

void ConleyProcess::progressReport ( void ) {
  std::ofstream progress_file ( "conleyprogress.txt" );
  progress_file << "Conley Process Progress: " << num_finished_ << " / " << num_incc_ << "\n";
  progress_file << "INCCs which have not been computed yet:\n";
  for ( uint64_t incc = 0; incc < num_incc_; ++ incc ) {
    if ( not finished_ [ incc ] ) progress_file << incc << " " ;
  }
  progress_file << "\n";
  progress_file . close ();
  time_of_last_progress_report_ = clock ();
}
#if 0

// DETERMINE CONLEY JOBS
  // we only compute conley index for INCC records that have a representative
  // with a single morse set. This requires searching the representatives.
  uint64_t num_incc = database . INCC_Records () . size ();
  for ( uint64_t incc = 0; incc < num_incc; ++ incc ) {
    const INCC_Record & incc_record = database . INCC_Records () [ incc ];
    BOOST_FOREACH ( uint64_t inccp, incc_record . inccp_indices ) {
      const INCCP_Record & inccp_record = database . INCCP_Records () [ inccp ];
      uint64_t cs = inccp_record . cs_index;
      if ( database . csData () [ cs ] . vertices . size () == 1 ) {
        const MGCCP_Record & mgccp_record = database . MGCCP_Records () [ inccp_record . mgccp_index ];
        Grid::GridElement ge = mgccp_record . grid_elements [ 0 ];
        uint64_t vertex =  database . csData () [ cs ] . vertices [ 0 ];

        // debug

        if ((int)vertex>=database.dagData()[database.morsegraphData()[mgccp_record.morsegraph_index].dag_index].num_vertices) {
          std::cout << "Invalid database.\n";
          abort ();
        }
        if ( database . pb_to_mgccp () [ ge ] != inccp_record . mgccp_index ) {
          std::cout << " mgccp = " << inccp_record . mgccp_index << " ge = " << ge << "\n";
          std::cout << " mgccp from ge = "  << database . pb_to_mgccp () [ ge ] << "\n";
          abort ();
        }
        // end debug
        conley_work_items . push_back ( std::make_pair ( incc, std::make_pair ( ge, vertex ) ) );
        break;
      }
    }
  }

  
  num_jobs_ = conley_work_items . size ();
  
  std::cout << "Writing job list to conleystage.txt\n";
  std::ofstream outfile ( "conleystage.txt" );
  outfile << "Number of conley jobs = " << num_jobs_ << "\n";
  
  typedef std::numeric_limits< double > dbl;
  outfile.precision(dbl::digits10);
  outfile << std::scientific;

  for ( unsigned int job_number = 0; job_number < num_jobs_; ++ job_number ) {
    uint64_t incc = conley_work_items [ job_number ] . first;
    progress_detail . insert ( incc );
    uint64_t pb_id = conley_work_items [ job_number ] . second . first;
    uint64_t ms = conley_work_items [ job_number ] . second . second;

    RectGeo GD = * boost::dynamic_pointer_cast < RectGeo > 
                 ( database . parameter_space () . geometry ( pb_id ) );
    outfile << "Job " << job_number << ": INCC = " << incc << " PB = " 
            << pb_id << ", MS = " << ms << ", geo = " << GD << "\n";
    std::cout << "Job " << job_number << ": INCC = " << incc << " PB = " 
            << pb_id << ", MS = " << ms << ", geo = " << GD << "\n";
  }
  std::cout << "Finished writing job list.\n";
  outfile . close ();

#endif
