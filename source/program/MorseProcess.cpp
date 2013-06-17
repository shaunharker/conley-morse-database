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
#include "database/structures/Database.h"

#ifdef HAVE_SUCCINCT
#include "database/structures/SuccinctGrid.h"
#endif
#include "database/structures/PointerGrid.h"

#include "chomp/Rect.h"
#include "chomp/Complex.h"
#include "chomp/Chain.h"

#include "ModelMap.h"

#include <boost/serialization/export.hpp>
#ifdef HAVE_SUCCINCT
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);


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
  
  int patch_width = 10; // try to use patch_width^d boxes per patch (plus or minus 1)
  
  // Initialize parameter space bounds
  parameter_grid = boost::shared_ptr<Grid> ( new PARAMETER_GRID );
  parameter_grid -> initialize ( config.PARAM_BOUNDS );   /// Parameter space grid
  
  // Subdivide parameter space toplex
  long num_across = 1;
  for (int i = 0; i < config.PARAM_SUBDIV_DEPTH; ++i) {
    num_across *= 2;
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      parameter_grid -> subdivide (); // subdivide every top cell
    }
  }
  
  int patches_across = 1 + num_across / patch_width; // distance between center of patches in box-units
  std::cout << "patches_across = " << patches_across << "\n";
  // Create the patches.
  std::vector < chomp::Rect > patches;
  // Loop through D-tuples
  std::vector < int > coordinates ( config.PARAM_DIM, 0);
  bool finished = false;
  while ( not finished ) {
    chomp::Rect patch ( config.PARAM_DIM );
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      // tol shouldn't be necessary if cover is rigorous
      double tol = (config.PARAM_BOUNDS . upper_bounds [ d ] - config.PARAM_BOUNDS . lower_bounds [ d ]) / (double) (1000 * num_across);
      patch . lower_bounds [ d ] = config.PARAM_BOUNDS . lower_bounds [ d ] + ( (double) coordinates[d] ) *
      (config.PARAM_BOUNDS . upper_bounds [ d ] - config.PARAM_BOUNDS . lower_bounds [ d ]) / (double)patches_across - tol;
      patch . upper_bounds [ d ] = config.PARAM_BOUNDS . lower_bounds [ d ] + ((double)(1 + coordinates[d])) *
      (config.PARAM_BOUNDS . upper_bounds [ d ] - config.PARAM_BOUNDS . lower_bounds [ d ]) / (double)patches_across + tol;
      
      if ( patch . lower_bounds [ d ] < config.PARAM_BOUNDS . lower_bounds [ d ] ) patch . lower_bounds [ d ] = config.PARAM_BOUNDS . lower_bounds [ d ];
      if ( patch . upper_bounds [ d ] > config.PARAM_BOUNDS . upper_bounds [ d ] ) patch . upper_bounds [ d ] = config.PARAM_BOUNDS . upper_bounds [ d ];
      
    }
    patches . push_back ( patch );
    // ODOMETER STEP
    finished = true;
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      ++ coordinates [ d ];
      if ( coordinates [ d ] == patches_across ) {
        coordinates [ d ] = 0;
      } else {
        finished = false;
        break;
      }
    }
  }
  
  size_t debug_size = 0;
  std::cout << "Created " << patches . size () << " patches.\n";
  BOOST_FOREACH ( const chomp::Rect & patch, patches ) {
    // Cover the geometric region with top cells
    GridSubset patch_subset;
    std::insert_iterator < GridSubset > ii ( patch_subset, patch_subset . begin () );
    parameter_grid -> cover ( ii, patch );
    // Add "patch_subset" to the growing vector of patches
    PS_patches . push_back (patch_subset);
    debug_size += patch_subset . size ();
  } /* for */
  num_jobs_ = PS_patches . size ();
  database . insert ( parameter_grid );
#endif
  std::cout << "MorseProcess Constructed, there are " << num_jobs_ << " jobs.\n";
  std::cout << "Number of parameter box calculations = " << debug_size << ".\n";
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
  
  //size_t local_clutchings_ordered = 0;
  static size_t number_of_clutching_jobs_ordered = 0;
  GridSubset patch_subset = PS_patches [job_number];


  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Grid::GridElement grid_element_in_patch, patch_subset ) {
    // Find geometry of patch cell
    Rect GD = parameter_grid -> geometry ( grid_element_in_patch );
    // DEBUG -- (check toplex::cover)
    double tol = 1e-8;
    for ( int d = 0; d < parameter_grid -> dimension (); ++ d ) {
      GD . lower_bounds [ d ] -= tol;
      GD . upper_bounds [ d ] += tol;
    }
    // END DEBUG
#ifdef CHECKIFMAPISGOOD
    ModelMap map ( GD );
    if ( not map . good () ) continue;
#endif
    // Store the name of the patch cell
    box_names . push_back ( grid_element_in_patch );
    // Store the geometric description of the patch cell
    box_geometries . push_back ( GD );
    // Find the cells in toplex which intersect patch cell
    GridSubset GD_Cover;
    std::insert_iterator < GridSubset > ii ( GD_Cover, GD_Cover . begin () );
    parameter_grid -> cover ( ii, GD );
    // Store the cells in the patch which intersect the patch cell as adjacency pairs
    BOOST_FOREACH (  Grid::GridElement grid_element_in_cover, GD_Cover ) {
#ifdef CHECKIFMAPISGOOD
      Rect adjGD = parameter_grid -> geometry ( grid_element_in_cover );
      ModelMap adjmap ( adjGD );
      if ( not adjmap . good () ) continue;
#endif
      if (( patch_subset . count (grid_element_in_cover) != 0 ) && grid_element_in_patch < grid_element_in_cover ) {
        box_adjacencies . push_back ( std::make_pair ( grid_element_in_patch, grid_element_in_cover ) );
        ++ number_of_clutching_jobs_ordered;
      }
    }
  }
#endif
  //std::cout << "# of clutchings ordered locally:" << local_clutchings_ordered << "\n";
  std::cout << "# of clutchings ordered so far: " << number_of_clutching_jobs_ordered << "\n";
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
	Clutching_Graph_Job < PHASE_GRID > ( &result , job );
  //result << (size_t)0;
  //result << Database ();
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
  if ( (float)(time - time_of_last_checkpoint ) / (float)CLOCKS_PER_SEC > 3600.0f ) {
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

  {
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.raw" );
  database . save ( (filestring + appendstring) . c_str () );
  }
  database . postprocess ();
  {
  std::string filestring ( argv[1] );
  std::string appendstring ( "/database.mdb" );
  database . save ( (filestring + appendstring) . c_str () );
  }
}
