/*
 *  GraphProcess.cpp
 */

#include "database/program/GraphProcess.h"
#include "database/program/ComputeGraph.h"

#include "ModelMap.h"
#include "chomp/Rect.h"

#define ATONCE 1000

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void GraphProcess::initialize ( void ) {
  evals . load ( "mapevals.txt" );
  for ( size_t startjob = 0; startjob < evals . size (); startjob += ATONCE ) {
    Message Job;
    Job << evals . parameter ();
    size_t endjob = std::min(evals . size (), startjob + ATONCE);
    size_t number_in_message = endjob - startjob;
    Job << number_in_message;
    for ( size_t j = startjob; j < endjob; ++ j ) {
      Job << j;
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
  job >> parameter;
  size_t number_in_message;
  job >> number_in_message;
  result << number_in_message;
  ModelMap f ( parameter );
  for ( size_t i = 0; i < number_in_message; ++ i ) {
    size_t j;
    chomp::Rect argument;
    job >> j;
    job >> argument;
    result << j;
    result << f ( argument );
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
      size_t j;
      chomp::Rect value;
      result >> j;
      result >> value;
      evals . val ( j ) = value;
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

