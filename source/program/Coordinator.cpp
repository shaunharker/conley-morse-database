/*
 *  Coordinator.cpp
 */

#include "program/Coordinator.h"
#include "boost/foreach.hpp"
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"
#include <ctime>
#include <fstream>
#include <algorithm>
#include "boost/foreach.hpp"

Coordinator::Coordinator(int argc, char **argv) {
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
    // DEBUG
    /*
    std::cout << "    patch_bounds = " << patch_bounds << "\n";
    std::cout << "    intersects " << patch_subset . size () << " top cells\n";
    BOOST_FOREACH ( Toplex::Top_Cell cell, patch_subset ) {
      std::cout << "        --> " << param_toplex . geometry ( cell ) << "\n";
    }
     */
  } /* for */
  num_jobs_ = PS_patches . size ();
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
  std::cout << "Coordinator Constructed, there are " << num_jobs_ << " jobs.\n";
  //char c; std::cin >> c;
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
    //std::cout << "  Creating patch #" << n << "\n";
    for (size_t i = 0; i < param_dim; ++i) {
      factor *= (i == 0) ? 1 : subdivisions_per_side;
      size_t index = (n / factor) % subdivisions_per_side;
      /// Compute bounding box for the patch
      patch_lower_bounds [i] = param_bounds . lower_bounds [i] + index * patch_sides_length [i];
      patch_upper_bounds [i] = patch_lower_bounds [i] + /*intersect_factor **/ patch_sides_length [i];
    }

    /// Construct the patch toplex
    Geometric_Description patch_bounds (param_dim, patch_lower_bounds, patch_upper_bounds);
    //std::cout << "    patch_bounds = " << patch_bounds << "\n";
    Toplex_Subset patch_subset = param_toplex . cover ( patch_bounds );
    //std::cout << "    intersects " << patch_subset . size () << " top cells\n";
    //BOOST_FOREACH ( Toplex::Top_Cell cell, patch_subset ) {
    //  std::cout << "        --> " << param_toplex . geometry ( cell ) << "\n";
    //}
    /// Add  patch to the vector of patches
    PS_patches . push_back (patch_subset);
  }
  //std::cout << "PS_patches . size () = " << PS_patches . size () << "\n";
  
  num_jobs_ = num_patches;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
  //std::cout << "Coordinator Constructed.\n";
  //char c; std::cin >> c;
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
  //std::cout << "Coordinator::Prepare: Preparing job " << job_number << "\n";

  //std::cout << "PS_patches . size () = " << PS_patches . size () << "\n";
  
  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = PS_patches [job_number];

  std::vector < Geometric_Description > geometric_descriptions (patch_subset . size ());

  /// Map with the pairing (top_cell, index in geometric_descriptions vector)
  std::map <Toplex::Top_Cell, size_t> cells_indices_map;

  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_subset . begin (); it != patch_subset . end (); ++it, ++key) {
    Geometric_Description Cell_GD = param_toplex . geometry (param_toplex . find (*it));
	/// Add geometric description to the vector of geo descriptions
    geometric_descriptions [ key ] = Cell_GD;
    //std::cout << "Cell_GD = " << Cell_GD << "\n";
	/// Add the pair (top_cell, key) to the indices map
    cells_indices_map . insert ( std::pair <Toplex::Top_Cell, size_t> (* param_toplex . find (*it), key) );

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
        //std::cout << "storing adjacency between " << cell_key << " and " << neighbor_key << "\n";
      }
    }
  }

  //std::cout << "Coordinator::Prepare: Sent job " << num_jobs_sent_ << "\n";
  //std::cout << "  patch size = " << geometric_descriptions . size () << "\n";
  //char c; std::cin >> c;
  // DEBUG
  //if ( num_jobs_sent_ == 50 ) {
  //  exit ( 1 );
  // }
  
  // prepare the message with the job to be sent
  *job << job_number;
  *job << cell_names;
  *job << geometric_descriptions;
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
  std::vector <Conley_Morse_Graph> conley_morse_graphs;
  result >> conley_morse_graphs;
  std::map < std::pair < size_t, size_t >, std::set < std::pair < Conley_Morse_Graph::Vertex, Conley_Morse_Graph::Vertex > > > clutch;
  //result >> clutch;
  std::vector < Toplex::Top_Cell > cell_names;
  result >> cell_names;
  std::vector < std::vector <size_t> > equivalence_classes;
  result >> equivalence_classes;

  // Turn equivalence classes into a UnionFind structure (converting to topcells)
  UnionFind < Toplex::Top_Cell > new_continuation_info;
  BOOST_FOREACH ( std::vector < size_t > & eqv_class, equivalence_classes ) {
    for ( int i = 0; i < (int) eqv_class . size (); ++ i ) {
      new_continuation_info . Add ( cell_names [ eqv_class [ i ] ] );
      if ( i > 0 ) new_continuation_info . Union ( cell_names [ eqv_class [ 0 ] ], 
                                                   cell_names [ eqv_class [ i ] ] );
    }
  }
  continuation_classes . Merge ( new_continuation_info );
  
  // Copy CMG's into DATABASE
  int count = 0;
  BOOST_FOREACH ( const Conley_Morse_Graph & cp, conley_morse_graphs ) {
    graphs [ cell_names [ count ] ] = cp;
    ++ count;
  }

  // Copy CLUTCHING GRAPHS into DATABASE
  typedef std::pair < std::pair < size_t, size_t >, std::set < std::pair < Conley_Morse_Graph::Vertex, Conley_Morse_Graph::Vertex > > > ClutchPair;
  BOOST_FOREACH ( const ClutchPair & cp, clutch ) {
    clutchings [ std::make_pair ( cell_names [ cp . first . first ], cell_names [ cp . first . second ] ) ] = cp . second;
  }

  std::cout << "Coordinator::Process: Received result " << job_number << "\n";
  
  /// Increment jobs received counter
  ++num_jobs_received_;
  
  // Are we done?
  if ( num_jobs_received_ == num_jobs_ ) {
    finalize ();
  }

  return;
}

