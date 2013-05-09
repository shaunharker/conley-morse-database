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

#define WIDTH 1.0/512.0

#define CMG_VERBOSE
#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

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
int INITIALSUBDIVISIONS = 30;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 32 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 34 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


/**************************************************/
/* Map, Parameter Space and Phase space Settings  */
/**************************************************/

#include "database/maps/Selkov.h"
typedef MapRect ModelMap;


Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 1.4;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 1.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( void ) {
  Rect parambox ( 4 );
  // nu
  parambox . lower_bounds[0] = 0.0285;
  parambox . upper_bounds[0] = 0.0285;
  // eta
  parambox . lower_bounds[1] = 0.1;
  parambox . upper_bounds[1] = 0.1;
  // alpha
  parambox . lower_bounds[2] = 1.0;
  parambox . upper_bounds[2] = 1.0;
   // gamma
  parambox . lower_bounds[3] = 2.0;
  parambox . upper_bounds[3] = 2.0;
  
  return parambox;
}


/*******************/
/* Map of Interest */
/*******************/
ModelMap generateMap ( double timestep ) {
  Rect parameter_box = initialize_parameter_space_box ();
  ModelMap map ( parameter_box, timestep );
  return map;
}

/************************/
/* Generate Time Series */
/************************/
std::vector < chomp::Rect > generateTimeSeries ( const chomp::Rect & x0,
                                                 const ModelMap & map, int N,
                                                 int tail = 0 ) {
  std::cout << "Generating time series.\n";

  std::vector < chomp::Rect > result;
  result . reserve ( N - tail );
  chomp::Rect x = x0;
  int percent= 0;
  for ( int i = 0; i < N; ++ i ) {
    int newpercent = (i*100)/N;
    if ( newpercent > percent ) {
      std::cout << "\r            \r" << newpercent << "%";
      std::cout . flush ();
      percent = newpercent;
    }
    if ( i >= tail ) result . push_back ( x );
    x = map ( x );
    x . lower_bounds [ 0 ] = x . upper_bounds [ 0 ] = (x . lower_bounds [ 0 ] + x . upper_bounds [ 0 ])/2.0;
    x . lower_bounds [ 1 ] = x . upper_bounds [ 1 ] = (x . lower_bounds [ 1 ] + x . upper_bounds [ 1 ])/2.0;
  }
  std::cout << "\rTime Series Generated.\n";
  return result;
}

/********************************/
/* Generate Initial Phase Space */
/********************************/
boost::shared_ptr<GRIDCHOICE> generateInitialPhaseSpace ( const std::vector < chomp::Rect > & time_series ) {
  std::cout << "Generating Initial Phase Space.\n";

  chomp::Rect phase_space_bounds = initialize_phase_space_box ();
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  phase_space -> initialize ( phase_space_bounds );
  for ( int subdivision_depth = 0; subdivision_depth < INITIALSUBDIVISIONS; ++ subdivision_depth ) {
    std::cout << "\r          Depth = " << subdivision_depth;
    boost::unordered_set < Grid::GridElement > subset_of_time_series;
    std::insert_iterator < boost::unordered_set < Grid::GridElement > >
      tsii ( subset_of_time_series, subset_of_time_series . begin () );
    int percent= 0;
    size_t n = time_series . size ();
    size_t i = 0;
    BOOST_FOREACH ( const chomp::Rect & rect, time_series ) {
      int newpercent = (i++ * 100)/n;
      if ( newpercent > percent ) {
        std::cout << "\r" << newpercent << "%";
        std::cout . flush ();
        percent = newpercent;
      }

      chomp::Rect collared_rect = rect;
      collared_rect . lower_bounds [ 0 ] -= WIDTH;
      collared_rect . upper_bounds [ 0 ] += WIDTH;
      collared_rect . lower_bounds [ 1 ] -= WIDTH;
      collared_rect . upper_bounds [ 1 ] += WIDTH;
      
      phase_space -> cover ( tsii, collared_rect );
    }
    std::deque < Grid::GridElement > deque_of_time_series;
    BOOST_FOREACH ( const Grid::GridElement ge, subset_of_time_series ) {
      deque_of_time_series . push_back ( ge );
    }
    boost::shared_ptr<GRIDCHOICE> subgrid ( phase_space -> subgrid ( deque_of_time_series ) );
    subgrid -> subdivide ();
    phase_space = subgrid;
  }
  std::cout << "\rInitial Phase Space Generated.\n";
  return phase_space;
}

/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] )
{
  
  clock_t start, stop;
  start = clock ();
  
  chomp::Rect x0 ( 2 );
  x0 . lower_bounds [ 0 ] = .2;
  x0 . upper_bounds [ 0 ] = .2;
  x0 . lower_bounds [ 1 ] = .85;
  x0 . upper_bounds [ 1 ] = .85;
  
  int N =  1000000;
  int tail = 100000;
  double timeseriestimestep = .01;
  ModelMap timeseriesmap = generateMap ( timeseriestimestep );
  std::vector < chomp::Rect > time_series = generateTimeSeries ( x0, timeseriesmap, N, tail );
  boost::shared_ptr<GRIDCHOICE> phase_space = generateInitialPhaseSpace ( time_series );
 
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  double time_of_flight = .02;
  ModelMap map = generateMap ( time_of_flight );

  MorseGraph conley_morse_graph;
  
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph,
                       phase_space,
                       map,
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
