/// Construct and display a conley morse graph for a given dynamical system

//#define VISUALIZE_DEBUG

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>

//#define VISUALIZE_DEBUG
//#define RGVISUALIZE_DEBUG
// HEADERS FOR DATA STRUCTURES
#include "data_structures/Conley_Morse_Graph.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"

// HEADERS FOR ALGORITHMS
#define PHASE_SUBDIVISIONS 12
#define COMPLEXITY_LIMIT 2000000

#define NEW_GRAPH_THEORY

#ifndef NEW_GRAPH_THEORY
#include "program/jobs/Compute_Conley_Morse_Graph3.h"
#else
#include "program/jobs/Compute_Conley_Morse_Graph4.h"
#endif

// HEADER FOR MAP FILE
#include "maps/leslie.h"
//#include "maps/fisheries.h"

// HEADERS FOR DEALING WITH PICTURES
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"

// TYPEDEFS
#ifdef NEW_GRAPH_THEORY
typedef std::vector < Adaptive_Cubical::Toplex::Top_Cell > CellContainer;
#else
typedef Adaptive_Cubical::Toplex::Subset CellContainer;
#endif

typedef ConleyMorseGraph < CellContainer , Conley_Index_t > CMG;

// FUNCTION DECLARATIONS
Adaptive_Cubical::Geometric_Description initialize_phase_space_box ( const int bx, const int by );
Adaptive_Cubical::Geometric_Description initialize_parameter_space_box ( const int bx, const int by );
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
    std::cout << "Input two integers in [0, 64)\n";
    return 0;
  }
  Adaptive_Cubical::Real bx = ( Adaptive_Cubical::Real ) atoi ( argv [ 1 ] ); //37.0;
  Adaptive_Cubical::Real by = ( Adaptive_Cubical::Real ) atoi ( argv [ 2 ] ); //17.0;
  
  
  /* INITIALIZE PHASE SPACE (create a single box, which will be subdivided later) */
  Adaptive_Cubical::Toplex phase_space;
  Adaptive_Cubical::Geometric_Description phase_box = initialize_phase_space_box (bx, by);
  /* fishery example
   Adaptive_Cubical::Geometric_Description phase_box (4);
  phase_box . lower_bounds [ 0 ] = 0;
  phase_box . lower_bounds [ 1 ] = 0;
  phase_box . lower_bounds [ 2 ] = 0;
  phase_box . upper_bounds [ 0 ] = 5.0;
  phase_box . upper_bounds [ 1 ] = 5.0;
  phase_box . upper_bounds [ 2 ] = 5.0;
  */
  phase_space . initialize ( phase_box );
  
  /* INITIALIZE PARAMETER SPACE REGION */
  Adaptive_Cubical::Geometric_Description parameter_box = initialize_parameter_space_box (bx, by);
  // FISHERY Adaptive_Cubical::Geometric_Description parameter_box (1);

  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG conley_morse_graph;
  
  /* COMPUTE CONLEY MORSE GRAPH */
#ifdef NEW_GRAPH_THEORY
  Compute_Conley_Morse_Graph4 < CMG, Adaptive_Cubical::Toplex, Adaptive_Cubical::Toplex, LeslieMap/*FishMap4*/ >
    ( & conley_morse_graph, & phase_space, parameter_box, true, false );
#else
  Compute_Conley_Morse_Graph3 < CMG, Adaptive_Cubical::Toplex, Adaptive_Cubical::Toplex, LeslieMap/*FishMap4*/ >
  ( & conley_morse_graph, & phase_space, parameter_box, true, false );
#endif
  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets, Connections, and Conley Indices: " << 
    (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  /* DRAW MORSE SETS */
  DrawMorseSets ( phase_space, conley_morse_graph );

  /* OUTPUT MORSE GRAPH */
  CreateDotFile ( conley_morse_graph );
  return 0;
} /* main */


Adaptive_Cubical::Geometric_Description initialize_phase_space_box ( const int bx, const int by ) {
  Adaptive_Cubical::Geometric_Description phase_space_box ( 2 , 0 , 300 );
  phase_space_box . lower_bounds [ 0 ] = -0.001;
  phase_space_box . upper_bounds [ 0 ] = 320.056;
  phase_space_box . lower_bounds [ 1 ] = -0.001;
  phase_space_box . upper_bounds [ 1 ] = 224.040;
  return phase_space_box;
}

Adaptive_Cubical::Geometric_Description initialize_parameter_space_box ( const int bx, const int by ) {
  Adaptive_Cubical::Geometric_Description parameter_space_limits ( 2 , 0 , 300 );
  parameter_space_limits . lower_bounds [ 0 ] = 8.0;
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  
  
  Adaptive_Cubical::Geometric_Description parameter_box ( 2 , 20.01 , 20.02 );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / 50.0;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / 50.0;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / 50.0;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / 50.0;
    
  return parameter_box;
}

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph ) {
  // Create a Picture
  
  int Width = 512;
  int Height = 512;
  
  /*
  Picture * picture = new Picture( Width, Height, 0.0, 0.0, 0.0, 0.0 ); // this assures I have the origin in current imp

  typedef CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    //draw_ascii_subset ( phase_space, conley_morse_graph . CubeSet ( * it ) );
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    Picture * morse_picture = draw_picture ( Width, Height, 
                                            Red, Green, Blue, 
                                            phase_space, conley_morse_graph . CubeSet ( *it ) );
    Picture * combination = combine_pictures ( Width, Height, *picture, *morse_picture );
    std::swap ( picture, combination );
    delete morse_picture;
    delete combination;
  }
   */
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
  //std::vector < V > index_to_vertex;
  //index_to_vertex . resize ( cmg . NumVertices () );
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices ();
       start != stop;
       ++ start ) {
    vertex_to_index [ *start ] = i;
    //index_to_vertex [ i ] = *start;
    ++ i;
  }
  
  // LOOP THROUGH EDGES
  EI estart, estop;
  for (boost::tie ( estart, estop ) = cmg . Edges ();
       estart != estop;
       ++ estart ) {
    V source = cmg . Source ( *estart );
    V target = cmg . Target ( *estart );
    int index_source = vertex_to_index [ source ];
    int index_target = vertex_to_index [ target ];
    outfile << index_source << " -> " << index_target << ";\n";
  }
  outfile << "}\n";
  
  outfile . close ();
  
}


