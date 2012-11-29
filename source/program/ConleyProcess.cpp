/*
 *  ConleyProcess.cpp
 */

#include <iostream>
#include <fstream>

#include "boost/foreach.hpp"

#include "database/program/Configuration.h"
#include "database/program/ConleyProcess.h"
#include "database/program/jobs/Conley_Index_Job.h"
#include "database/structures/UnionFind.hpp"

#include "chomp/Rect.h"

#include "ModelMap.h"

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void ConleyProcess::initialize ( void ) {
  using namespace chomp;

  num_jobs_sent_ = 0;
  std::cout << "ConleyProcess::initialize ()\n";
  
  std::cout << "Attempting to load configuration...\n";
  config . loadFromFile ( argv[1] );
  std::cout << "Loaded configuration.\n";
  
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.mdb" );
  database . load ( (filestring + appendstring) . c_str () );
    
  database . conley_records () . clear ();
  std::cout << "Loaded database.\n";

  // Goal is to create disjoint set data structure of equivalent morse sets
  typedef std::pair<int, int> intpair;
  typedef std::pair<int, int> ms_id; // morse_set id
  UnionFind < ms_id > classes;
  
  /* Process Parameter Box Records */
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    for ( int i = 0; i < record . num_morse_sets_ ; ++ i ) {
      classes . Add ( ms_id ( record . id_, i ) );
    }
    param_boxes [ record . id_ ] . lower_bounds = record . ge_ . lower_bounds_;
    param_boxes [ record . id_ ] . upper_bounds = record . ge_ . upper_bounds_;    
  }
  
  /* Process all Clutching Records */
  BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
    std::map < int, int > first;
    std::map < int, int > second;
    BOOST_FOREACH ( const intpair & line, record . clutch_ ) {
      if ( first . find ( line . first ) == first . end () ) 
        first [ line . first ] = 0;
      if ( second . find ( line . second ) == second . end () ) 
        second [ line . second ] = 0;
      ++ first [ line . first ];
      ++ second [ line . second ];
    }
    BOOST_FOREACH ( const intpair & line, record . clutch_ ) {
      if ( first [ line . first ] == 1 && second [ line . second ] == 1 ) {
        classes . Union ( ms_id ( record . id1_, line . first ), 
                          ms_id ( record . id2_, line . second ) );
      }
    }
  }
  
  
  // Now the union-find structure is prepared.
  /* TODO: use a small representative.
  std::map < ms_id, ms_id > smallest_reps;
  BOOST_FOREACH ( const ms_id & element, classes . Elements () ) {
    ms_id rep = classes . Representative ( element );
    if ( smallest_reps . find ( rep ) == smallest_reps . end () ) 
      smallest_reps [ rep ] = element;
    ms_id small = smallest_reps [ rep ];
    
  }
  */
  std::set < ms_id > work_items;
  BOOST_FOREACH ( const ms_id & element, classes . Elements () ) {
    ms_id rep = classes . Representative ( element );
    if ( work_items . insert ( rep ) . second == true )
      conley_work_items . push_back ( rep );
  }
  
  num_jobs_ = conley_work_items . size ();
  
  std::cout << "Writing job list to conleystage.txt\n";
  std::ofstream outfile ( "conleystage.txt" );
  //std::cout << "Number of conley jobs = " << num_jobs_ << "\n";
  //std::cout << "conley_work_items_ . size () = " << conley_work_items . size () << "\n";
  outfile << "Number of conley jobs = " << num_jobs_ << "\n";
  
  for ( unsigned int job_number = 0; job_number < num_jobs_; ++ job_number ) {
    //std::cout << "fetch cell from work item\n";
    int pb_id = conley_work_items [ job_number ] . first;
    std::cout << "Found " << pb_id << ", now we determine corresponding prism\n";
  	Rect GD = param_boxes [ pb_id ]; 
    outfile << "job number " << job_number << ", pb_id = " << pb_id << ", geo = " << GD << "\n";
    std::cout << "job number " << job_number << ", pb_id = " << pb_id << ", geo = " << GD << "\n";

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
  int pb_id = conley_work_items [ job_number ] . first;
  Rect GD = param_boxes [ pb_id ];

  job << job_number;
  job << GD;
  job << conley_work_items [ job_number ];
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;
  job << config.PHASE_BOUNDS;
  job << config.PERIODIC;

  std::cout << "Preparing conley job " << job_number 
            << " with GD = " << GD << "  and  ms_id = (" << conley_work_items [ job_number ] . first << ", " << conley_work_items [ job_number ] . second << ")\n";
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0; // Code 0: Job was sent.
}

/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void ConleyProcess::work ( Message & result, const Message & job ) const {
  Conley_Index_Job < chomp::Toplex, chomp::Toplex > ( &result , job ); 
  // debug
  /*
  Database job_database;
  size_t job_number;
  job >> job_number;
  result << job_number;
  result << job_database;
   */
}

/* * * * * * * * * */
/* read definition */
/* * * * * * * * * */
void ConleyProcess::accept (const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "ConleyProcess::accept: Received result " 
            << job_number << "\n";
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void ConleyProcess::finalize ( void ) {
  std::cout << "ConleyProcess::finalize ()\n";
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.cmdb" );
  database . save ( (filestring + appendstring) . c_str () );
    
    std::cout << "DEBUG...\n";
    Database debugtest;
    debugtest . load ( (filestring + appendstring) . c_str () );
    std::cout << typeid ( chomp::Ring ).name() << "\n";
  
}
