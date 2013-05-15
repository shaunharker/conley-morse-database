/// Construct and display a conley morse graph for a given dynamical system

#define CONLEYINDEXCUTOFF 2

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

//#define WIDTH 1.0/8.0
#define TIMESTEP .05
#define STEPS 50
#define SAMPLES 4

// This is to set a common spatial scale when we vary parameters
// The number of subdivisions will be found from COMMONSCALE
// NOTE : This spatial scale is guaranteed along the longest phase space direction
// but in the smallest phase space direction, we will have much smaller scale
//#define COMMONSCALE 0.25

#define CMG_VERBOSE
#define NO_REACHABILITY
//#define COMPUTE_MORSE_GRAPH
#define COMPUTE_CONLEY_MORSE_GRAPH

#define DRAW_IMAGES

//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/Rect.h"
#include "database/tools/SingleOutput.h"

/*************************/
/* Subdivision Settings  */
/*************************/

#undef GRIDCHOICE
#include <boost/serialization/export.hpp>

#ifdef USE_SUCCINCT
#define GRIDCHOICE SuccinctGrid
#include "database/structures/SuccinctGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);

#else
#define GRIDCHOICE PointerGrid
#include "database/structures/PointerGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
#endif

int TIMESERIESLENGTH = 1000000;
int TAIL = 0;

using namespace chomp;
int INITIALSUBDIVISIONS = 16;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 60; //36 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 65; //37 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


double Ndivisions;
double WIDTH;
double COMMONSCALE;

#include "database/maps/WavePool.h"
typedef MapRect ModelMap;

/*************************/
/* FORWARD DECLARATIONS  */
/*************************/
Rect phaseBounds ( void );
Rect parameterBounds ( void );
ModelMap initializeMap (double timestep,
                        int steps,
                        int samples );
void computeMorseGraph (MorseGraph & morsegraph,
                        ModelMap & map,
                        const char * outputfile = NULL );
void computeConleyMorseGraph (MorseGraph & morsegraph,
                              ModelMap & map,
                              const char * outputfile = NULL,
                              const char * inputfile = NULL );


boost::shared_ptr<GRIDCHOICE> generateInitialPhaseSpace ( const std::vector < chomp::Rect > & time_series, double delta_box );


std::vector < chomp::Rect > generateTimeSeries ( const chomp::Rect & x0,
                                                const ModelMap & map, int N,
                                                int tail );

std::vector < chomp::Rect > generateTimeSeries ( const chomp::Rect & x0,
                                                const ModelMap & map, double dt, double lscale, int N,
                                                int tail );



Rect BoundingBox ( std::vector < Rect > listrect,  boost::shared_ptr < Grid > phase_space );


/**************************************************/
/* Map, Parameter Space and Phase space Settings  */
/**************************************************/
ModelMap initializeMap ( Rect parameter_box, double timestep, int steps, int samples ) {
  //Rect parameter_box = parameterBounds ();
  ModelMap map ( parameter_box, timestep, steps, samples );
  return map;
}

ModelMap initializeMap ( Rect parameter_box, double timestep, int steps, double lscale, int samples ) {
  //Rect parameter_box = parameterBounds ();
  ModelMap map ( parameter_box, timestep, steps, lscale, samples );
  return map;
}


