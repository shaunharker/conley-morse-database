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

#define WIDTH 1.0/128.0
#define STEPS 100
#define SAMPLES 4

#define CMG_VERBOSE
#define NO_REACHABILITY
#define COMPUTE_MORSE_GRAPH
#define COMPUTE_CONLEY_MORSE_GRAPH
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


using namespace chomp;
int INITIALSUBDIVISIONS = 8;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 10 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 12 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;


#include "database/maps/Selkov.h"
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

/**************************************************/
/* Map, Parameter Space and Phase space Settings  */
/**************************************************/
ModelMap initializeMap ( double timestep, int steps, int samples ) {
  Rect parameter_box = parameterBounds ();
  ModelMap map ( parameter_box, timestep, steps, samples );
  return map;
}

Rect phaseBounds ( void ) {
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 1.4;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 1.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect parameterBounds ( void ) {
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
  Compute_Morse_Graph ( & morsegraph,
                       phase_space,
                       map,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
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
  /* INITIALIZE MAP ************************************************/
  double time_of_flight = .1;                                      //
  ModelMap map = initializeMap ( time_of_flight, STEPS, SAMPLES ); //
  /*****************************************************************/

#ifdef COMPUTE_MORSE_GRAPH
  /* INITIALIZE MORSE GRAPH WITH PHASE SPACE *********************/
  MorseGraph morsegraph ( new GRIDCHOICE );                      //
  morsegraph . phaseSpace () -> initialize ( phaseBounds () );   //
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )               //
    morsegraph . phaseSpace () -> subdivide ();                  //
  /***************************************************************/

  /* COMPUTE MORSE GRAPH *************************************/
  TIC;                                                       //
  computeMorseGraph ( morsegraph, map, "selkov.mg" );        //
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
  ConleyMorseGraph conleymorsegraph ( "selkov.mg" );                  //
  computeConleyMorseGraph ( conleymorsegraph, map, "selkov.cmg" );    //
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
