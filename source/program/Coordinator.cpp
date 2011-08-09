/*
 *  Coordinator.cpp
 */

#include <iostream>

#include "boost/foreach.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/set.hpp"

#include "program/Coordinator.h"
#include "program/Configuration.h"
#include "data_structures/UnionFind.hpp"

Coordinator::Coordinator(int argc, char **argv) {
  StartMorseStage ();
}

/* * * * * * * * * * * * */
/* Stage Initialization  */
/* * * * * * * * * * * * */

void Coordinator::StartMorseStage ( void ) {
  stage = 0;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
  
  int patch_stride = 1; // distance between center of patches in box-units
  // warning: currently only works if patch_stride is a power of two
  
  // Initialize parameter space bounds
  param_toplex . initialize (param_bounds);   /// Parameter space toplex
  // Subdivide parameter space toplex
  Real scale = Real ( 1.0 );
  int num_across = 1; // The number of boxes across
  for (int i = 0; i < PARAM_SUBDIVISIONS; ++i) {
    scale /= (Real) 2.0;
    num_across *= 2;
    param_toplex . subdivide (); // subdivide every top cell
  }
  // Determine the lengths of the boxes.
  // Also, determine the number of interior vertices = (num_across/stride - 1)^dim
  int num_interior_vertices = 1;
  int limit = num_across / patch_stride - 1;
  std::vector < Real > side_length ( PARAM_DIMENSION );
  for (int dim = 0; dim < PARAM_DIMENSION; ++ dim ) {
    num_interior_vertices *= limit;
    side_length [ dim ] = scale * (param_bounds . upper_bounds [ dim ] -
                                   param_bounds . lower_bounds [ dim ] );
  }
  // Cover the toplex with patches so that each codimension-one interior cell is
  // interior to at least one of the patches.
  for ( int i = 0; i < num_interior_vertices; ++ i ) {
    // Acquire the coordinates of the ith interior vertex
    int value = i;
    std::vector < Real > coordinates ( PARAM_DIMENSION );
    std::vector < Real > lower_bounds ( PARAM_DIMENSION );
    std::vector < Real > upper_bounds ( PARAM_DIMENSION );
    
    for (int dim = 0; dim < PARAM_DIMENSION; ++ dim ) {
      coordinates [ dim ] = param_bounds . lower_bounds [ dim ] 
      + ((Real) patch_stride) * side_length [ dim ] * ( (Real) 1.0 + (Real) ( value % limit ) );
      lower_bounds [ dim ] = coordinates [ dim ] - ((Real) patch_stride) * side_length [ dim ] / (Real) 2.0;
      upper_bounds [ dim ] = coordinates [ dim ] + ((Real) patch_stride) * side_length [ dim ] / (Real) 2.0;
      value /= limit;
    }
    // Produce a geometric region impinging on the neighbors of the interior vertex
    Geometric_Description patch_bounds (PARAM_DIMENSION, lower_bounds, upper_bounds);
    // Cover the geometric region with top cells
    Toplex_Subset patch_subset = param_toplex . cover ( patch_bounds );
    // Add "patch_subset" to the growing vector of patches
    PS_patches . push_back (patch_subset);
  } /* for */
  num_jobs_ = PS_patches . size ();

  std::cout << "Coordinator Constructed, there are " << num_jobs_ << " jobs.\n";
  //char c; std::cin >> c;
}

void Coordinator::StartConleyStage ( void ) {
  stage = 1;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;

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
    if ( work_items . insert ( classes . Representative ( element ) ) . second == true )
      conley_work_items . push_back ( element );
  }
  
  num_jobs_ = conley_work_items . size ();
}

/* * * * * * * * * * * */
/* Prepare Definitions */
/* * * * * * * * * * * */

CoordinatorBase::State Coordinator::Prepare(Message *job) {
  
  // All jobs have finished
  if (num_jobs_received_ == num_jobs_) {
    // The stage is complete. Move to next stage, if there is one.
    if ( stage == 0 ) {
      StartConleyStage ();
    } else {
      return kFinish;
    }
  }
  
  
  /// All jobs have been sent
  /// Still waiting for some jobs to finish
  if (num_jobs_sent_ == num_jobs_)
    return kPending;
  
  switch ( stage ) {
    case 0 :
      return MorsePrepare ( job );
      break;
    case 1 :
      return ConleyPrepare ( job );
      break;
  }
  
  return kOK; // never reached
}

CoordinatorBase::State Coordinator::MorsePrepare(Message *job) {

  /// There are jobs to be processed
  // Prepare a new job and send it to process

  /// Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;
  
  std::cout << "Coordinator::Prepare: Preparing job " << job_number << "\n";
  
  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = PS_patches [job_number];

  // Cell Name data (translate back into top-cell names from index number)
  std::vector < Toplex::Top_Cell > cell_names;
  // Geometric Description Data
  std::vector < Geometric_Description > geometric_descriptions;
  /// Adjacency information vector
  std::vector < std::pair < size_t, size_t > > adjacency_information;
  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Toplex::Top_Cell cell_in_patch, patch_subset ) {
    // Find geometry of patch cell
    Geometric_Description GD = param_toplex . geometry (param_toplex . find (cell_in_patch));
    // Store the name of the patch cell
    cell_names . push_back ( cell_in_patch );
    // Store the geometric description of the patch cell
    geometric_descriptions . push_back ( GD );
    // Find the cells in toplex which intersect patch cell
    Toplex_Subset GD_Cover = param_toplex . cover ( GD );
    // Store the cells in the patch which intersect the patch cell as adjacency pairs
    BOOST_FOREACH ( Toplex::Top_Cell cell_in_cover, GD_Cover ) {
      if (( patch_subset . count (cell_in_cover) != 0 ) && cell_in_patch < cell_in_cover ) {
        adjacency_information . push_back ( std::make_pair ( cell_in_patch, cell_in_cover ) );
      }
    }
  }

  //std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  
  // prepare the message with the job to be sent
  *job << stage;
  *job << job_number;
  *job << cell_names;
  *job << geometric_descriptions;
  *job << adjacency_information;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}

CoordinatorBase::State Coordinator::ConleyPrepare(Message *job) {
  /// send the job
  size_t job_number = num_jobs_sent_;
  Toplex::Top_Cell cell = conley_work_items [ job_number ] . first;
  Geometric_Description GD = param_toplex . geometry (param_toplex . find (cell));

  *job << stage;
  *job << job_number;
  *job << GD;
  *job << conley_work_items [ job_number ];
  
  //std::cout << "Preparing conley job " << job_number << " with GD = " << GD << "\n";
  
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}

/* * * * * * * * * * * */
/* Process Definitions */
/* * * * * * * * * * * */

void Coordinator::Process(const Message &result) {
  switch ( stage ) {
    case 0:
      MorseProcess ( result );
      break;
    case 1:
      ConleyProcess ( result );
      break;
    default:
      break;
  }
  /// Increment jobs received counter
  ++num_jobs_received_;
  // Are we done?
  if ( num_jobs_received_ == num_jobs_ && stage == 1 ) finalize ();
  return;
}  
  
void Coordinator::MorseProcess(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "Coordinator::Process: Received result " << job_number << "\n";

}

void Coordinator::ConleyProcess(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "Coordinator::Process: Received result " << job_number << "\n";

}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */

void Coordinator::finalize ( void ) {
  std::cout << "Coordinate::finalize ()\n";
  database . save ( "database.cmdb" );
}
