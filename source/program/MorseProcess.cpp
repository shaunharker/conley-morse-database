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
#include "database/structures/UniformGrid.h"

#include "chomp/Rect.h"
#include "chomp/Complex.h"
#include "chomp/Chain.h"

#include "ModelMap.h"

/* In Conley_Morse_Database.cpp
#include <boost/serialization/export.hpp>
#ifdef HAVE_SUCCINCT
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
*/

void MorseProcess::command_line ( int argcin, char * argvin [] ) {
  argc = argcin;
  argv = argvin;
  model . initialize ( argc, argv );
}

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
 
 
  // CONSTUCT THE PARAMETER GRID

  parameter_grid = boost::shared_ptr<Grid> ( new PARAMETER_GRID );

  // Initialization for TreeGrid

  if ( boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid ) ) {
    boost::shared_ptr<TreeGrid> grid = 
      boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_PERIODIC );  
    for (int i = 0; i < config.PARAM_SUBDIV_DEPTH[0]; ++i) {
      for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
        parameter_grid -> subdivide (); // subdivide every top cell
      }
    }
  }

  // Initialization for UniformGrid
  if ( boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid ) ) {
    boost::shared_ptr<UniformGrid> grid = 
      boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_SUBDIV_SIZES,
                         config.PARAM_PERIODIC );
  }
  


  // JOB CONSTRUCTION

  // We create a collection of patches PS_patches
  
  // In order to accommodate periodicity, 
  //    we let the patches overhang from the outer bounds slightly.
  int patch_width = 4; // try to use (patch_width +- 1)^d boxes per patch 
  
  // Create patches_across:
  //    The distance between patch centers in each dimension.
  // EXAMPLE: num_across = 64, patch_width = 4 ---> patches_across = 9 

  std::vector<int> patches_across ( config.PARAM_DIM );
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
    patches_across[d] = 1 + config.PARAM_SUBDIV_SIZES[d] / patch_width; 
  }

  // Create "patches":
  //   The collection of rectangles whose coverings give rise to the
  //   patch based jobs.
  std::vector < RectGeo > patches;
  // Loop through D-tuples
  std::vector < int > coordinates ( config.PARAM_DIM, 0);
  bool finished = false;
  while ( not finished ) {
    RectGeo patch ( config.PARAM_DIM );
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      // tol shouldn't be necessary if cover is rigorous
      double tol = (config.PARAM_BOUNDS.upper_bounds[d] 
                   -config.PARAM_BOUNDS.lower_bounds[d]) 
                   /(double)(1000.0 * config.PARAM_SUBDIV_SIZES[d]);
      patch . lower_bounds [ d ] = 
        config.PARAM_BOUNDS.lower_bounds[d]+((double)coordinates[d])*
        (config.PARAM_BOUNDS.upper_bounds[d]-config.PARAM_BOUNDS.lower_bounds[d]) 
        /(double)patches_across[d] - tol;
      patch . upper_bounds [ d ] = 
        config.PARAM_BOUNDS.lower_bounds[d]+((double)(1+coordinates[d]))*
        (config.PARAM_BOUNDS.upper_bounds[d]-config.PARAM_BOUNDS.lower_bounds[d])
        /(double)patches_across[d] + tol;
      
      if ( not config.PARAM_PERIODIC [ d ] ) {
        if ( patch . lower_bounds [ d ] < config.PARAM_BOUNDS . lower_bounds [ d ] ) 
          patch . lower_bounds [ d ] = config.PARAM_BOUNDS . lower_bounds [ d ];
        if ( patch . upper_bounds [ d ] > config.PARAM_BOUNDS . upper_bounds [ d ] ) 
          patch . upper_bounds [ d ] = config.PARAM_BOUNDS . upper_bounds [ d ];
      }
    }
    patches . push_back ( patch );
    // ODOMETER STEP
    finished = true;
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      ++ coordinates [ d ];
      if ( coordinates [ d ] == patches_across [ d ] ) {
        coordinates [ d ] = 0;
      } else {
        finished = false;
        break;
      }
    }
  }
  
  size_t debug_size = 0;
  std::cout << "Created " << patches . size () << " patches.\n";
  BOOST_FOREACH ( const RectGeo & patch, patches ) {
    // Cover the geometric region with top cells 
    std::vector<Grid::GridElement> patch_vector = parameter_grid -> cover ( patch );
    GridSubset patch_subset ( patch_vector . begin (), patch_vector . end () );
    // Add "patch_subset" to the growing vector of patches
    PS_patches . push_back (patch_subset);
    debug_size += patch_subset . size ();
  } /* for */
  num_jobs_ = PS_patches . size ();
  database . insert ( parameter_grid );

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

  
  //size_t local_clutchings_ordered = 0;
  static size_t number_of_clutching_jobs_ordered = 0;
  GridSubset patch_subset = PS_patches [job_number];


  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Grid::GridElement grid_element_in_patch, patch_subset ) {
    // Find geometry of patch cell
    boost::shared_ptr<RectGeo> rect_geo = boost::dynamic_pointer_cast<RectGeo> 
      ( parameter_grid -> geometry ( grid_element_in_patch ) );
    RectGeo GD = * rect_geo;

#ifdef CHECKIFMAPISGOOD
    if ( not model . map ( GD ) -> good () ) continue;
#endif
    // Store the name of the patch cell
    box_names . push_back ( grid_element_in_patch );
    // Store the geometric description of the patch cell
    box_geometries . push_back ( GD );
    // Find the cells in toplex which intersect patch cell
    double tol = 1e-8; //TODO make this more robust
    for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
      GD . lower_bounds [ d ] -= tol;
      GD . upper_bounds [ d ] += tol;
    }
    std::vector<Grid::GridElement> GD_Cover_vec = parameter_grid -> cover ( GD );
    GridSubset GD_Cover ( GD_Cover_vec . begin (), GD_Cover_vec . end () );
    
    // Store the cells in the patch which intersect the patch cell as adjacency pairs
    BOOST_FOREACH (  Grid::GridElement grid_element_in_cover, GD_Cover ) {
#ifdef CHECKIFMAPISGOOD
      boost::shared_ptr<RectGeo> adjGD = boost::dynamic_pointer_cast<RectGeo> 
        ( parameter_grid -> geometry ( grid_element_in_cover ) );
      if ( not model . map ( *adjGD ) -> good () ) continue;
#endif
      if (( patch_subset . count (grid_element_in_cover) != 0 ) && grid_element_in_patch < grid_element_in_cover ) {
        box_adjacencies . push_back ( std::make_pair ( grid_element_in_patch, grid_element_in_cover ) );
        ++ number_of_clutching_jobs_ordered;
      }
    }
  }
  //std::cout << "# of clutchings ordered locally:" << local_clutchings_ordered << "\n";
  std::cout << "# of clutchings ordered so far: " << number_of_clutching_jobs_ordered << "\n";
  //std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  
  // prepare the message with the job to be sent
  job << job_number;
  job << box_names;
  job << box_geometries;
  job << box_adjacencies;
  job << config.PHASE_SUBDIV_INIT;
  job << config.PHASE_SUBDIV_MIN;
  job << config.PHASE_SUBDIV_MAX;
  job << config.PHASE_SUBDIV_LIMIT;
  job << model;
  //job << config.PHASE_BOUNDS;
  //job << config.PHASE_PERIODIC;
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
    // saves a checkpoint.
    // (note: checkpointing is same as finalize, but repeated)
    std::ofstream progress_file ( "progress.txt" );
    progress_file << "Morse Process Progress: " << progress_bar << " / " << num_jobs_ << "\n";
    progress_file . close ();
    std::string filestring ( argv[1] );
    std::string appendstring ( "/database.raw" );
    database . save ( (filestring + appendstring) . c_str () );
    time_of_last_checkpoint = clock ();
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
 
}
