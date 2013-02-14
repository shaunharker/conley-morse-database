/// Construct and display a conley morse graph for a given dynamical system

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <set>
#include <map>

// To get SCC chatter
//#define CMG_VERBOSE
//#define DO_CONLEY_INDEX
#define NO_REACHABILITY
#define NOREACHABILITY
//#define VISUALIZE_DEBUG
//#define ILLUSTRATE
#define CMDB_STORE_GRAPH

// HEADERS FOR DATA STRUCTURES
#include "chomp/Toplex.h"

#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"

// HEADERS FOR DEALING WITH PICTURES
#include "database/tools/picture.h"
#include "database/tools/lodepng/lodepng.h"

// MAP FUNCTION OBJECT
#include <cmath>

#include "chomp/Rect.h"
#include "chomp/Prism.h"

using namespace chomp;

int INITIALSUBDIVISIONS = 19;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 32 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 40 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


#include "data/travellingWave/ModelMap.h"

Rect initialize_phase_space_box ( void ) {
  int phase_space_dimension = 4;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -2.0;
  phase_space_bounds . upper_bounds [ 0 ] =  2.0;
  phase_space_bounds . lower_bounds [ 1 ] = -2.0;
  phase_space_bounds . upper_bounds [ 1 ] =  2.0;
  phase_space_bounds . lower_bounds [ 2 ] = -2.0;
  phase_space_bounds . upper_bounds [ 2 ] =  2.0;
  phase_space_bounds . lower_bounds [ 3 ] = -2.0;
  phase_space_bounds . upper_bounds [ 3 ] =  2.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( void) {
  int parameter_space_dimension = 4;
  Rect parameter_space_limits ( parameter_space_dimension );
  parameter_space_limits . lower_bounds [ 0 ] = 1.65;
  parameter_space_limits . upper_bounds [ 0 ] = 1.65;
  parameter_space_limits . lower_bounds [ 1 ] = 0.25;
  parameter_space_limits . upper_bounds [ 1 ] = 0.25;
  parameter_space_limits . lower_bounds [ 2 ] = 0.8;
  parameter_space_limits . upper_bounds [ 2 ] = 0.8;
  parameter_space_limits . lower_bounds [ 3 ] = 0.5;
  parameter_space_limits . upper_bounds [ 3 ] = 0.5;
  std::cout << "Parameter Space Bounds = " << parameter_space_limits << "\n";
  return parameter_space_limits;
}

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
  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();

  /* SET PARAMETER SPACE REGION */
  Rect parameter_box = initialize_parameter_space_box ();

  /* INITIALIZE PHASE SPACE */
  Toplex phase_space;
  phase_space . initialize ( phase_space_bounds );
  
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )
    phase_space . subdivide ();
  
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

  //output_cubes ( phase_space, conley_morse_graph );
  
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
