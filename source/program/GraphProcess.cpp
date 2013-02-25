/*
 *  GraphProcess.cpp
 */

#include "database/program/GraphProcess.h"
#include "database/program/ComputeGraph.h"

#include "ModelMap.h"
#include "chomp/Rect.h"

#define ATONCE 1000


/* * * * * * * * * * * * * */
/* command_line definition */
/* * * * * * * * * * * * * */
void GraphProcess::command_line ( int argc, char * argv [] ) {
  toplex_ . load ( "grid.txt" );
  evals . load ( "mapevals.txt" );
  f_ = new ModelMap ( evals . parameter () );
}

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void GraphProcess::initialize ( void ) {
  for ( size_t startjob = 0; startjob < evals . size (); startjob += ATONCE ) {
    Message Job;
    size_t endjob = std::min(evals . size (), startjob + ATONCE);
    size_t number_in_message = endjob - startjob;
    Job << number_in_message;
    for ( size_t j = startjob; j < endjob; ++ j ) {
      Job << evals . arg ( j );
    }
    JOBS_TO_SEND . push ( Job );
  }
}
 
/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void GraphProcess::work ( Message & result, const Message & job ) const {
  chomp::Rect parameter;
  size_t number_in_message;
  job >> number_in_message;
  result << number_in_message;
  ModelMap & f = *f_;
  for ( size_t i = 0; i < number_in_message; ++ i ) {
    size_t j;
    chomp::Toplex::value_type argument;
    std::vector < chomp::Toplex::value_type > value;
    chomp::Rect argument_geo;
    
    // Retrieve argument
    job >> argument;
    
    // If interior node, just give 2 children
    // cii stands for child insert iterator
    std::insert_iterator < CellContainer > cii ( value, value . begin () );
    toplex_ . children ( cii, argument );
    // If leaf, value is still empty. Apply the map.
    if ( value . empty () ) {
      argument_geo = toplex_ . geometry ( argument );
      std::insert_iterator < CellContainer > ii ( value, value . begin () );
      toplex_ . cover ( ii, argument_geo ); // here is the work
    }

    // Emit (argument, image) pair
    result << argument;
    result << value 
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void GraphProcess::finalize ( void ) {
  while ( not RESULTS_RECEIVED . empty () ) {
    Message result = RESULTS_RECEIVED . top ();
    RESULTS_RECEIVED . pop ();
    size_t number_in_message;
    result >> number_in_message;
    for ( size_t i = 0; i < number_in_message; ++ i ) {
      chomp::Toplex::value_type argument;
      std::vector < chomp::Toplex::value_type > value;
      result >> argument;
      result >> value;
      evals . val ( argument ) = value;
    }
  }
  evals . save ( "mapevals.txt" );
}

/* * * * * * * * * */
/* main definition */
/* * * * * * * * * */
int main ( int argc, char * argv [] ) {
  delegator::Start ();
  delegator::Run < GraphProcess > (argc, argv);
  delegator::Stop ();
  return 0;
}

