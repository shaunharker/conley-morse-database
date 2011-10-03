/*
 *  MorseProcess.cpp
 */

#include <iostream>
#include <fstream>

#include "boost/foreach.hpp"

#include "program/Configuration.h"
#include "program/MorseProcess.h"
#include "program/jobs/Clutching_Graph_Job.h"
#include "structures/UnionFind.hpp"

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void MorseProcess::initialize ( void ) {
  num_jobs_sent_ = 0;
  
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
    Prism patch_bounds (PARAM_DIMENSION, lower_bounds, upper_bounds);
    // Cover the geometric region with top cells
    Toplex_Subset patch_subset;
    std::insert_iterator < Toplex_Subset > ii ( patch_subset, patch_subset . begin () );
    param_toplex . cover ( ii, patch_bounds );
    // Add "patch_subset" to the growing vector of patches
    PS_patches . push_back (patch_subset);
  } /* for */
  num_jobs_ = PS_patches . size ();

  std::cout << "MorseProcess Constructed, there are " << num_jobs_ << " jobs.\n";
  //char c; std::cin >> c;
}

/* * * * * * * * * * * */
/* prepare definition  */
/* * * * * * * * * * * */
int MorseProcess::prepare ( Message & job ) {
  /// All jobs have been sent
  /// Still waiting for some jobs to finish
  if (num_jobs_sent_ == num_jobs_) return 1;

  /// There are jobs to be processed
  // Prepare a new job and send it to process

  /// Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;
  
  std::cout << "MorseProcess::write: Preparing job " << job_number << "\n";
  
  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = PS_patches [job_number];

  // Cell Name data (translate back into top-cell names from index number)
  std::vector < Toplex::Top_Cell > cell_names;
  // Geometric Description Data
  std::vector < Prism > Prisms;
  /// Adjacency information vector
  std::vector < std::pair < size_t, size_t > > adjacency_information;
  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Toplex::Top_Cell cell_in_patch, patch_subset ) {
    // Find geometry of patch cell
    Prism GD = param_toplex . geometry (param_toplex . find (cell_in_patch));
    // Store the name of the patch cell
    cell_names . push_back ( cell_in_patch );
    // Store the geometric description of the patch cell
    Prisms . push_back ( GD );
    // Find the cells in toplex which intersect patch cell
    Toplex_Subset GD_Cover;
    std::insert_iterator < Toplex_Subset > ii ( GD_Cover, GD_Cover . begin () );
    param_toplex . cover ( ii, GD );
    // Store the cells in the patch which intersect the patch cell as adjacency pairs
    BOOST_FOREACH ( Toplex::Top_Cell cell_in_cover, GD_Cover ) {
      if (( patch_subset . count (cell_in_cover) != 0 ) && cell_in_patch < cell_in_cover ) {
        adjacency_information . push_back ( std::make_pair ( cell_in_patch, cell_in_cover ) );
      }
    }
  }

  //std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  
  // prepare the message with the job to be sent
  job << job_number;
  job << cell_names;
  job << Prisms;
  job << adjacency_information;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0;
}
 
/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void MorseProcess::work ( Message & result, const Message & job ) const {
	Clutching_Graph_Job < Toplex, Toplex, ConleyIndex_t > ( &result , job ); 
}

/* * * * * * * * * * */
/* accept definition */
/* * * * * * * * * * */
void MorseProcess::accept(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "MorseProcess::read: Received result " 
            << job_number << "\n";
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void MorseProcess::finalize ( void ) {
  std::cout << "MorseProcess::finalize ()\n";
  database . save ( "database.cmdb" );
}
