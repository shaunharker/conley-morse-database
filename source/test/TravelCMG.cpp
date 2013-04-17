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

//#define CMG_VERBOSE
#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH

#include "database/structures/SuccinctGrid.h"
#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/Rect.h"

/*************************/
/* Subdivision Settings  */
/*************************/

using namespace chomp;
int INITIALSUBDIVISIONS = 20;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 28 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 32 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


/**************************************************/
/* Map, Parameter Space and Phase space Settings  */
/**************************************************/

#include "data/HollingPredatorPrey/ModelMap.h"

Rect initialize_phase_space_box ( void ) {
  int phase_space_dimension = 4;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -0.1;
  phase_space_bounds . upper_bounds [ 0 ] =  1.9;
  phase_space_bounds . lower_bounds [ 1 ] = -2.0;
  phase_space_bounds . upper_bounds [ 1 ] =  2.0;
  phase_space_bounds . lower_bounds [ 2 ] = -0.1;
  phase_space_bounds . upper_bounds [ 2 ] =  1.9;
  phase_space_bounds . lower_bounds [ 3 ] = -2.0;
  phase_space_bounds . upper_bounds [ 3 ] =  2.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( void) {
  int parameter_space_dimension = 6;
  Rect parameter_space_limits ( parameter_space_dimension );
  parameter_space_limits . lower_bounds [ 0 ] = 1;
  parameter_space_limits . upper_bounds [ 0 ] = 1;
  parameter_space_limits . lower_bounds [ 1 ] = 0.25;
  parameter_space_limits . upper_bounds [ 1 ] = 0.25;
  parameter_space_limits . lower_bounds [ 2 ] = 1.0;
  parameter_space_limits . upper_bounds [ 2 ] = 1.0;
  parameter_space_limits . lower_bounds [ 3 ] = 0.5;
  parameter_space_limits . upper_bounds [ 3 ] = 0.5;
  parameter_space_limits . lower_bounds [ 4 ] = 0.5;
  parameter_space_limits . upper_bounds [ 4 ] = 0.5;
  parameter_space_limits . lower_bounds [ 5 ] = 2.0;
  parameter_space_limits . upper_bounds [ 5 ] = 2.0;
  std::cout << "Parameter Space Bounds = " << parameter_space_limits << "\n";
  return parameter_space_limits;
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
  SuccinctGrid phase_space;
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
  
  
  return 0;
} /* main */


