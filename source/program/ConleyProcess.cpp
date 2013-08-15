/*
 *  ConleyProcess.cpp
 */

//#define SNF_DEBUG
#include <iostream>
#include <fstream>
#include <limits>
#include <ctime>

#include "boost/foreach.hpp"

#include "database/program/Configuration.h"
#include "database/program/ConleyProcess.h"
#include "database/program/jobs/Conley_Index_Job.h"
#include "database/structures/UnionFind.hpp"

#ifdef USE_SUCCINCT
#include "database/structures/SuccinctGrid.h"
#endif
#include "database/structures/PointerGrid.h"

#include "chomp/Rect.h"

#include "ModelMap.h"

#ifndef GRIDCHOICE
#define GRIDCHOICE PointerGrid
#endif

/*  in Conley_Morse_Database.cpp
#include <boost/serialization/export.hpp>
#ifdef HAVE_SUCCINCT
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
*/

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void ConleyProcess::initialize ( void ) {
  using namespace chomp;
  time_of_last_checkpoint = clock ();
  time_of_last_progress = clock ();
  progress_bar = 0;

  // LOAD DATABASE
  num_jobs_sent_ = 0;
  std::cout << "ConleyProcess::initialize ()\n";
  
  std::cout << "Attempting to load configuration...\n";
  config . loadFromFile ( argv[1] );
  std::cout << "Loaded configuration.\n";
  
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.mdb" );
  database . load ( (filestring + appendstring) . c_str () );
    
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
        if ( (int)vertex >= database . dagData () [ mgccp_record . dag_index ] . num_vertices ) {
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

  	Rect GD = database . parameter_space () . geometry ( pb_id );
    outfile << "Job " << job_number << ": INCC = " << incc << " PB = " 
            << pb_id << ", MS = " << ms << ", geo = " << GD << "\n";
    std::cout << "Job " << job_number << ": INCC = " << incc << " PB = " 
            << pb_id << ", MS = " << ms << ", geo = " << GD << "\n";
  }
  std::cout << "Finished writing job list.\n";
  outfile . close ();
}

/* * * * * * * * * * */
/* write definition  */
/* * * * * * * * * * */
int ConleyProcess::prepare ( Message & job ) {
  using namespace chomp;
  // All jobs have already been sent.
  std::cout << " ConleyProcess::prepare\n";
  std::cout << " num_jobs_sent_ = " << num_jobs_sent_ << "\n";
  std::cout << " num_jobs_ = " << num_jobs_ << "\n";
  
  if (num_jobs_sent_ == num_jobs_) return 1; // Code 1: No more jobs.
  
  size_t job_number = num_jobs_sent_;

  uint64_t incc = conley_work_items [ job_number ] . first;
  uint64_t pb_id = conley_work_items [ job_number ] . second . first;
  uint64_t ms = conley_work_items [ job_number ] . second . second;
  Rect GD = database . parameter_space () . geometry ( pb_id );

  // TEMPORARY FIX -- REMOVE, WILL CAUSE ERRORS
  double tol = 1e-8;
  for ( int d = 0; d < database . parameter_space () . dimension (); ++ d ) {
    GD . lower_bounds [ d ] -= tol;
    GD . upper_bounds [ d ] += tol;
  }
  // END DEBUG

  job << job_number;
  job << incc;
  job << GD;
  job << ms;
    job << config.PHASE_SUBDIV_INIT;
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;
  job << config.PHASE_BOUNDS;
  job << config.PHASE_PERIODIC;

  std::cout << "Preparing conley job " << job_number 
            << " with GD = " << GD << "  and  ms = (" <<  ms << ")\n";
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0; // Code 0: Job was sent.
}

/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void ConleyProcess::work ( Message & result, const Message & job ) const {
  Conley_Index_Job < GRIDCHOICE > ( &result , job );
}

/* * * * * * * * * */
/* read definition */
/* * * * * * * * * */
void ConleyProcess::accept (const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  uint64_t incc;
  CI_Data job_result;
  result >> job_number;
  result >> incc;
  result >> job_result;
  database . insert ( incc, job_result );
  std::cout << "ConleyProcess::accept: Received result " 
            << job_number << "\n";
  ++ progress_bar;
  progress_detail . erase ( incc );
  clock_t time = clock ();
  if ( (float)(time - time_of_last_progress ) / (float)CLOCKS_PER_SEC > 1.0f ) {
    std::ofstream progress_file ( "conleyprogress.txt" );
    progress_file << "Conley Process Progress: " << progress_bar << " / " << num_jobs_ << "\n";
    BOOST_FOREACH ( uint64_t incc, progress_detail ) {
      progress_file << incc << " " ;
    }
    progress_file << "\n";
    progress_file . close ();
    time_of_last_progress = time;
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void ConleyProcess::finalize ( void ) {
  std::cout << "ConleyProcess::finalize ()\n";
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.cmdb" );
  database . save ( (filestring + appendstring) . c_str () );
}
