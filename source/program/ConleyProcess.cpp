/*
 *  ConleyProcess.cpp
 */

#include <iostream>
#include <fstream>

#include "boost/foreach.hpp"

#include "program/Configuration.h"
#include "program/ConleyProcess.h"
#include "program/jobs/Conley_Index_Job.h"
#include "structures/UnionFind.hpp"

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void ConleyProcess::initialize ( void ) {
  num_jobs_sent_ = 0;
  std::cout << "ConleyProcess::initialize ()\n";
  database . load ( "database.cmdb" );
  
    // Initialize parameter space bounds
  param_toplex . initialize (param_bounds);   /// Parameter space toplex
  for ( int i = 0; i < PARAM_SUBDIVISIONS; ++i ) { 
  	param_toplex . subdivide (); // subdivide every top cell
  }
  
  // Goal is to create disjoint set data structure of equivalent morse sets
  typedef std::pair<int, int> intpair;
  typedef std::pair<int, int> ms_id; // morse_set id
  UnionFind < ms_id > classes;
  
  /* Process Parameter Box Records */
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    for ( int i = 0; i < record . num_morse_sets_ ; ++ i ) {
      classes . Add ( ms_id ( record . id_, i ) );
    }
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
  
  std::ofstream outfile ( "conleystage.txt" );
  outfile << "Number of conley jobs = " << num_jobs_ << "\n";
  
  for ( unsigned int job_number = 0; job_number < num_jobs_; ++ job_number ) {
  	Toplex::Top_Cell cell = conley_work_items [ job_number ] . first;
  	Geometric_Description GD = param_toplex . geometry (param_toplex . find (cell));
    outfile << "job number " << job_number << ", cell = " << cell << ", geo = " << GD << "\n";

  }
  outfile . close ();
}

/* * * * * * * * * * */
/* write definition  */
/* * * * * * * * * * */
int ConleyProcess::write ( Message & job ) {
  
  // All jobs have already been sent.
  if (num_jobs_sent_ == num_jobs_) return 1; // Code 1: No more jobs.
  
  size_t job_number = num_jobs_sent_;
  Toplex::Top_Cell cell = conley_work_items [ job_number ] . first;
  Geometric_Description GD = param_toplex . geometry (param_toplex . find (cell));

  job << job_number;
  job << GD;
  job << conley_work_items [ job_number ];
  
  //std::cout << "Preparing conley job " << job_number 
  //          << " with GD = " << GD << "\n";
  
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0; // Code 0: Job was sent.
}

/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void ConleyProcess::work ( Message & result, const Message & job ) const {
  Conley_Index_Job < Toplex, Toplex, Conley_Index_t > ( &result , job ); 
}

/* * * * * * * * * */
/* read definition */
/* * * * * * * * * */
void ConleyProcess::read(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "ConleyProcess::read: Received result " 
            << job_number << "\n";
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void ConleyProcess::finalize ( void ) {
  std::cout << "ConleyProcess::finalize ()\n";
  database . save ( "database.cmdb" );
}
