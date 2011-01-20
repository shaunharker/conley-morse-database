/*
 *  Coordinator.cpp
 */

#include "program/Coordinator.h"
#include "boost/foreach.hpp"
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"

Coordinator::Coordinator(int argc, char **argv) {
  int patch_stride = 2; // distance between center of patches in box-units
  // warning: currently only works if patch_stride is a power of two
  
  // Initialize parameter space bounds
  param_toplex . initialize (param_bounds);   /// Parameter space toplex
  // Subdivide parameter space toplex
  Real scale = 1;
  int num_across = 1;
  for (int i = 0; i < PARAM_SUBDIVISIONS; ++i) {
    scale /= (Real) 2.0;
    num_across *= 2;
    param_toplex . subdivide (); // subdivide every top cell
  }
  // Determine the lengths of the boxes.
  // Also, determine the number of interior vertices = (num_across - 1)^dim
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
    // DEBUG
    std::cout << "    patch_bounds = " << patch_bounds << "\n";
    std::cout << "    intersects " << patch_subset . size () << " top cells\n";
    BOOST_FOREACH ( Toplex::Top_Cell cell, patch_subset ) {
      std::cout << "        --> " << param_toplex . geometry ( cell ) << "\n";
    }
  } /* for */
  num_jobs_ = PS_patches . size ();
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
  std::cout << "Coordinator Constructed.\n";
  char c; std::cin >> c;
}
  
#if 0 // deprecating
Coordinator::Coordinator(int argc, char **argv) {
  size_t maxPatchSize = MAX_PATCH_SIZE; /// Maximum size of a patch in parameter space
  size_t param_dim = PARAM_DIMENSION;      /// Parameter space dimension
  param_toplex . initialize (param_bounds);   /// Parameter space toplex

  std::cout << "PARAM_DIMENSION = " << PARAM_DIMENSION << "\n";  
  std::cout << "PARAM_SUBDIVISIONS = " << PARAM_SUBDIVISIONS << "\n";
  std::cout << "param_bounds = " << param_bounds << "\n";
  
  /// Factor of the size of patch side that intersect next patch
  Real intersect_factor = 0.5;

  /// Subdivide parameter space toplex
  for (int i = 0; i < PARAM_SUBDIVISIONS; ++i) {
    intersect_factor /= 2.0;  /// Make intersect_factor x box_side equal half the size of a cell
    param_toplex . subdivide (); // subdivide every top cell
  }

  if (maxPatchSize < 2) {
    std::cout << "Warning: user specified MAX_PATCH_SIZE must be at least 2; adjusting.\n";
    maxPatchSize = 2; 
  }
  
  std::cout << "param_toplex . size () = " << param_toplex . size () << "\n";
  
  /// Minimum number of parameter patches
  size_t min_num_patches = static_cast <size_t> (param_toplex . size () / maxPatchSize) + 1;
  /// Number of subdivisons per side
  size_t subdivisions_per_side = static_cast <size_t> (std::ceil (std::exp (std::log ( (Real) min_num_patches) / param_dim)));

  std::cout << "maxPatchSize = " << maxPatchSize << "\n";
  std::cout << "min_num_patches = " << min_num_patches << "\n";
  std::cout << "subdivisions_per_side = " << subdivisions_per_side << "\n";
  
  /// Lenghts of the sides of the patch
  std::vector < Real > patch_sides_length (param_dim);

  /// Number of patches
  size_t num_patches = 1;
  for (size_t i = 0; i < param_dim; ++i) {
    patch_sides_length [i] = (param_bounds . upper_bounds [i] - param_bounds . lower_bounds [i]) / subdivisions_per_side;
    num_patches *= subdivisions_per_side;
    std::cout << "patch_sides_length [ " << i << " ] = " << patch_sides_length [ i ] << "\n";
  }

  std::cout << "num_patches = " << num_patches << "\n";
  
  /// Bounds for the patch bounding box
  std::vector < Real > patch_lower_bounds (param_dim);
  std::vector < Real > patch_upper_bounds (param_dim);

  /// Create all the patches
  for (size_t n = 0; n < num_patches; ++n) {
    size_t factor = 1;
    std::cout << "  Creating patch #" << n << "\n";
    for (size_t i = 0; i < param_dim; ++i) {
      factor *= (i == 0) ? 1 : subdivisions_per_side;
      size_t index = (n / factor) % subdivisions_per_side;
      /// Compute bounding box for the patch
      patch_lower_bounds [i] = param_bounds . lower_bounds [i] + index * patch_sides_length [i];
      patch_upper_bounds [i] = patch_lower_bounds [i] + /*intersect_factor **/ patch_sides_length [i];
    }

    /// Construct the patch toplex
    Geometric_Description patch_bounds (param_dim, patch_lower_bounds, patch_upper_bounds);
    std::cout << "    patch_bounds = " << patch_bounds << "\n";
    Toplex_Subset patch_subset = param_toplex . cover ( patch_bounds );
    std::cout << "    intersects " << patch_subset . size () << " top cells\n";
    BOOST_FOREACH ( Toplex::Top_Cell cell, patch_subset ) {
      std::cout << "        --> " << param_toplex . geometry ( cell ) << "\n";
    }
    /// Add  patch to the vector of patches
    PS_patches . push_back (patch_subset);
  }
  std::cout << "PS_patches . size () = " << PS_patches . size () << "\n";

  /// Create a map with Cached_Box_Information for the intersecting boxes
  for (size_t i = 0; i < num_patches; ++i) {
    BOOST_FOREACH ( Toplex::Top_Cell cell, PS_patches [i] ) {
      for (size_t j = i + 1; j < num_patches; ++j) {
	    /// If top cell is in the intersection of PS_patches [i] and PS_patches [j]
        if (PS_patches [j] . find (cell) != PS_patches [j] . end ()) {
          /// Add an entry to the map of Cached_Box_Information
          Cached_Box_Information cached_box_info;
          PS_Toplex_Cached_Info . insert (Toplex_Cached_Box_Pair (cell, cached_box_info));
        }
      }
    }
  }
  
  num_jobs_ = num_patches;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
  std::cout << "Coordinator Constructed.\n";
  char c; std::cin >> c;
}
#endif


