/// Construct and display a conley morse graph for a given dynamical system

//#define OLD_CMAP_METHOD
//#define CMG_VISUALIZE

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <set>
#include <map>
#include "boost/tuple/tuple.hpp"

// To get SCC chatter
#define CMG_VERBOSE
//#define CMG_VISUALIZE
#define DO_CONLEY_INDEX
#define NOREACHABILITY
//#define VISUALIZE_DEBUG
//#define ILLUSTRATE

// HEADERS FOR DATA STRUCTURES
#include "chomp/Toplex.h"

#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/ConleyIndex.h"


//////////////////////////////////////BEGIN USER EDIT//////////////////////////////////////////

// MAP FUNCTION OBJECT
#include <cmath>
#include "database/maps/simple_interval.h"   // for interval arithmetic
#include "chomp/Rect.h"
#include "chomp/Prism.h"

using namespace chomp;
//using namespace capd;

// choose example

int PRESUBDIVIDE = 6;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 20 - PRESUBDIVIDE;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 26 - PRESUBDIVIDE;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


//#include "database/maps/LorenzMap.h"
//typedef LorenzMap ModelMap;

#include "database/maps/VanderPolRect.h"
typedef VanderPolRect ModelMap;


Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -10.0;// -25.0
  phase_space_bounds . upper_bounds [ 0 ] = 10.0;
  phase_space_bounds . lower_bounds [ 1 ] = -10.0;
  phase_space_bounds . upper_bounds [ 1 ] = 10.0;
  //phase_space_bounds . lower_bounds [ 2 ] = -8.0;
  //phase_space_bounds . upper_bounds [ 2 ] = 32.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( void ) {
  Rect parambox ( 1 );
  parambox . lower_bounds[0] = 2.5;
  parambox . upper_bounds[0] = 2.5;
  return parambox;
}
  

// TYPEDEFS
typedef std::vector < GridElement > CellContainer;
typedef ConleyMorseGraph < CellContainer , ConleyIndex_t > CMG;

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph );

// Declarations
void CreateDotFile ( const CMG & cmg, const std::vector < CellContainer > & marked  );

//template < class Toplex, class CellContainer >
//void output_cubes ( const Toplex & my_toplex,
//                   const ConleyMorseGraph < CellContainer , chomp::ConleyIndex_t > & conley_morse_graph );


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
  for ( int i = 0; i < PRESUBDIVIDE; ++ i ) phase_space . subdivide ();
  
  
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
  //CreateDotFile ( conley_morse_graph, marked );
  
  return 0;
} /* main */

/* ----------------------------  OUTPUT FUNCTIONS ---------------------------- */

bool cellIntersect ( const CellContainer & A, const CellContainer & B ) {
  boost::unordered_set < GridElement > As;
  BOOST_FOREACH ( const GridElement & a, A ) {
    As . insert ( a );
  }
  BOOST_FOREACH ( const GridElement & b, B ) {
    if ( As . count ( b ) != 0 ) return true;
  }
  return false;
}

#if 0
void CreateDotFile ( const CMG & cmg, 
                     const std::vector < CellContainer > & marked ) {
  typedef CMG::Vertex V;
  typedef CMG::Edge E;
  typedef CMG::VertexIterator VI;
  typedef CMG::EdgeIterator EI;

  std::ofstream outfile ("morsegraph-leslie5d.gv");
  
  outfile << "digraph G { \n";
  //outfile << "node [ shape = point, color=black  ];\n";
  //outfile << "edge [ color=red  ];\n";

  // LOOP THROUGH VERTICES AND GIVE THEM NAMES
  std::map < V, int > vertex_to_index;
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
    vertex_to_index [ *start ] = i;
    outfile << i << " [label=\""<< cmg . CellSet (*start) .size ();
    int j = 0;
    BOOST_FOREACH ( const CellContainer & cc, marked ) {
      if ( cellIntersect ( cc, cmg . CellSet ( *start ) ) ) {
        outfile << "," << j;
        }
      ++ j;
    }
    outfile << "\"]\n";
    
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
    //std::cout << "I see (" << source << ", " << target << ")\n";
    int index_source = vertex_to_index [ source ];
    int index_target = vertex_to_index [ target ];
    //std::cout << "I see indexed (" << index_source << ", " << index_target << ")\n";

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

#endif

// HEADERS FOR DEALING WITH PICTURES
#include "database/tools/picture.h"
#include "database/tools/lodepng/lodepng.h"

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

#if 0
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

#endif
