
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
#include <sstream>
#include <algorithm>

/***************************/
/* Preprocessor directives */
/***************************/

#define CMG_VERBOSE
#define MEMORYBOOKKEEPING
#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/Rect.h"

#include "database/tools/SingleOutput.h"
#include "database/numerics/simple_interval.h"

#include "database/program/Configuration.h"

#include "ModelMap.h"

/*************************/
/* Subdivision Settings  */
/*************************/

//#undef GRIDCHOICE
#undef PHASE_GRID
#include <boost/serialization/export.hpp>

#ifdef USE_SUCCINCT
#define PHASE_GRID SuccinctGrid
#include "database/structures/SuccinctGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#else
#define PHASE_GRID PointerGrid
#include "database/structures/PointerGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
#endif

using namespace chomp;

int INITIALSUBDIVISIONS;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT;

Rect parameterBounds ( const Rect parameter_box, const std::vector<Real> b, const int param_subdiv ) {
  
  int parameter_space_dimension = parameter_box . dimension();
 
  Rect parameter_space_limits ( parameter_box );

  double PARAMETER_BOXES = pow(2,param_subdiv);
  Rect parameter_box_new ( parameter_space_dimension );

  std::cout << PARAMETER_BOXES;
  
  Real max = b [ 0 ];
  for (int i=0; i<b.size(); ++i ) {
    if ( b [ i ] > max ) max = b [ i ];
  }
  if ( max > PARAMETER_BOXES ) {
    std::cout << "The choice of the subinterval for the parameters is incorrect.";
    std::cout << "Check the arguments of SingleCMG\n";
    exit(-1);
  }

  for ( int i=0; i<parameter_space_dimension; ++i ) {
    parameter_box_new . lower_bounds [ i ] = parameter_space_limits . lower_bounds [ i ] + 
      ( parameter_space_limits . upper_bounds [ i ] - parameter_space_limits . lower_bounds [ i ] ) * b[i] / PARAMETER_BOXES;
    parameter_box_new . upper_bounds [ i ] = parameter_space_limits . lower_bounds [ i ] + 
      ( parameter_space_limits . upper_bounds [ i ] - parameter_space_limits . lower_bounds [ i ] ) * ( b[i] + 1.0 ) / PARAMETER_BOXES;
  }

  return parameter_box_new;
}

/*************************/
/* FORWARD DECLARATIONS  */
/*************************/
void computeMorseGraph (MorseGraph & morsegraph,
                        ModelMap & map,
                        const int SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_COMPLEXITY_LIMIT,
                        const char * outputfile = NULL );
void computeConleyMorseGraph (MorseGraph & morsegraph,
                              ModelMap & map,
                              const char * outputfile = NULL,
                              const char * inputfile = NULL );


/*******************************/
/* Computation of Morse Graph  */
/*******************************/
void computeMorseGraph ( MorseGraph & morsegraph,
                        ModelMap & map,
                        const int SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_COMPLEXITY_LIMIT,
                        const char * outputfile ) {
#ifdef CMG_VERBOSE
  std::cout << "computeMorseGraph.\n";
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
  std::cout << "computeConleyMorseGraph.\n";
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

  // Load the configuration file 
  Configuration config;
  config . loadFromFile ( "./" );

  Rect parameter_box = config . PARAM_BOUNDS;
  int param_subdiv_depth = config . PARAM_SUBDIV_DEPTH;

  /* INITIALIZE MAP ************************************************/
  std::vector < Real > b;
  // Check the number of integers provided is correct
  if ( argc - 1 == parameter_box . dimension() ) {
    for ( int i=1; i<argc; ++i ) b . push_back ( (Real) atoi ( argv[i] ) );
  } else {
    //std::cout << "You did not provide the proper number of integers\n";
    std::cout << "Expecting " << config . PARAM_DIM << " integers ";
    std::cout << "in the interval [0, " << (int)(pow(2,param_subdiv_depth)-1) << "]\n";
    std::cout << "Program terminated, no computation performed\n";
    exit(-1);
  }

  Rect parameter_box_new = parameterBounds ( parameter_box, b, param_subdiv_depth );
  ModelMap map ( parameter_box_new );
  /*****************************************************************/
  
  /* INITIALIZE THE PHASE SPACE SUBDIVISION PARAMETERS FROM CONFIG FILE */
  INITIALSUBDIVISIONS = config . PHASE_SUBDIV_INIT;
  SINGLECMG_MIN_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MIN;
  SINGLECMG_MAX_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MAX;
  SINGLECMG_COMPLEXITY_LIMIT= config . PHASE_SUBDIV_LIMIT;

#ifdef COMPUTE_MORSE_SETS

  MorseGraph morsegraph ( new PHASE_GRID ); 

  /* INITIALIZE MORSE GRAPH WITH PHASE SPACE *********************/                       //
  morsegraph . phaseSpace () -> initialize ( config . PHASE_BOUNDS );   //
  morsegraph . phaseSpace () -> periodicity () = config . PHASE_PERIODIC;   //
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )               //
    morsegraph . phaseSpace () -> subdivide ();                  //
  /***************************************************************/

  /* COMPUTE MORSE GRAPH *************************************/
  TIC;                                                       //
  computeMorseGraph ( morsegraph, map,
                      SINGLECMG_MIN_PHASE_SUBDIVISIONS, 
                      SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                      SINGLECMG_COMPLEXITY_LIMIT, "data.mg" );        //
  std::cout << "Total Time for Finding Morse Sets ";         //
#ifndef NO_REACHABILITY                                      //
  std::cout << "and reachability relation: ";                //
#elseif                                                      //
  std::cout << ": ";                                         //
#endif                                                       //
  TOC;                                                       //
  /***********************************************************/
#endif // endif for COMPUTE_MORSE_SETS



#ifdef COMPUTE_CONLEY_INDEX

  /* COMPUTE CONLEY MORSE GRAPH ***************************************/
  TIC;                                                             //
  ConleyMorseGraph conleymorsegraph ( "data.mg" );
  computeConleyMorseGraph ( conleymorsegraph, map, "data.cmg" );    //
  TOC;                                                                //
  /********************************************************************/
#else
  ConleyMorseGraph & conleymorsegraph = morsegraph;
#endif

// Always output the Morse Graph
  std::cout << "Creating graphviz .dot file...\n";                         //
  CreateDotFile ( conleymorsegraph );  
  
#ifdef DRAW_IMAGES
  /* DRAW IMAGES ***********************************************************/
  TIC;                                                                     //
  std::cout << "Creating image file...\n";                                 //
  DrawMorseSets ( * (conleymorsegraph . phaseSpace ()), conleymorsegraph );//
  TOC;                                                                     //
  /*************************************************************************/
#endif

  return 0;
} /* main */