// temporary hack to get continuation graph, eventually to be promoted to database structure
void Coordinator::save_continuation_graph ( void ) {
  Toplex & toplex = param_toplex; // convenient
  std::vector < std::vector < Toplex::Top_Cell > > data;
  continuation_classes . FillToVector ( &data );
  
  // First, convert the classes into sets of top cells 
  std::vector < std::set < Toplex::Top_Cell > > classes ( data . size () );
  for ( unsigned int i = 0; i < data . size (); ++ i ) {
    std::insert_iterator < std::set < Toplex::Top_Cell > > ii ( classes [ i ], classes [ i ] . begin () );
    std::copy ( data [ i ] . begin (), data [ i ] . end (), ii );
  }
  // Loop through cells in classes and form a transpose table
  std::vector < int > included ( toplex . tree_size (), -1 );
  for ( unsigned int i = 0; i < data . size (); ++ i ) {
    BOOST_FOREACH ( Toplex::Top_Cell t, data [ i ] ) {
      included [ t ] = i;
    }
  }
  
  std::set < std::pair < unsigned int, unsigned int > > my_edges;
  // Loop through cells
  typedef Toplex::iterator IT;
  for ( IT it = toplex . begin (); it != toplex . end (); ++ it ) {
    Top_Cell t = * it;
    typedef std::vector < Toplex::Top_Cell > CellContainer;
    std::vector < Toplex::Top_Cell > neighbors;
    std::insert_iterator < CellContainer > ii ( neighbors, neighbors . begin () );
    toplex . cover ( ii, toplex . geometry ( t ) );
    BOOST_FOREACH ( Top_Cell s, neighbors ) {
      if ( included [ s ] < included [ t ] ) {
        my_edges . insert ( std::make_pair ( included [ s ], included [ t ] ) );
      }
    }
  }
  
  // Now output the continuation graph to a file
  std::ofstream outfile ("continuationgraph.gv");
  outfile << "digraph G {\n";
  for ( unsigned int i = 0; i < data . size (); ++ i ) {
    if ( data [ i ] . size () > 1 ) {
      outfile << "subgraph cluster" << i << " {\n"; //[label=\"" << data [ i ] . size () << "\"];\n";
      Top_Cell rep = continuation_classes . Representative ( data [ i ] [ 0 ] );
      // Make rep's CMG
      typedef Conley_Morse_Graph CMG;
      CMG & cmg = graphs [ rep ];
      typedef CMG::Vertex V;
      typedef CMG::Edge E;
      typedef CMG::VertexIterator VI;
      typedef CMG::EdgeIterator EI;
      
      // LOOP THROUGH VERTICES AND GIVE THEM NAMES
      std::map < V, int > vertex_to_index;
      VI start, stop;
      int j = 0;
      for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
        vertex_to_index [ *start ] = i;
        outfile << i << "N" << j << " [label=\""<< " " <<  /*cmg . CubeSet (*start) .size () <<*/ "\"]\n";
        ++ j;
      }
      int N = cmg . NumVertices ();
      
      // LOOP THROUGH CMG EDGES
      EI estart, estop;
      typedef std::pair<int, int> int_pair;
      std::set < int_pair > edges;
      for (boost::tie ( estart, estop ) = cmg . Edges ();
           estart != estop;
           ++ estart ) {
        V source = cmg . Source ( *estart );
        V target = cmg . Target ( *estart );
        int index_source = vertex_to_index [ source ];
        int index_target = vertex_to_index [ target ];
        if ( index_source != index_target ) // Cull the self-edges
          edges . insert ( std::make_pair ( index_source, index_target ) );
      }
      // TRANSITIVE REDUCTION (n^5, non-optimal)
      // We determine those edges (a, c) for which there are edges (a, b) and (b, c)
      // and store them in "transitive_edges"
      std::set < int_pair > transitive_edges;
      BOOST_FOREACH ( int_pair edge, edges ) {
        for ( int j = 0; j < N; ++ j ) {
          bool left = false;
          bool right = false;
          BOOST_FOREACH ( int_pair edge2, edges ) {
            if ( edge2 . first == edge . first && edge2 . second == j ) left = true;
            if ( edge2 . first == j && edge2 . second == edge . second ) right = true;
          }
          if ( left && right ) transitive_edges . insert ( edge );
        }
      }
      
      // PRINT OUT EDGES OF TRANSITIVE CLOSURE
      BOOST_FOREACH ( int_pair edge, edges ) {
        if ( transitive_edges . count ( edge ) == 0 )
          outfile << i << "N" << edge . first << " -> " << i << "N" << edge . second << ";\n";
      }
      
      edges . clear ();
      transitive_edges . clear ();
      
      outfile << "}\n";
      
      // END CMG WRITING CODE
    }
    
  }
  
  
  // clutching edges
  outfile << "edge [dir=none,color=red];\n";
  typedef std::pair < unsigned int, unsigned int > Edge_t; 
  BOOST_FOREACH ( const Edge_t & my_edge, my_edges ) {
    if ( data [ my_edge . first ] . size () > 1 && data [ my_edge . second ] . size () > 1 ) {
      //outfile << edge . first << " -- " << edge . second << ";\n";
      Top_Cell rep1 = continuation_classes . Representative ( data [ my_edge.first ] [ 0 ] );
      Top_Cell rep2 = continuation_classes . Representative ( data [ my_edge.second ] [ 0 ] );
      if ( rep1 > rep2) continue;
      // Make rep's CMG
      typedef Conley_Morse_Graph CMG;
      CMG & cmg1 = graphs [ rep1 ];
      CMG & cmg2 = graphs [ rep2 ];
      
      outfile << my_edge.first << "N" << 0 << " -> " << my_edge.second << "N" << 0 << " " << 
      "[ltail=cluster" << my_edge.first << ",lhead=cluster" << my_edge.second << "];\n";
#if 0
      typedef CMG::Vertex V;
      typedef CMG::Edge E;
      typedef CMG::VertexIterator VI;
      typedef CMG::EdgeIterator EI;
      
      //std::ofstream outfile ( filename );
      
      //outfile << "digraph G { \n";
      //outfile << "node [ shape = point, color=black  ];\n";
      //outfile << "edge [ color=red  ];\n";
      
      // LOOP THROUGH VERTICES AND GIVE THEM NAMES
      std::map < V, int > vertex_to_index1;
      std::map < V, int > vertex_to_index2;
      
      VI start, stop;
      
      
      
      typedef std::pair<int, int> int_pair;
      std::set < int_pair > edges;
      std::set < int_pair > transitive_edges;
      
      // LOOP THROUGH CMG EDGES
      EI estart, estop;
      //outfile << " subgraph cluster0 {\n color=blue; ";
      
      int i = 0;
      for (boost::tie ( start, stop ) = cmg1 . Vertices ();
           start != stop;
           ++ start ) {
        vertex_to_index1 [ *start ] = i;
        //outfile << i << " [label=\""<< cmg1.CubeSet(*start).size() << "\"]\n";
        ++ i;
      }
      int N = i;
      
      for (boost::tie ( estart, estop ) = cmg1 . Edges ();
           estart != estop;
           ++ estart ) {
        V source = cmg1 . Source ( *estart );
        V target = cmg1 . Target ( *estart );
        int index_source = vertex_to_index1 [ source ];
        int index_target = vertex_to_index1 [ target ];
        edges . insert ( std::make_pair ( index_source, index_target ) );
      }
      // a kind of bad transitive closure algorithm (n^5, should be n^3, worry later)
      BOOST_FOREACH ( int_pair edge, edges ) {
        for ( int j = 0; j < N; ++ j ) {
          bool left = false;
          bool right = false;
          BOOST_FOREACH ( int_pair edge2, edges ) {
            if ( edge2 . first == edge . first && edge2 . second == j ) left = true;
            if ( edge2 . first == j && edge2 . second == edge . second ) right = true;
          }
          if ( left && right ) transitive_edges . insert ( edge );
        }
      }
      /*
      BOOST_FOREACH ( int_pair edge, edges ) {
        if ( transitive_edges . count ( edge ) == 0 )
          outfile << edge . first << " -> " << edge . second << ";\n";
      }
       */
      
      edges . clear ();
      transitive_edges . clear ();
      
      //outfile << " }\n subgraph cluster1 {\n color=blue;\n";
      for (boost::tie ( start, stop ) = cmg2 . Vertices ();
           start != stop;
           ++ start ) {
        vertex_to_index2 [ *start ] = i;
        //outfile << i << " [label=\""<< cmg2.CubeSet(*start).size() << "\"]\n";
        ++ i;
      }
      int M = i;
      for (boost::tie ( estart, estop ) = cmg2 . Edges ();
           estart != estop;
           ++ estart ) {
        V source = cmg2 . Source ( *estart );
        V target = cmg2 . Target ( *estart );
        int index_source = vertex_to_index2 [ source ];
        int index_target = vertex_to_index2 [ target ];
        edges . insert ( std::make_pair ( index_source, index_target ) );
      }
      
      // a kind of bad transitive closure algorithm (n^5, should be n^3, worry later)
      BOOST_FOREACH ( int_pair edge, edges ) {
        for ( int j = N; j < M; ++ j ) {
          bool left = false;
          bool right = false;
          BOOST_FOREACH ( int_pair edge2, edges ) {
            if ( edge2 . first == edge . first && edge2 . second == j ) left = true;
            if ( edge2 . first == j && edge2 . second == edge . second ) right = true;
          }
          if ( left && right ) transitive_edges . insert ( edge );
        }
      }
      /*
      BOOST_FOREACH ( int_pair edge, edges ) {
        if ( transitive_edges . count ( edge ) == 0 )
          outfile << edge . first << " -> " << edge . second << ";\n";
      }
       */
      
      // LOOP THROUGH CLUTCHING EDGES
      typedef std::pair<V,V> VertexPair;
      BOOST_FOREACH ( const VertexPair & edge, clutchings [ std::make_pair ( rep1, rep2 ) ] ) {
        int index_source = vertex_to_index1 [ edge . first ];
        int index_target = vertex_to_index2 [ edge . second ];
        outfile << my_edge.first << "N" << index_source << " -> " << my_edge.second << "N" << index_target << ";\n";
        
      }
#endif
    } // if
  } // boostforeach
  outfile << "}\n";
  outfile . close ();
}

