/// Construct and display a conley morse graph for a given dynamical system

//#define OLD_CMAP_METHOD

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <set>
#include <map>

// To get SCC chatter
#define CMG_VERBOSE 
//#define DO_CONLEY_INDEX
//#define NOREACHABILITY
#define VISUALIZE_DEBUG
#define ILLUSTRATE

// HEADERS FOR DATA STRUCTURES
#include "chomp/Toplex.h"

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"

// HEADERS FOR DEALING WITH PICTURES
#include "database/tools/picture.h"
#include "database/tools/lodepng/lodepng.h"


//////////////////////////////////////BEGIN USER EDIT//////////////////////////////////////////

// MAP FUNCTION OBJECT
#include <cmath>
#include "database/numerics/simple_interval.h"   // for interval arithmetic

#include "capd/capdlib.h"
#include "capd/dynsys/DynSysMap.h"
#include "capd/dynset/C0PpedSet.hpp"

#include "chomp/Rect.h"
#include "chomp/Prism.h"

using namespace chomp;

// choose example
//#define LORENZ
//#define TWODIMLESLIE
//#define PRISMLESLIE
//#define FIVEDIMPRISMLESLIE
//#define HENONEXAMPLE

#define CleverHenonMap
//#define HenonMap //20120726 CU
//#define HM3D     //20120726 CU

int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 8;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 12;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


#ifdef CleverHenonMap

#include "data/henon/ModelMap.h"

Rect initialize_phase_space_box ( void ) {
  
#ifdef HM3D
  int phase_space_dimension = 3;
#else
  int phase_space_dimension = 2;
#endif
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -1.5;
  phase_space_bounds . upper_bounds [ 0 ] =  1.5;
  phase_space_bounds . lower_bounds [ 1 ] = -1.5;
  phase_space_bounds . upper_bounds [ 1 ] =  1.5;
#ifdef HM3D
  phase_space_bounds . lower_bounds [ 2 ] = -1.5;
  phase_space_bounds . upper_bounds [ 2 ] =  1.5;
#endif
  
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension );
  parameter_space_limits . lower_bounds [ 0 ] = -2.5;
  parameter_space_limits . upper_bounds [ 0 ] =  2.5 ;
  parameter_space_limits . lower_bounds [ 1 ] = -2.5 ;
  parameter_space_limits . upper_bounds [ 1 ] =  2.5 ;
  int PARAMETER_BOXES = 10000;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] +
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] +
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] +
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] +
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}


#endif
#ifdef HenonMap  //20120726 CU

struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval p0, p1;
  
  ModelMap ( const Rect & rectangle ) {
    p0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    p1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () 
  ( const Rect & rectangle ) const {    
#ifdef HM3D
    Rect r ( 3 );
#else
    Rect r ( 2 );
#endif
    /* Read input */
    interval x0  (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]); 
    interval x1  (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]); 
        
    interval y0 = 0.3 * x1 + 1.0 - 1.4 * x0 * x0;
    interval y1 = x0;

    r . lower_bounds [ 0 ] = y0 . lower ();
    r . upper_bounds [ 0 ] = y0 . upper ();
    r . lower_bounds [ 1 ] = y1 . lower ();
    r . upper_bounds [ 1 ] = y1 . upper ();
#ifdef HM3D
    r . lower_bounds [ 2 ] = y0 . lower ();
    r . upper_bounds [ 2 ] = y0 . upper ();
#endif
    //std::cout << rectangle << " -> " << r << "\n";
    return r;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
        
#ifdef HM3D
  int phase_space_dimension = 3;
#else
  int phase_space_dimension = 2;
#endif
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -1.5;
  phase_space_bounds . upper_bounds [ 0 ] =  1.5;
  phase_space_bounds . lower_bounds [ 1 ] = -1.5;
  phase_space_bounds . upper_bounds [ 1 ] =  1.5;
#ifdef HM3D
  phase_space_bounds . lower_bounds [ 2 ] = -1.5;
  phase_space_bounds . upper_bounds [ 2 ] =  1.5;
#endif

  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = -2.5; 
  parameter_space_limits . upper_bounds [ 0 ] =  2.5 ;
  parameter_space_limits . lower_bounds [ 1 ] = -2.5 ;
  parameter_space_limits . upper_bounds [ 1 ] =  2.5 ;
  int PARAMETER_BOXES = 10000;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
    ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
    ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
    ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
    ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}
#endif

// TYPEDEFS
typedef std::vector < GridElement > CellContainer;
typedef ConleyMorseGraph < CellContainer , ConleyIndex_t > CMG;

// Declarations
void DrawMorseSets ( const Toplex & phase_space, const CMG & cmg );
void CreateDotFile ( const CMG & cmg );

template < class Toplex, class CellContainer >
void output_cubes ( const Toplex & my_toplex, 
                   const ConleyMorseGraph < CellContainer , chomp::ConleyIndex_t > & conley_morse_graph );


