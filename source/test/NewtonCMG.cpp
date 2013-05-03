
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
#define MEMORYBOOKKEEPING
//#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/ConleyIndex.h"
#include "chomp/Rect.h"

#include "database/tools/SingleOutput.h"


/*************************/
/* Subdivision Settings  */
/*************************/

#undef GRIDCHOICE

#include "database/structures/PointerGrid.h"
#ifdef USE_SUCCINCT
#define GRIDCHOICE SuccinctGrid
#include "database/structures/SuccinctGrid.h"
#else
#define GRIDCHOICE PointerGrid
#include "database/structures/PointerGrid.h"
#endif

using namespace chomp;
int INITIALSUBDIVISIONS = 0;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 13 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 18 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 100;


/**************/
/*    MAP     */
/**************/
#include "/Users/sharker/CMDBFiles/Wes/2D/7-13/ModelMap.h"
//#include "data/newton2d/ModelMap.h"

Rect initialize_phase_space_box ( void ) {
  const Real pi = 3.14159265358979323846264338327950288;
  int phase_space_dimension = 1;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -pi;
  phase_space_bounds . upper_bounds [ 0 ] = pi ;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( void ) {
  int parameter_space_dimension = 2;
  //[-0.84375, -0.828125]x[0.245437, 0.269981]
  
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = -0.84375;
  parameter_space_limits . upper_bounds [ 0 ] = -0.828125;
  parameter_space_limits . lower_bounds [ 1 ] = 0.245437;
  parameter_space_limits . upper_bounds [ 1 ] = 0.269981;
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
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  phase_space -> initialize ( phase_space_bounds );
  
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )
    phase_space -> subdivide ();
  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box );
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  MorseGraph mg;
  
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & mg,
                       phase_space,
                       map,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  
  typedef std::vector < Grid::GridElement > Subset;
  for ( size_t v = 0; v < mg . NumVertices (); ++ v) {
    Subset subset = phase_space -> subset ( * mg . grid ( v ) );
    std::cout << "Calling Conley_Index on Morse Set " << v << "\n";
    ConleyIndex_t conley;
    ConleyIndex ( & conley,
                 *phase_space,
                 subset,
                 map );
    
  }

  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " <<
  (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  std::cout << "Creating image files...\n";
  //DrawMorseSets ( *phase_space, mg );
  std::cout << "Creating DOT file...\n";
  CreateDotFile ( mg );
  
  return 0;
} /* main */

