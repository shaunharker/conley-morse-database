
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
//#define ODE_METHOD

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/RectGeo.h"
#include "database/tools/SingleOutput.h"
//#include "cmgtools.h"


/*************************/
/* Subdivision Settings  */
/*************************/

#ifndef GRIDCHOICE
#define GRIDCHOICE PointerGrid
#endif

#include "database/structures/PointerGrid.h"
//#include "database/structures/SuccinctGrid.h"

using namespace chomp;
// PARAMETERS
int order = 10;
double integrationTime = 1./8.;
double tolerance = 1.e-4;  // error tolerance for one step
int minSub = 20;
int maxSub = 25;
 
int INITIALSUBDIVISIONS = 10;
#define SINGLECMG_MIN_PHASE_SUBDIVISIONS  minSub - INITIALSUBDIVISIONS;
#define SINGLECMG_MAX_PHASE_SUBDIVISIONS  maxSub - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 30000;

/**************/
/*  EXAMPLES  */
/**************/
  
// choose example by uncommenting define
#define VANDERPOL_TK
// #define GOLDBETER_LEFEVER

#ifdef VANDERPOL_TK
// time ./odeCMG 20 25 10 0.4   - 2m52s

#include "database/maps/VanderPolTK.h"
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

  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box (int noParams, Real * params) {
  Rect parambox ( 1 );
  parambox . lower_bounds[0] = 2.5;
  parambox . upper_bounds[0] = 2.5;
  return parambox;
}

#endif
#ifdef GOLDBETER_LEFEVER
#include "GoldbeterLefever.h"
typedef GoldbeterLefever ModelMap;

Rect initialize_phase_space_box ( void ) {
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension ); 
  phase_space_bounds . lower_bounds [ 0 ] = 0;// -25.0
  phase_space_bounds . upper_bounds [ 0 ] = 40.0;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 20.0;

  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box (int noParams, Real * params) {
  Rect parambox ( 3 );
  parambox . lower_bounds[0] = 200;
  parambox . upper_bounds[0] = 200;
  parambox . lower_bounds[1] = 120;
  parambox . upper_bounds[1] = 120;
  parambox . lower_bounds[2] = 1;
  parambox . upper_bounds[2] = 1;
  return parambox;
}

#endif
////////////////////////////////////////END USER EDIT//////////////////////////////////////////

void write_params(std::ostream & out, int n, Real * params){
  out << "Parameters : \n subdivisions : " << minSub << " - " << maxSub 
                << "  initial : " << INITIALSUBDIVISIONS<< "\n" ;
  out << " order : " << order << "  itegration time : " << integrationTime << "\n";
  for(int i=0; i+1<n; i+=2){
    out << "[" << params[i] << ", " <<  params[i+1] << "]\n";
  }
}
/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] )
{
  
  clock_t start, stop;
  start = clock ();
  
  if(argc>1 && argv[1][0]=='h'){
    std::cout << "USAGE:\n odeCMG  minSub maxSub initSub order integrationTime tolerance\n\n";
    exit(0);
  }
  if(argc>1) minSub = (int)atoi(argv[1]);
  if(argc>2) maxSub = (int)atoi(argv[2]);
  if(argc>3) INITIALSUBDIVISIONS = (int)atoi(argv[3]);
  if(argc>4) order  = (int)atoi(argv[4]);
  if(argc>5) integrationTime = (double) atof(argv[5]);
  if(argc>6) tolerance = (double) atof(argv[6]);

  Real * params = 0;
  int noParams = 0;
//  int noParams = argc-3;
//  if(noParams > 0){
//    params = new Real[noParams];
//    
//  /* READ TWO INPUTS (which will give a parameter space box) */
//    std::cout << "\n argc = " << argc << std::endl;
//    for(int i=0; i<noParams; ++i){
//      params[i] = ( Real ) atof ( argv [ i+3 ] );
//      std::cout << "  " << params[i] ;
//    }
//  }
  write_params(std::cout, noParams, params );  
  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();
  
  /* SET PARAMETER SPACE REGION */
  Rect parameter_box = initialize_parameter_space_box (noParams, params);
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  MorseGraph morsegraph ( new PHASE_GRID ); 
  
  /* INITIALIZE PHASE SPACE */
  boost::shared_ptr<TreeGrid> phaseSpace = 
    boost::dynamic_pointer_cast<TreeGrid> ( morsegraph . phaseSpace () );
  phaseSpace -> initialize ( phase_space_bounds );
  
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )
    phaseSpace -> subdivide ();
  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box, order, integrationTime, tolerance );
    
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & morsegraph,
                       phaseSpace,
                       map,
                       minSub - INITIALSUBDIVISIONS,
                       //SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       maxSub - INITIALSUBDIVISIONS,
                       //SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  
  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " <<
  (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  std::cout << "Creating image files...\n";

  DrawMorseSets ( * boost::dynamic_pointer_cast<TreeGrid> 
                  (morsegraph . phaseSpace ()), 
                     morsegraph );
  //DrawMorseSets ( *phase_space, conley_morse_graph );
 
//  std::cout << "Writing recurrent sets to the file\n";

//  std::ofstream file2("boxes2.txt");
//  write_params(file2, noParams, params);
//  write_morse_sets_notrefined(file2, *phase_space, conley_morse_graph);
//  file2.close();

  std::cout << "Creating DOT file...\n";
  CreateDotFile ( morsegraph );
  
  return 0;
} /* main */
