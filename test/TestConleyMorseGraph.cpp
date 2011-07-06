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

#define PARAMETER_BOXES 50
// HEADERS FOR ALGORITHMS
#define MIN_PHASE_SUBDIVISIONS 12
#define MAX_PHASE_SUBDIVISIONS 16
#define SPACE_DIMENSION 3
#define COMPLEXITY_LIMIT 10000

// HEADER FOR MAP FILE
#define FISHMODEL
#include "maps/leslie.h"
//#include "maps/fisheries.h"


#include "program/jobs/Compute_Conley_Morse_Graph.h"



// HEADERS FOR DEALING WITH PICTURES
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"

// TYPEDEFS
typedef std::vector < Adaptive_Cubical::Toplex::Top_Cell > CellContainer;


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

  phase_space . initialize ( phase_box );
  
  /* INITIALIZE PARAMETER SPACE REGION */
  Adaptive_Cubical::Geometric_Description parameter_box = initialize_parameter_space_box (bx, by);

  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG conley_morse_graph;
  
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Conley_Morse_Graph < CMG, Adaptive_Cubical::Toplex, Adaptive_Cubical::Toplex, 
                              LeslieMap /*FishMap4*/ >
    ( & conley_morse_graph, & phase_space, parameter_box, true, false );

  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " << 
    (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  /* DRAW MORSE SETS */
  DrawMorseSets ( phase_space, conley_morse_graph );

  /* OUTPUT MORSE GRAPH */
  CreateDotFile ( conley_morse_graph );
  return 0;
} /* main */


Adaptive_Cubical::Geometric_Description initialize_phase_space_box ( const int bx, const int by ) {
  Adaptive_Cubical::Geometric_Description phase_space_box ( SPACE_DIMENSION , 0 , (double)4000.0/(double)2.718  );
#ifndef FISHMODEL
  phase_space_box . lower_bounds [ 0 ] = 0.0;
  phase_space_box . upper_bounds [ 0 ] = 320.056;

  phase_space_box . lower_bounds [ 1 ] = 0.0;
  phase_space_box . upper_bounds [ 1 ] = 224.040;
#endif
 
  std::cout << "Phase Space = " << phase_space_box << "\n";
  return phase_space_box;
}

Adaptive_Cubical::Geometric_Description initialize_parameter_space_box ( const int bx, const int by ) {
  Adaptive_Cubical::Geometric_Description parameter_space_limits ( 2 , 0 , 300 );
  parameter_space_limits . lower_bounds [ 0 ] = 0.0;//8.0;
  parameter_space_limits . upper_bounds [ 0 ] = 0.0;//37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 0.0;//3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 0.0;//50.0;
  
  
  Adaptive_Cubical::Geometric_Description parameter_box ( 2 , 20.01 , 20.02 );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box = " << parameter_box << "\n";

  return parameter_box;
}

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph ) {
  // Create a Picture
  
  int Width =  256;
  int Height = 256;
  
  /*
  Picture * picture = new Picture( Width, Height, 0.0, 0.0, 0.0, 0.0 ); // this assures I have the origin in current imp

  typedef CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    //draw_ascii_subset ( phase_space, conley_morse_graph . CellSet ( * it ) );
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    Picture * morse_picture = draw_picture ( Width, Height, 
                                            Red, Green, Blue, 
                                            phase_space, conley_morse_graph . CellSet ( *it ) );
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


