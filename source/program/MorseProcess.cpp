/*
 *  MorseProcess.cpp
 */

#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>

#include "boost/foreach.hpp"

#include "database/program/Configuration.h"
#include "database/program/MorseProcess.h"
#include "database/program/jobs/Clutching_Graph_Job.h"
#include "database/structures/UnionFind.hpp"

#include "chomp/Rect.h"
#include "chomp/Complex.h"
#include "chomp/Chain.h"

#include "ModelMap.h"

/* * * * * * * * * * * * */
/* initialize definition */
/* * * * * * * * * * * * */
void MorseProcess::initialize ( void ) {
  using namespace chomp;
  std::cout << "Attempting to load configuration...\n";
  config . loadFromFile ( argv[1] );
  std::cout << "Loaded configuration.\n";
  
  time_of_last_checkpoint = clock ();
  time_of_last_progress = clock ();
  progress_bar = 0;
  num_jobs_sent_ = 0;
  
  // EDGEMETHOD AND SKELETON METHOD REPEAT CODE
#if defined EDGEMETHOD || defined SKELETONMETHOD
  // Get width, length, height, etc... of parameter space.
  std::vector<uint32_t> dimension_sizes ( config.PARAM_DIM,
                                         1 << config.PARAM_SUBDIV_DEPTH);
  // Count total number of "boxes" (volume) of parameter space.
  uint32_t total = 1;
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
    total *= dimension_sizes [ d ];
  }
  // Initialize a complex to represent parameter space.
  param_complex . initialize ( dimension_sizes );
  // We have to add all the cubes. This is a little silly,
  // that there isn't a member function to do this yet.
  std::vector<uint32_t> cube ( config.PARAM_DIM, 0 );
  for ( long x = 0; x < total; ++ x ) {
    param_complex . addFullCube ( cube );
    bool carry = true;
    for ( unsigned int d = 0; d < cube . size (); ++ d ) {
      if ( not carry ) break;
      if ( ++ cube [ d ] == dimension_sizes [ d ] ) {
        cube [ d ] = 0;
        carry = true;
      } else {
        carry = false;
      }
    }
  }
  // Now we set the bounds and finalize the complex (so it is indexed).
  param_complex . bounds () = config.PARAM_BOUNDS;
  param_complex . finalize ();
  
#ifdef EDGEMETHOD
  int dim = 0;
#endif
#ifdef SKELETONMETHOD
  int dim = 1;
#endif
  for ( Index i = 0; i < param_complex . size ( dim ); ++ i ) {
    jobs_ . push_back ( std::make_pair ( i, dim ) );
  }
  num_jobs_ = jobs_ . size ();
#endif

#ifdef PATCHMETHOD
  int patch_stride = 1; // distance between center of patches in box-units
  // warning: currently only works if patch_stride is a power of two
  
  // Initialize parameter space bounds
  param_toplex . initialize ( config.PARAM_BOUNDS );   /// Parameter space toplex
  // Subdivide parameter space toplex
  Real scale = Real ( 1.0 );
  int num_across = 1; // The number of boxes across
  for (int i = 0; i < config.PARAM_SUBDIV_DEPTH; ++i) {
    scale /= (Real) 2.0;
    num_across *= 2;
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      param_toplex . subdivide (); // subdivide every top cell
    }
  }
  // Determine the lengths of the boxes.
  // Also, determine the number of interior vertices = (num_across/stride - 1)^dim
  int num_interior_vertices = 1;
  int limit = num_across / patch_stride - 1;
  std::vector < Real > side_length ( config.PARAM_DIM );
  for (int dim = 0; dim < config.PARAM_DIM; ++ dim ) {
    num_interior_vertices *= limit;
    side_length [ dim ] = scale * ( config.PARAM_BOUNDS . upper_bounds [ dim ] -
                                    config.PARAM_BOUNDS . lower_bounds [ dim ] );
  }
  // Cover the toplex with patches so that each codimension-one interior cell is
  // interior to at least one of the patches.
  for ( int i = 0; i < num_interior_vertices; ++ i ) {
    // Acquire the coordinates of the ith interior vertex
    int value = i;
    std::vector < Real > coordinates ( config.PARAM_DIM );
    std::vector < Real > lower_bounds ( config.PARAM_DIM );
    std::vector < Real > upper_bounds ( config.PARAM_DIM );
    
    for (int dim = 0; dim < config.PARAM_DIM; ++ dim ) {
      coordinates [ dim ] = config . PARAM_BOUNDS . lower_bounds [ dim ] 
      + ((Real) patch_stride) * side_length [ dim ] * ( (Real) 1.0 + (Real) ( value % limit ) );
      lower_bounds [ dim ] = coordinates [ dim ] - ((Real) patch_stride) * side_length [ dim ] / (Real) 2.0;
      upper_bounds [ dim ] = coordinates [ dim ] + ((Real) patch_stride) * side_length [ dim ] / (Real) 2.0;
      value /= limit;
    }
    // Produce a geometric region impinging on the neighbors of the interior vertex
    Rect patch_bounds (config.PARAM_DIM, lower_bounds, upper_bounds);
    // Cover the geometric region with top cells
    Toplex_Subset patch_subset;
    std::insert_iterator < Toplex_Subset > ii ( patch_subset, patch_subset . begin () );
    param_toplex . cover ( ii, patch_bounds );
    // Add "patch_subset" to the growing vector of patches
    PS_patches . push_back (patch_subset);
  } /* for */
  num_jobs_ = PS_patches . size ();
#endif
  std::cout << "MorseProcess Constructed, there are " << num_jobs_ << " jobs.\n";
  //char c; std::cin >> c;
}

