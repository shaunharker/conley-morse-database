/// Construct and display a conley morse graph for a given dynamical system
// STANDARD HEADERS
#include <iostream>
#include <fstream>

// HEADERS FOR DATA STRUCTURES
#include "data_structures/Conley_Morse_Graph.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"

// HEADERS FOR ALGORITHMS
#define PHASEDEPTH 15
#include "program/jobs/Compute_Conley_Morse_Graph.h"
#include "program/jobs/Clutching_Graph_Job.h"

// HEADER FOR MAP FILE
#include "maps/leslie.h"

// HEADERS FOR DEALING WITH PICTURES
#include "tools/picture.h"
#include "tools/lodepng/lodepng.h"

// STANDARD INCLUDES
#include <set>
#include "boost/foreach.hpp"
#include <cstring>

// TYPEDEFS
typedef ConleyMorseGraph < Adaptive_Cubical::Toplex::Subset , Conley_Index_t > CMG;
typedef std::set< std::pair < CMG::Vertex, CMG::Vertex > > ClutchingGraph;

// FUNCTION DECLARATIONS
Adaptive_Cubical::Geometric_Description initialize_phase_space_box ( void );
Adaptive_Cubical::Geometric_Description initialize_parameter_space_box ( const int bx, const int by );
void DrawMorseSets ( const Toplex & phase_space, const CMG & cmg, const char * fileprefix );
void CreateDotFile ( const CMG & cmg );
void CreateClutchDotFile ( const CMG & cmg1, const CMG & cmg2, const ClutchingGraph & clutch,
                          const char * filename );

// MAIN PROGRAM
int main ( int argc, char * argv [] ) 
{
  
  /* READ TWO INPUTS (which will give a parameter space box) */
  
  if ( argc != 3 ) {
    std::cout << "Usage: Supply 2 (not " << argc << ") arguments:\n";
    std::cout << "Input two integers in [0, 64)\n";
    return 0;
  }
  Adaptive_Cubical::Real bx = ( Adaptive_Cubical::Real ) atoi ( argv [ 1 ] ); //37.0;
  Adaptive_Cubical::Real by = ( Adaptive_Cubical::Real ) atoi ( argv [ 2 ] ); //17.0;
  
  
  /* INITIALIZE PHASE SPACE (create a single box, which will be subdivided later) */
  Adaptive_Cubical::Toplex phase_space1, phase_space2;
  Adaptive_Cubical::Geometric_Description phase_box = initialize_phase_space_box ();
  phase_space1 . initialize ( phase_box );
  phase_space2 . initialize ( phase_box );
  
  /* INITIALIZE PARAMETER SPACE REGION */
  Adaptive_Cubical::Geometric_Description parameter_box1 = initialize_parameter_space_box (bx, by);
  Adaptive_Cubical::Geometric_Description parameter_box2 = initialize_parameter_space_box (bx + 1, by);

  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG cmg1, cmg2;
  
  /* COMPUTE CONLEY MORSE GRAPHS */
  Compute_Conley_Morse_Graph < CMG, Adaptive_Cubical::Toplex, Adaptive_Cubical::Toplex, LeslieMap >
    ( & cmg1, & phase_space1, parameter_box1 );
  Compute_Conley_Morse_Graph < CMG, Adaptive_Cubical::Toplex, Adaptive_Cubical::Toplex, LeslieMap >
  ( & cmg2, & phase_space2, parameter_box2 );
  
  /* COMPUTE CLUTCHING RELATIONSHIP */
  //ClutchingGraph clutch;
  ClutchingGraph clutch;
  std::cout << "Calculation Clutching Relationship.\n";
  ClutchingTwoGraphs < CMG, Adaptive_Cubical::Toplex > ( &clutch, cmg1, cmg2, phase_space1, phase_space2 );
  
  /* DRAW MORSE SETS */
  //std::cout << "Drawing Morse Sets to various files.\n";
  DrawMorseSets ( phase_space1, cmg1, "TBC1" );
  DrawMorseSets ( phase_space2, cmg2, "TBC2" );

  /* OUTPUT MORSE GRAPH */
  std::cout << "Drawing Morse Graph to file clutching.gv\n";
  CreateClutchDotFile ( cmg1, cmg2, clutch, "clutching.gv" );
  
  return 0;
} /* main */


Adaptive_Cubical::Geometric_Description initialize_phase_space_box () {
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

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph, const char * fileprefix ) {
  // Create a Picture
  std::cout << "Drawing Morse Sets to File\n";
  int Width = 4096;
  int Height = 4096;
  
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
  char filename [ 80 ];
  Picture * picture = draw_morse_sets ( Width, Height, phase_space, conley_morse_graph );
  strcpy ( filename, fileprefix );
  strcat ( filename, "_morse_sets.png");
  LodePNG_encode32_file( filename, picture -> bitmap, picture -> Width, picture -> Height);
  
  Picture * picture2 = draw_toplex ( Width, Height, phase_space );
  strcpy ( filename, fileprefix );
  strcat ( filename, "_toplex.png");
  LodePNG_encode32_file( filename, picture2 -> bitmap, picture2 -> Width, picture2 -> Height);
  
  Picture * picture3 = draw_toplex_and_morse_sets ( Width, Height, phase_space, conley_morse_graph );
  strcpy ( filename, fileprefix );
  strcat ( filename, "_toplex_and_morse.png");
  LodePNG_encode32_file( filename, picture3 -> bitmap, picture3 -> Width, picture3 -> Height);

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

void CreateClutchDotFile ( const CMG & cmg1, const CMG & cmg2, const ClutchingGraph & clutch,
                           const char * filename ) {
  typedef CMG::Vertex V;
  typedef CMG::Edge E;
  typedef CMG::VertexIterator VI;
  typedef CMG::EdgeIterator EI;
  
  std::ofstream outfile ( filename );
  
  outfile << "digraph G { \n";
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
  outfile << " subgraph cluster0 {\n color=blue; ";
  
  int i = 0;
  for (boost::tie ( start, stop ) = cmg1 . Vertices ();
       start != stop;
       ++ start ) {
    vertex_to_index1 [ *start ] = i;
    outfile << i << " [label=\""<< cmg1.CellSet(*start).size() << "\"]\n";
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
  BOOST_FOREACH ( int_pair edge, edges ) {
    if ( transitive_edges . count ( edge ) == 0 )
      outfile << edge . first << " -> " << edge . second << ";\n";
  }
  
  edges . clear ();
  transitive_edges . clear ();
  
  outfile << " }\n subgraph cluster1 {\n color=blue;\n";
  for (boost::tie ( start, stop ) = cmg2 . Vertices ();
       start != stop;
       ++ start ) {
    vertex_to_index2 [ *start ] = i;
    outfile << i << " [label=\""<< cmg2.CellSet(*start).size() << "\"]\n";
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
  BOOST_FOREACH ( int_pair edge, edges ) {
    if ( transitive_edges . count ( edge ) == 0 )
      outfile << edge . first << " -> " << edge . second << ";\n";
  }
  
  // LOOP THROUGH CLUTCHING EDGES
  outfile << "}\n edge [dir=none,color=red];\n";
  typedef std::pair<V,V> VertexPair;
  BOOST_FOREACH ( const VertexPair & edge, clutch ) {
    int index_source = vertex_to_index1 [ edge . first ];
    int index_target = vertex_to_index2 [ edge . second ];
    outfile << index_source << " -> " << index_target << ";\n";
    
  }
  outfile << "}\n";
  
  outfile . close ();
  
}