void Coordinator::finalize ( void ) {
  std::cout << "Coordinate::finalize ()\n";
  
  // dump continuation class data
  std::cout << "continuation classes so far:\n";
  std::vector < std::vector < Toplex::Top_Cell > > data;
  continuation_classes . FillToVector ( &data );
  
  save_continuation_graph (); // put this puppy in a file
  
  BOOST_FOREACH ( std::vector < Toplex::Top_Cell > & eqv_class, data ) {
    std::cout << "CLASS: ";
    for ( int i = 0; i < (int) eqv_class . size (); ++ i ) {
      std::cout << eqv_class [ i ] << " ";
    }
    std::cout << "\n";
  }
  
  std::vector<std::vector<Toplex::Top_Cell> > classes;
  continuation_classes . FillToVector( &classes ); 
  std::cout << "Number of classes = " << classes . size () << "\n";
  for ( int i = 0; i < (int) classes . size (); ++ i ) {
    std::cout << "  Size of class " << i << " is " << classes [ i ] . size () << "\n";
  }
  
  
  // Draw a parameter space picture if it is 2D.
  if ( PARAM_DIMENSION != 2 ) return;
  std::cout << "Drawing parameter space picture\n";
  // Create a picture of parameter space.

  clock_t start = clock ();
  int Width = 640;
  int Height = 640;
  // Start with a clear picture of the right size.
  Picture * picture = new Picture(Width, Height, 
                                  param_bounds . lower_bounds [ 0 ], 
                                  param_bounds . upper_bounds [ 0 ],
                                  param_bounds . lower_bounds [ 1 ], 
                                  param_bounds . upper_bounds [ 1 ]);
  BOOST_FOREACH ( std::vector < Toplex::Top_Cell > & eqv_class, classes ) {
    // Cobble together the subset
    start = clock ();
    Toplex::Subset subset;
    BOOST_FOREACH ( Toplex::Top_Cell cell, eqv_class ) {
      subset . insert ( cell );
    }
    std::cout << "Made subset, " << (float)(clock() - start)/(float)CLOCKS_PER_SEC << "\n";
    
    // Pick Some Random Color
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    // Draw a picture of the subset
    start = clock ();

    Picture * class_picture = draw_picture ( Width, Height, 
                                            Red, Green, Blue, 
                                            param_toplex, subset );
    std::cout << "draw_picture, " << (float)(clock() - start)/(float)CLOCKS_PER_SEC << "\n";

    // Combine the picture with the previous ones
    start = clock ();
    Picture * combination = combine_pictures ( Width, Height, *picture, *class_picture );
    std::cout << "combine_pictures, " << (float)(clock() - start)/(float)CLOCKS_PER_SEC << "\n";

    std::swap ( picture, combination );
    delete class_picture;
    delete combination;
  }
  std::cout << "encoding file...\n";
  LodePNG_encode32_file( "image.png", picture -> bitmap, picture -> Width, picture -> Height);

}