/* * * * * * * * * * * */
/* prepare definition  */
/* * * * * * * * * * * */
int MorseProcess::prepare ( Message & job ) {
  using namespace chomp;
  /// All jobs have been sent
  /// Still waiting for some jobs to finish
  if (num_jobs_sent_ == num_jobs_) return 1;

  /// There are jobs to be processed
  // Prepare a new job and send it to process

  /// Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;
  
  std::cout << "MorseProcess::prepare: Preparing job " << job_number << "\n";
  
  /// Variables needed for job.
  // Cell Name data (translate back into top-cell names from index number)
  std::vector < size_t > box_names;
  // Geometric Description Data
  std::vector < Rect > box_geometries;
  /// Adjacency information vector
  std::vector < std::pair < size_t, size_t > > box_adjacencies;
#ifdef EDGEMETHOD
  std::pair < Index, int > cell = jobs_ [ job_number ];
  Chain cbd = param_complex . coboundary ( cell . first, cell . second ); //cell.second==0
  BOOST_FOREACH ( const Term & t, cbd () ) {
    box_geometries . push_back ( param_complex . geometry ( t . index (), cell . second + 1 ) );
    Index edge_name = param_complex . size ( cell . second ) + t . index ();
    box_names . push_back ( edge_name );
  }
  for ( unsigned int i = 0; i < box_names . size (); ++ i ) {
    for ( unsigned int j = i + 1; j < box_names . size (); ++ j ) {
      box_adjacencies . push_back ( std::make_pair ( box_names [ i ], box_names [ j ] ) );
    }
  }
#endif
  
#ifdef SKELETONMETHOD
  std::pair < Index, int > cell = jobs_ [ job_number ];
  box_geometries . push_back ( param_complex . geometry ( cell . first, cell . second ) );
  Index job_cell_name =  param_complex . size ( cell . second - 1 ) + cell . first;
  box_names . push_back ( job_cell_name );
  Chain bd = param_complex . boundary ( cell . first, cell . second );
  BOOST_FOREACH ( const Term & t, bd () ) {
    box_geometries . push_back ( param_complex . geometry ( t . index (), cell . second - 1 ) );
    Index sub_cell_name = param_complex . size ( cell . second - 2 ) + t . index ();
    box_names . push_back ( sub_cell_name );
    box_adjacencies . push_back ( std::make_pair ( job_cell_name, sub_cell_name ) );
  }
#endif
  
#ifdef PATCHMETHOD
  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = PS_patches [job_number];


  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Toplex::Top_Cell cell_in_patch, patch_subset ) {
    // Find geometry of patch cell
    Rect GD = param_toplex . geometry (param_toplex . find (cell_in_patch));
    // Store the name of the patch cell
    box_names . push_back ( cell_in_patch );
    // Store the geometric description of the patch cell
    box_geometries . push_back ( GD );
    // Find the cells in toplex which intersect patch cell
    Toplex_Subset GD_Cover;
    std::insert_iterator < Toplex_Subset > ii ( GD_Cover, GD_Cover . begin () );
    param_toplex . cover ( ii, GD );
    // Store the cells in the patch which intersect the patch cell as adjacency pairs
    BOOST_FOREACH ( Toplex::Top_Cell cell_in_cover, GD_Cover ) {
      if (( patch_subset . count (cell_in_cover) != 0 ) && cell_in_patch < cell_in_cover ) {
        box_adjacencies . push_back ( std::make_pair ( cell_in_patch, cell_in_cover ) );
      }
    }
  }
#endif
  //std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  
  // prepare the message with the job to be sent
  job << job_number;
  job << box_names;
  job << box_geometries;
  job << box_adjacencies;
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;
  job << config.PHASE_BOUNDS;
  job << config.PERIODIC;
  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return 0;
}
 
/* * * * * * * * * */
/* work definition */
/* * * * * * * * * */
void MorseProcess::work ( Message & result, const Message & job ) const {
  using namespace chomp;
	Clutching_Graph_Job < Toplex, Toplex, ConleyIndex_t > ( &result , job ); 
}

/* * * * * * * * * * */
/* accept definition */
/* * * * * * * * * * */
void MorseProcess::accept(const Message &result) {
  using namespace chomp;
  /// Read the results from the result message
  size_t job_number;
  Database job_database;
  result >> job_number;
  result >> job_database;
  // Merge the results
  database . merge ( job_database );
  std::cout << "MorseProcess::read: Received result " 
            << job_number << "\n";
  ++ progress_bar;
  
  clock_t time = clock ();
  if ( (float)(time - time_of_last_checkpoint ) / (float)CLOCKS_PER_SEC > 300.0f ) {
    finalize (); // doesn't end things, just saves a checkpoint.
    time_of_last_checkpoint = time;
  }
  if ( (float)(time - time_of_last_progress ) / (float)CLOCKS_PER_SEC > 1.0f ) {
    std::ofstream progress_file ( "progress.txt" );
    progress_file << "Morse Process Progress: " << progress_bar << " / " << num_jobs_ << "\n";
    progress_file . close ();
    time_of_last_progress = time;
  }
}

/* * * * * * * * * * * */
/* finalize definition */
/* * * * * * * * * * * */
void MorseProcess::finalize ( void ) {
  using namespace chomp;
  std::cout << "MorseProcess::finalize ()\n";
  
  std::ofstream progress_file ( "progress.txt" );
  progress_file << "Morse Process Progress: " << progress_bar << " / " << num_jobs_ << "\n";
  progress_file . close ();

  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.mdb" );
  database . save ( (filestring + appendstring) . c_str () );
}
