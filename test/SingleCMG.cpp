/// Construct and display a conley morse graph for a given dynamical system

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <set>
#include <map>

// To get SCC chatter
#define CMG_VERBOSE 
#define DO_CONLEY_INDEX
//#define VISUALIZE_DEBUG
//#define ILLUSTRATE

// HEADERS FOR DATA STRUCTURES
#include "data_structures/Conley_Morse_Graph.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "program/jobs/Compute_Morse_Graph.h"

// HEADERS FOR DEALING WITH PICTURES
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"

using namespace Adaptive_Cubical;

//////////////////////////////////////BEGIN USER EDIT//////////////////////////////////////////

// MAP FUNCTION OBJECT
#include "maps/simple_interval.h"   // for interval arithmetic

struct LeslieMap {
  
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  LeslieMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Adaptive_Cubical::Geometric_Description operator () 
  ( const Adaptive_Cubical::Geometric_Description & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * exp ( (double) -0.1 * (x0 + x1) );     
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    Adaptive_Cubical::Geometric_Description return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();

    return return_value;
  } 
  
};

Geometric_Description initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  int phase_space_dimension = 2;
  Geometric_Description phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 224.040;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Geometric_Description initialize_parameter_space_box ( const int bx, const int by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Geometric_Description parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  
  // Use command line arguments to choose a box from 50 x 50 choices.
  int PARAMETER_BOXES = 64;
  Geometric_Description parameter_box ( parameter_space_dimension );
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

int MIN_PHASE_SUBDIVISIONS = 12;
int MAX_PHASE_SUBDIVISIONS = 16;
int COMPLEXITY_LIMIT = 10000;

////////////////////////////////////////END USER EDIT//////////////////////////////////////////

// TYPEDEFS
typedef std::vector < Toplex::Top_Cell > CellContainer;
typedef ConleyMorseGraph < CellContainer , Conley_Index_t > CMG;

// Declarations
void DrawMorseSets ( const Toplex & phase_space, const CMG & cmg );
void CreateDotFile ( const CMG & cmg );

// MAIN PROGRAM
int main ( int argc, char * argv [] ) 
{
  
  clock_t start, stop;
  start = clock ();
  
  /* READ TWO INPUTS (which will give a parameter space box) */
  if ( argc != 3 ) {
    std::cout << "Usage: Supply 2 (not " << argc << ") arguments:\n";
    std::cout << "Input two integers in [0, " << 50 << "\n";
    return 0;
  }
  Real bx = ( Real ) atoi ( argv [ 1 ] );
  Real by = ( Real ) atoi ( argv [ 2 ] );
  
  /* SET PHASE SPACE REGION */
  Geometric_Description phase_space_bounds = initialize_phase_space_box ();

  /* SET PARAMETER SPACE REGION */
  Geometric_Description parameter_box = initialize_parameter_space_box (bx, by);

  /* INITIALIZE PHASE SPACE */
  Toplex phase_space;
  phase_space . initialize ( phase_space_bounds );

  /* INITIALIZE MAP */
  LeslieMap map ( parameter_box );
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG conley_morse_graph;

  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph, 
                        & phase_space, 
                        map, 
                        MIN_PHASE_SUBDIVISIONS, 
                        MAX_PHASE_SUBDIVISIONS, 
                        COMPLEXITY_LIMIT );

  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " << 
    (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  /* DRAW MORSE SETS */
  DrawMorseSets ( phase_space, conley_morse_graph );

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