Rect phaseBounds ( void ) {
  int phase_space_dimension = 4;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 8.0;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 16.0;
  phase_space_bounds . lower_bounds [ 2 ] = 0.0;
  phase_space_bounds . upper_bounds [ 2 ] = 10.0;
  phase_space_bounds . lower_bounds [ 3 ] = 0.0;
  phase_space_bounds . upper_bounds [ 3 ] = 16.0;
  
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect parameterBounds ( void ) {
  Rect parambox ( 11 );
  // alpha1
  parambox . lower_bounds[0] = 20.0;
  parambox . upper_bounds[0] = 20.0;
  // K1
  parambox . lower_bounds[1] = 4.0;
  parambox . upper_bounds[1] = 4.0;
  // alpha3
  parambox . lower_bounds[2] = 20.0;
  parambox . upper_bounds[2] = 20.0;
  // K3
  parambox . lower_bounds[3] = 4.0;
  parambox . upper_bounds[3] = 4.0;
  // alpha4
  parambox . lower_bounds[4] = 20.0;
  parambox . upper_bounds[4] = 20.0;
  // K4
  parambox . lower_bounds[5] = 4.0;
  parambox . upper_bounds[5] = 4.0;
  // alpha21
  parambox . lower_bounds[6] = 20.0;
  parambox . upper_bounds[6] = 20.0;
  // K21
  parambox . lower_bounds[7] = 5.0;
  parambox . upper_bounds[7] = 5.0;
  // alpha22
  parambox . lower_bounds[8] = 20.0;
  parambox . upper_bounds[8] = 20.0;
  // K22
  parambox . lower_bounds[9] = 10.0;
  parambox . upper_bounds[9] = 10.0;
  // n
  parambox . lower_bounds[10] = 4.0;
  parambox . upper_bounds[10] = 4.0;
  
  std::cout << "Parameter Choice = " << parambox << "\n";
  return parambox;
}



/*******************************/
/* Computation of Morse Graph  */
/*******************************/
void computeMorseGraph ( MorseGraph & morsegraph,
                        ModelMap & map,
                        const char * outputfile ) {
  /* COMPUTE CONLEY MORSE GRAPH */
#ifdef CMG_VERBOSE
  std::cout << "Computing Morse Graph\n";
#endif
  boost::shared_ptr < Grid > phase_space = morsegraph . phaseSpace ();
std::cout << "Ndivisions " << Ndivisions << "\n";
  Compute_Morse_Graph ( & morsegraph,
                       phase_space,
                       map,
//                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
//                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
//Ndivisions + 1,
//Ndivisions + 3,
5,
6,
                       SINGLECMG_COMPLEXITY_LIMIT );
  if ( outputfile != NULL ) {
    morsegraph . save ( outputfile );
  }
}

/**************************************/
/* Computation of Conley Morse Graph  */
/**************************************/
void computeConleyMorseGraph ( MorseGraph & morsegraph,
                              ModelMap & map,
                              const char * outputfile,
                              const char * inputfile ) {
  if ( inputfile != NULL ) {
    morsegraph . load ( inputfile );
  }
  boost::shared_ptr < Grid > phase_space = morsegraph . phaseSpace ();
#ifdef CMG_VERBOSE
  std::cout << "Number of Morse Sets = " << morsegraph . NumVertices () << "\n";
#endif
  typedef std::vector < Grid::GridElement > Subset;
  for ( size_t v = 0; v < morsegraph . NumVertices (); ++ v) {
    Subset subset = phase_space -> subset ( * morsegraph . grid ( v ) );
#ifdef CMG_VERBOSE
    std::cout << "Calling Conley_Index on Morse Set " << v << "\n";
#endif
    boost::shared_ptr<ConleyIndex_t> conley ( new ConleyIndex_t );
    morsegraph . conleyIndex ( v ) = conley;
    ConleyIndex ( conley . get (),
                 *phase_space,
                 subset,
                 map );
    
  }
  if ( outputfile != NULL ) {
    morsegraph . save ( outputfile );
  }
}

/*****************/
/* Timer Macros  */
/*****************/
clock_t global_timer;
#define TIC global_timer=clock()
#define TOC std::cout << (float)(clock()-global_timer)/(float)CLOCKS_PER_SEC << " seconds elapsed.\n";

/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] )
{
  
  double newalpha21, newalpha22;
  if ( argc == 3 ) {
    // TO DO : LOGARITHMIC SCALING
    newalpha21 = atof ( argv [ 1 ] );
    newalpha22 = atof ( argv [ 2 ] );
  } else {
    std::cout << "Need two arguments\n";
    return 0;
  }
  
  Rect parambox = parameterBounds ( );
  // change alpha21 and alpha22 from the arguments
  parambox . lower_bounds [ 6 ] = newalpha21;
  parambox . upper_bounds [ 6 ] = newalpha21;
  parambox . lower_bounds [ 8 ] = newalpha22;
  parambox . upper_bounds [ 8 ] = newalpha22;


  COMMONSCALE = 0.5;
  WIDTH = COMMONSCALE;
  
  // Compute the trajectory from the same initial condition
  // First initialize the map with a small dt for better accuracy
  // Make sure we get to the attractor if there is one
  // Initialize the map and the parameters
  
  /* INITIALIZE MAP ************************************************/
  ModelMap map = initializeMap ( parambox, TIMESTEP, STEPS, SAMPLES );
  ModelMap tsmap = initializeMap ( parambox, TIMESTEP, WIDTH/4.0, 1,  SAMPLES );
  /*****************************************************************/
  
  // Generate the time series, i.e. the trajectory
  // using the same I.C.
  chomp::Rect x0 ( 4 );
  x0 . lower_bounds [ 0 ] = 10.0;
  x0 . upper_bounds [ 0 ] = 10.0;
  x0 . lower_bounds [ 1 ] = 1.0;
  x0 . upper_bounds [ 1 ] = 1.0;
  x0 . lower_bounds [ 2 ] = 1.0;
  x0 . upper_bounds [ 2 ] = 1.0;
  x0 . lower_bounds [ 3 ] = 0.1;
  x0 . upper_bounds [ 3 ] = 0.1;
  // Two consecutive iterates are less than WIDTH/4.0 away from each other
  std::vector < chomp::Rect > time_series =
  generateTimeSeries ( x0, tsmap, TIMESTEP, WIDTH/4.0, TIMESERIESLENGTH, TAIL );
  // Define the phase space based on the trajectory
  // we have to encapsulate the IC and the possible attractor
  // WIDTH is the size of the boxes covering the trajectory
  boost::shared_ptr<GRIDCHOICE> phase_space = generateInitialPhaseSpace ( time_series, WIDTH );
 
#ifdef COMPUTE_MORSE_GRAPH
  /* INITIALIZE MORSE GRAPH WITH PHASE SPACE *********************/
  MorseGraph morsegraph ( phase_space );                         //
  //for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )               //
  //  morsegraph . phaseSpace () -> subdivide ();                  //
  /***************************************************************/
  
  /* COMPUTE MORSE GRAPH *************************************/
  TIC;                                                       //
  computeMorseGraph ( morsegraph, map, "WavePool.mg" );      //
  std::cout << "Total Time for Finding Morse Sets ";         //
#ifndef NO_REACHABILITY                                      //
  std::cout << "and reachability relation: ";                //
#elseif                                                      //
  std::cout << ": ";                                         //
#endif                                                       //
  TOC;                                                       //
  /***********************************************************/
#endif
  
#ifdef COMPUTE_CONLEY_MORSE_GRAPH
  /* COMPUTE CONLEY MORSE GRAPH ***************************************/
  TIC;                                                                //
  ConleyMorseGraph conleymorsegraph ( "WavePool.mg" );                  //
  computeConleyMorseGraph ( conleymorsegraph, map, "WavePool.cmg" );    //
  TOC;                                                                //
  /********************************************************************/
#else
  ConleyMorseGraph & conleymorsegraph = morsegraph;
#endif
  
#ifdef DRAW_IMAGES
  /* DRAW IMAGES ***********************************************************/
  TIC;                                                                     //
  std::cout << "Creating image file...\n";                                 //
  DrawMorseSets ( * (conleymorsegraph . phaseSpace ()), conleymorsegraph );//
  std::cout << "Creating graphviz .dot file...\n";                         //
  CreateDotFile ( conleymorsegraph );                                      //
  TOC;                                                                     //
  /*************************************************************************/
#endif
  return 0;
} /* main */






