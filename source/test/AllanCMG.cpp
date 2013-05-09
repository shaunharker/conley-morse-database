/// Construct and display a conley morse graph for a given dynamical system

/********************/
/* Standard Headers */
/********************/
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <set>
#include <map>

/***************************/
/* Preprocessor directives */
/***************************/

#define CMG_VERBOSE
#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
#define ODE_METHOD

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/Rect.h"
#include "database/tools/SingleOutput.h"

/*************************/
/* Subdivision Settings  */
/*************************/

#ifndef GRIDCHOICE
#define GRIDCHOICE PointerGrid
#endif

#include "database/structures/PointerGrid.h"
//#include "database/structures/SuccinctGrid.h"

using namespace chomp;
int INITIALSUBDIVISIONS = 8;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 30 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 36 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


/**************************************************/
/* Map, Parameter Space and Phase space Settings  */
/**************************************************/

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


/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] )
{
  
  clock_t start, stop;
  start = clock ();
  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();
  
  /* SET PARAMETER SPACE REGION */
  Rect parameter_box = initialize_parameter_space_box ();
  
  /* INITIALIZE PHASE SPACE */
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  phase_space -> initialize ( phase_space_bounds );
  
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )
    phase_space -> subdivide ();
  
  /* INITIALIZE MAP */
  std::vector < ModelMap > maps;
  for ( double timestep = 1.0/(64.0*256.0); timestep < 1.0; timestep *= 2.0 ) {
    ModelMap map ( parameter_box, timestep );
    maps . push_back ( map );
  }
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  MorseGraph conley_morse_graph;
  
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph,
                       phase_space,
                       maps,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  
  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " <<
  (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  std::cout << "Creating PNG file...\n";
  DrawMorseSets ( *phase_space, conley_morse_graph );
  std::cout << "Creating DOT file...\n";
  CreateDotFile ( conley_morse_graph );

  return 0;
} /* main */