// MAIN PROGRAM
int main ( int argc, char * argv [] ) 
{
  
  clock_t start, stop;
  start = clock ();
  
  
  /* READ TWO INPUTS (which will give a parameter space box) */
  if ( argc != 3 ) {
    std::cout << "Usage: Supply 2 (not " << argc << ") arguments:\n";
    std::cout << "Input two integers in [0, " << 64 << "\n";
    return 0;
  }
  Real bx = ( Real ) atoi ( argv [ 1 ] );
  Real by = ( Real ) atoi ( argv [ 2 ] );
  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();

  /* SET PARAMETER SPACE REGION */
  Rect parameter_box = initialize_parameter_space_box (bx, by);

  /* INITIALIZE PHASE SPACE */
  Toplex phase_space;
  phase_space . initialize ( phase_space_bounds );
  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box );
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG conley_morse_graph;

  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph, 
                        & phase_space, 
                        map, 
                        SINGLECMG_MIN_PHASE_SUBDIVISIONS, 
                        SINGLECMG_MAX_PHASE_SUBDIVISIONS, 
                        SINGLECMG_COMPLEXITY_LIMIT );

  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " << 
    (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  /* DRAW MORSE SETS */
  DrawMorseSets ( phase_space, conley_morse_graph );

  output_cubes ( phase_space, conley_morse_graph );
  
  /* OUTPUT MORSE GRAPH */
  CreateDotFile ( conley_morse_graph );
  
  return 0;
} /* main */

/* ----------------------------  OUTPUT FUNCTIONS ---------------------------- */

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph ) {
  // Create a Picture
  int Width =  4096;
  int Height = 4096;
  Picture * picture = draw_morse_sets<Toplex,CellContainer>( Width, Height, phase_space, conley_morse_graph );
  LodePNG_encode32_file( "morse_sets.png", picture -> bitmap, picture -> Width, picture -> Height);
  Picture * picture2 = draw_toplex <Toplex,CellContainer>( Width, Height, phase_space );
  LodePNG_encode32_file( "toplex.png", picture2 -> bitmap, picture2 -> Width, picture2 -> Height);
  Picture * picture3 = draw_toplex_and_morse_sets <Toplex,CellContainer>( Width, Height, phase_space, conley_morse_graph );
  LodePNG_encode32_file( "toplex_and_morse.png", picture3 -> bitmap, picture3 -> Width, picture3 -> Height);
  delete picture;
  delete picture2;
  delete picture3;
}


void CreateDotFile ( const CMG & cmg ) {
  typedef CMG::Vertex V;
  typedef CMG::Edge E;
  typedef CMG::VertexIterator VI;
  typedef CMG::EdgeIterator EI;

  std::ofstream outfile ("morsegraph.gv");
  
  outfile << "digraph G { \n";
  //outfile << "node [ shape = point, color=black  ];\n";
  //outfile << "edge [ color=red  ];\n";

  // LOOP THROUGH VERTICES AND GIVE THEM NAMES
  std::map < V, int > vertex_to_index;
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
    vertex_to_index [ *start ] = i;
    outfile << i << " [label=\""<< cmg . CellSet (*start) .size () << "\"]\n";
    ++ i;
  }
  int N = cmg . NumVertices ();
  
  // LOOP THROUGH CMG EDGES
  EI estart, estop;
  typedef std::pair<int, int> int_pair;
  std::set < int_pair > edges;
  for (boost::tie ( estart, estop ) = cmg . Edges ();
       estart != estop;
       ++ estart ) {
    V source = estart -> first; 
    V target = estart -> second; 
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
      outfile << edge . first << " -> " << edge . second << ";\n";
  }
  
  edges . clear ();
  transitive_edges . clear ();

  outfile << "}\n";
  
  outfile . close ();
  
}

template < class Toplex, class CellContainer >
void output_cubes ( const Toplex & my_toplex, 
                    const ConleyMorseGraph < CellContainer , chomp::ConleyIndex_t > & conley_morse_graph ) {
  using namespace chomp;
  typedef ConleyMorseGraph < CellContainer , ConleyIndex_t > CMG;
    
  // Loop Through Morse Sets to determine bounds
  typedef typename CMG::VertexIterator VI;
  VI it, stop;
  std::vector < std::vector < uint32_t > > cubes;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices (); it != stop; ++ it ) {
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    int depth = my_toplex . getDepth ( my_subset );
    BOOST_FOREACH ( const typename Toplex::Top_Cell & ge, my_subset ) {
      my_toplex . GridElementToCubes ( & cubes, ge, depth  );
    }
  }
  std::ofstream outfile ( "morsecubes.txt" );
  for ( uint32_t i = 0; i < cubes . size (); ++ i ) {
    for ( uint32_t j = 0; j < cubes [ i ] . size (); ++ j ) {
      outfile << cubes [ i ] [ j ] << " ";
    }
    outfile << "\n";
  }
  outfile . close ();
} /* draw_morse_sets */