Rect BoundingBox ( const std::vector < Rect > & listrect ) {
  
  size_t dim;
  dim = listrect[0].dimension();
  Rect output ( dim );
  double xmin [ dim ], xmax [ dim ];
  for ( size_t i=0; i<dim; ++i ) { xmin [ i ] = 10000.0; xmax [ i ] = -10000.0; }
  //
  if ( listrect . size ( ) != 0 ) {
    for ( size_t i=0; i<listrect.size(); ++i ) {
      for ( size_t j=0; j<dim; ++j ) {
        if ( listrect [ i ] . lower_bounds [ j ] < xmin [ j ] )
          xmin [ j ] = listrect [ i ] . lower_bounds [ j ];
        if ( listrect [ i ] . upper_bounds [ j ] > xmax [ j ] )
          xmax [ j ] = listrect [ i ] . upper_bounds [ j ];
      }
    }
  }
  for ( size_t d = 0; d < dim; ++ d ) {
    output . lower_bounds [ d ] = xmin [ d ];
    output . upper_bounds [ d ] = xmax [ d ] ;
  }
  return output;
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
  int dim;
  dim = x0 . dimension ( );
  int percent= 0;
  for ( int i = 0; i < N; ++ i ) {
    //std::cout << x << "\n";
    int newpercent = (i*100)/N;
    if ( newpercent > percent ) {
      std::cout << "\r            \r" << newpercent << "%";
      std::cout . flush ();
      percent = newpercent;
    }
    if ( i >= tail ) result . push_back ( x );
    x = map ( x );
    for ( int j=0; j<dim; ++j ) {
      x . lower_bounds [ j ] = x . upper_bounds [ j ] = (x . lower_bounds [ j ] + x . upper_bounds [ j ])/2.0;
    }
  }
  std::cout << "\rTime Series Generated.\n";
  
  //for ( int i=0; i<result.size(); ++i ) std::cout << result[i].lower_bounds[0]<<"\n";
  
  return result;
}