CoordinatorBase::State Coordinator::Prepare(Message *job) {
  /// All jobs have finished
  if (num_jobs_received_ == num_jobs_)
    return kFinish;

  /// All jabs have been sent
  /// Still waiting for some jobs to finish
  if (num_jobs_sent_ == num_jobs_)
    return kPending;

  /// There are jobs to be processed
  // Prepare a new job and send it to process

  /// Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;

  std::cout << "Coordinator::Prepare: Preparing job " << job_number << "\n";
  std::cout << "PS_patches . size () = " << PS_patches . size () << "\n";
  
  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = PS_patches [job_number];

  std::vector < Geometric_Description > geometric_descriptions (patch_subset . size ());
  //Patch_Cached_Box_Map patch_cached_info;

  /// Map with the pairing (top_cell, index in geometric_descriptions vector)
  std::map <Toplex::Top_Cell, size_t> cells_indices_map;

  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_subset . begin (); it != patch_subset . end (); ++it, ++key) {
    Geometric_Description Cell_GD = param_toplex . geometry (param_toplex . find (*it));
	/// Add geometric description to the vector of geo descriptions
    geometric_descriptions [ key ] = Cell_GD;
    std::cout << "Cell_GD = " << Cell_GD << "\n";
	/// Add the pair (top_cell, key) to the indices map
    cells_indices_map . insert ( std::pair <Toplex::Top_Cell, size_t> (* param_toplex . find (*it), key) );
    //Insert cached box info into the map if there is any
    //if (PS_Toplex_Cached_Info . find (*it) != PS_Toplex_Cached_Info . end ())
    //  patch_cached_info . insert (Patch_Cached_Box_Pair (key, PS_Toplex_Cached_Info . find (*it) -> second));
  
  }
  // Cell Name data (translate back into top-cell names from index number)
  std::vector < Toplex::Top_Cell > cell_names;
  /// Adjacency information vector
  std::vector < std::vector <size_t> > adjacency_information (patch_subset . size (), std::vector <size_t> (0) );
  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Toplex::Top_Cell cell_in_patch, patch_subset ) {
    // Store the actual top cell name
    cell_names . push_back ( cell_in_patch );
    /// Find geometric description for the cell
    Geometric_Description Cell_GD = param_toplex . geometry (param_toplex . find (cell_in_patch));

    /// Cover the geometric description in the toplex to get the neighbors
    Toplex_Subset Cell_GD_Cover = param_toplex . cover ( Cell_GD );

    /// For all cells in the cover
    BOOST_FOREACH ( Toplex::Top_Cell cell_in_cover, Cell_GD_Cover ) {
	  /// If cell_in_cover is in the patch its not the same as cell_in_patch, then its a neighbor
      if (( patch_subset . find (cell_in_cover) != patch_subset . end () ) &&
          ( patch_subset . find (cell_in_patch) != patch_subset . find (cell_in_cover) ) ) {
        size_t cell_key = cells_indices_map . find (cell_in_patch) -> second;
        size_t neighbor_key = cells_indices_map . find (cell_in_cover) -> second;
        adjacency_information [cell_key] . push_back (neighbor_key);
      }
    }
  }

  std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  std::cout << "  patch size = " << geometric_descriptions . size () << "\n";
  //char c; std::cin >> c;
  // DEBUG
  //if ( num_jobs_sent_ == 50 ) {
  //  exit ( 1 );
  // }
  
  // prepare the message with the job to be sent
  *job << job_number;
  *job << cell_names;
  *job << geometric_descriptions;
  *job << Patch_Cached_Box_Map (); //patch_cached_info;
  *job << adjacency_information;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}


void Coordinator::Process(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  result >> job_number;
  Patch_Cached_Box_Map patch_cached_info;
  result >> patch_cached_info;
  std::vector <Conley_Morse_Graph> conley_morse_graphs;
  result >> conley_morse_graphs;
  std::vector < Toplex::Top_Cell > cell_names;
  result >> cell_names;
  std::vector < std::vector <size_t> > equivalence_classes;
  result >> equivalence_classes;

  
  // Turn equivalence classes into a UnionFind structure (converting to topcells)
  UnionFind < Toplex::Top_Cell > new_continuation_info;
  BOOST_FOREACH ( std::vector < size_t > & eqv_class, equivalence_classes ) {
    for ( int i = 0; i < eqv_class . size (); ++ i ) {
      new_continuation_info . Add ( cell_names [ eqv_class [ i ] ] );
      if ( i > 0 ) new_continuation_info . Union ( cell_names [ eqv_class [ 0 ] ], 
                                                   cell_names [ eqv_class [ i ] ] );
    }
  }
  continuation_classes . Merge ( new_continuation_info );
  
  std::cout << "Coordinator::Process: Received result " << job_number << "\n";
  //char c; std::cin >> c;
  /// Paramter patch corresponding to received results
  Toplex_Subset patch_results = PS_patches [job_number];

  /// Update the cached box information in PS_Toplex_Cached_Info
  
  /*
  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_results . begin (); it != patch_results . end (); ++it, ++key) {
    if (patch_cached_info . find (key) != patch_cached_info . end ()) {
      PS_Toplex_Cached_Info . insert (Toplex_Cached_Box_Pair (* param_toplex . find (*it), patch_cached_info . find (key) -> second));
    }
  }
  */
  
  /// Number of parameter cells in this patch
  size_t num_cells = patch_results . size ();


  /// Temporary output
  std::cout << std::endl;
  std::cout << "Job " << job_number << " complete!" << std::endl;
  std::cout << "Number of CM Graphs computed: " << conley_morse_graphs . size() << std::endl;

  /// Save the Conley Morse graphs
  for (size_t i = 0; i < num_cells; ++i) {
//    std::cout << "Graph with " << conley_morse_graphs [i] . NumVertices() << " vertices." << std::endl;

    BOOST_FOREACH (Conley_Morse_Graph::Vertex vert_from, conley_morse_graphs [i] . Vertices ()) {
//      std::cout << conley_morse_graphs [i] . CubeSet (vert_from) << " -> ";
	  BOOST_FOREACH (Conley_Morse_Graph::Vertex vert_to, conley_morse_graphs [i] . OutEdges (vert_from)) {
//        std::cout << conley_morse_graphs [i] . CubeSet (vert_to) << " ";
      }
//      std::cout << std::endl;
    }
  }
//  std::cout << std::endl;


  /// Save the equivalence class information
  for (size_t i = 0; i < num_cells; ++i) {
    for (size_t j = 0; j < num_cells; ++j) {
      // std::cout << equivalence_classes [i] [j] << std::endl;
	}
  }

  /// Increment jobs received counter
  ++num_jobs_received_;
  
  // Are we done?
  if ( num_jobs_received_ == num_jobs_ ) {
    finalize ();
  }

  return;
}

void Coordinator::finalize ( void ) {
  // Draw a parameter space picture if it is 2D.
  if ( PARAM_DIMENSION != 2 ) return;
  // Create a picture of parameter space.
  std::vector<std::vector<Toplex::Top_Cell> > classes;
  continuation_classes . FillToVector( &classes ); 
  int Width = 500;
  int Height = 500;
  // Start with a clear picture of the right size.
  Picture * picture = new Picture(Width, Height, 
                                  param_bounds . lower_bounds [ 0 ], 
                                  param_bounds . upper_bounds [ 0 ],
                                  param_bounds . lower_bounds [ 1 ], 
                                  param_bounds . upper_bounds [ 1 ]);
  BOOST_FOREACH ( std::vector < Toplex::Top_Cell > & eqv_class, classes ) {
    // Cobble together the subset
    Toplex::Subset subset;
    BOOST_FOREACH ( Toplex::Top_Cell cell, eqv_class ) {
      subset . insert ( cell );
    }
    // Pick Some Random Color
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    // Draw a picture of the subset
    Picture * class_picture = draw_picture ( Width, Height, 
                                            Red, Green, Blue, 
                                            param_toplex, subset );
    // Combine the picture with the previous ones
    Picture * combination = combine_pictures ( Width, Height, *picture, *class_picture );
    std::swap ( picture, combination );
    delete class_picture;
    delete combination;
  }
  LodePNG_encode32_file( "image.png", picture -> bitmap, picture -> Width, picture -> Height);

}