std::vector < chomp::Rect > generateTimeSeries ( const chomp::Rect & x0,
                                                const ModelMap & map, double dt, double lscale, int N,
                                                int tail = 0 ) {
  
  std::cout << "Generating time series.\n";
  
  std::vector < chomp::Rect > result;
  result . reserve ( N - tail );
  chomp::Rect x = x0;
  int dim;
  dim = x0 . dimension ( );
  int percent= 0;
  for ( int i = 0; i < N; ++ i ) {
    //std::cout << x << "\n";
    int newpercent = (i*100)/N;
    if ( newpercent > percent ) {
      std::cout << "\r            \r" << newpercent << "%";
      std::cout . flush ();
      percent = newpercent;
    }
    if ( i >= tail ) result . push_back ( x );
    x = map ( x, dt, lscale );
    for ( int j=0; j<dim; ++j ) {
      x . lower_bounds [ j ] = x . upper_bounds [ j ] = (x . lower_bounds [ j ] + x . upper_bounds [ j ])/2.0;
    }
  }
  std::cout << "\rTime Series Generated.\n";
  
  //for ( int i=0; i<result.size(); ++i ) std::cout << result[i].lower_bounds[0]<<"\n";
  
  return result;
}


/********************************/
/* Generate Initial Phase Space */
/********************************/
boost::shared_ptr<GRIDCHOICE> generateInitialPhaseSpace ( const std::vector < chomp::Rect > & time_series, double delta_box ) {
  std::cout << "Generating Initial Phase Space.\n";
  
  //chomp::Rect phase_space_bounds = phaseBounds ();
  //
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  //phase_space -> initialize ( phase_space_bounds );
  
  // HERE WE RESET THE PHASE SPACE BOUNDS BASED ON THE TRAJECTORY
  std::cout << "Time series size: " <<  time_series . size () << "\n";
  chomp::Rect outerbox = BoundingBox ( time_series );
  
  std::cout << "Bounding box = " << outerbox << "\n";
  
  //
  int dim;
  dim = 4;//phase_space_bounds . dimension ( );
  for ( int i=0; i<dim; ++i ) {
    double width = outerbox . upper_bounds [ i ]
    - outerbox . lower_bounds [ i ] + 1.0 ;  // add 1.0 to avoid crashing if we remove trajectory tail
    outerbox . upper_bounds [ i ] += width/2.0;
    outerbox . lower_bounds [ i ] -= width/2.0;
  }
  //
  std::cout << "Thickened bounding box = " << outerbox << "\n";
  
  phase_space -> initialize ( outerbox );
  std::cout << "New phase space: " << outerbox << "\n";
 

  // to guarantee consistency of the subdivision we subdivide till a certain accuracy level is met
  // the accuracy is set by COMMONSCALE
  //
  double mymax ( outerbox . upper_bounds[0] - outerbox . lower_bounds[0] );
  for ( int i=0; i<dim; ++i ) {
    if ( outerbox . upper_bounds[i] - outerbox . lower_bounds[i] > mymax ) 
      mymax = outerbox . upper_bounds[i] - outerbox . lower_bounds[i]; 
  }
 
  //int Ndivisions;
  Ndivisions = (int) ( dim * log2 ( mymax / COMMONSCALE ) ) + 1;
  std::cout << "we need : " << Ndivisions << " subdivisions.\n";
//  for ( int subdivision_depth = 0; subdivision_depth < INITIALSUBDIVISIONS; ++ subdivision_depth ) {
  for ( int subdivision_depth = 0; subdivision_depth < Ndivisions; ++ subdivision_depth ) {

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
      collared_rect . lower_bounds [ 0 ] -= delta_box;
      collared_rect . upper_bounds [ 0 ] += delta_box;
      collared_rect . lower_bounds [ 1 ] -= delta_box;
      collared_rect . upper_bounds [ 1 ] += delta_box;
      
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

