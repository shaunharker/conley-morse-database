
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
#define DRAW_IMAGES
//#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

//#include "Model.h"
#include "ModelMap.h"

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/RectGeo.h"

#include "database/tools/SingleOutput.h"
#include "database/numerics/simple_interval.h"

#include "database/program/Configuration.h"



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

/// Choose a parameter region from within a parameter box
/// given coordinates "b" and a subdivision level (dimension-wise)
/// "param_subdiv". For example, param_subdiv[2] = 3 means that
/// there will be 8 boxes in dimension 2.
RectGeo parameterBounds ( const RectGeo & parameter_box, 
                          const std::vector<Real> b, 
                          const std::vector<int> & param_subdiv ) {
  typedef RectGeo Rect;
  int parameter_space_dimension = parameter_box . dimension();
  Rect parameter_space_limits ( parameter_box );
  std::vector<double> PARAMETER_BOXES_ACROSS ( parameter_space_dimension );
  for ( int dim = 0; dim < parameter_space_dimension; ++ dim ) {
    PARAMETER_BOXES_ACROSS[dim] = pow(2,param_subdiv[dim]);
  }
  Rect parameter_box_new ( parameter_space_dimension );
  
  for ( int i=0; i<parameter_space_dimension; ++i ) {
    parameter_box_new . lower_bounds [ i ] = parameter_space_limits . lower_bounds [ i ] + 
      ( parameter_space_limits . upper_bounds [ i ] - parameter_space_limits . lower_bounds [ i ] ) * b[i] / PARAMETER_BOXES_ACROSS[i];
    parameter_box_new . upper_bounds [ i ] = parameter_space_limits . lower_bounds [ i ] + 
      ( parameter_space_limits . upper_bounds [ i ] - parameter_space_limits . lower_bounds [ i ] ) * ( b[i] + 1.0 ) / PARAMETER_BOXES_ACROSS[i];
  }

  return parameter_box_new;
}

/*************************/
/* FORWARD DECLARATIONS  */
/*************************/
void computeMorseGraph (MorseGraph & morsegraph,
                        ModelMap & map,
                        const int SINGLECMG_INIT_PHASE_SUBDIVISIONS,
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
                        const int SINGLECMG_INIT_PHASE_SUBDIVISIONS,
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
                       SINGLECMG_INIT_PHASE_SUBDIVISIONS,
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
  boost::shared_ptr < TreeGrid > phase_space = 
    boost::dynamic_pointer_cast<TreeGrid> ( morsegraph . phaseSpace () );
  if ( not phase_space ) {
    std::cout << "Cannot interface with chomp for this grid type.\n";
    std::cout << " (not producing .cmg file.)\n";
    return;
  }
#ifdef CMG_VERBOSE
  std::cout << "Number of Morse Sets = " << morsegraph . NumVertices () << "\n";
#endif
  typedef std::vector < Grid::GridElement > Subset;
  for ( size_t v = 0; v < morsegraph . NumVertices (); ++ v) {
    Subset subset = phase_space -> subset ( * morsegraph . grid ( v ) );
#ifdef CMG_VERBOSE
    std::cout << "Calling Conley_Index on Morse Set " << v << "\n";
#endif
    boost::shared_ptr<chomp::ConleyIndex_t> conley ( new chomp::ConleyIndex_t );
    morsegraph . conleyIndex ( v ) = conley;
    chomp::ConleyIndex ( conley . get (),
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

  //Model model;
  //model . initialize ( argc, argv );

  // Load the configuration file 
  Configuration config;
  config . loadFromFile ( "./" );

  RectGeo parameter_box = config . PARAM_BOUNDS;
  std::vector<int> param_subdiv_depth = config . PARAM_SUBDIV_DEPTH;

  /* INITIALIZE MAP ************************************************/
  std::vector < Real > b;
  RectGeo parameter_box_new;
  // Check the number of integers provided is correct
  if ( argc - 1 == (int) parameter_box . dimension() ) {
    for ( int i=1; i<argc; ++i ) b . push_back ( (Real) atoi ( argv[i] ) );
    parameter_box_new = parameterBounds ( parameter_box, b, param_subdiv_depth );
  } else {
    //std::cout << "You did not provide the proper number of integers\n";
    std::cout << "Expecting " << config . PARAM_DIM << " integers ";
    std::cout << "in the interval "; 
    for ( int dim = 0; dim < parameter_box . dimension(); ++ dim ) {
      if ( dim > 0 ) std::cout << " x ";
      std::cout << "[0, " << (int)(pow(2,param_subdiv_depth[dim])-1) << "]\n";
    }
    std::cout << "Computing using entire parameter region as parameter.\n";
    parameter_box_new = parameter_box;
  }

  ModelMap map ( parameter_box_new );
  /*****************************************************************/
  
  /* INITIALIZE THE PHASE SPACE SUBDIVISION PARAMETERS FROM CONFIG FILE */
  int SINGLECMG_INIT_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_INIT;
  int SINGLECMG_MIN_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MIN;
  int SINGLECMG_MAX_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MAX;
  int SINGLECMG_COMPLEXITY_LIMIT= config . PHASE_SUBDIV_LIMIT;

#ifdef COMPUTE_MORSE_SETS

  boost::shared_ptr < TreeGrid > phaseGrid ( new PHASE_GRID );

  MorseGraph morsegraph ( phaseGrid ); 

  /* INITIALIZE MORSE GRAPH WITH PHASE SPACE *********************/
  phaseGrid -> initialize ( config . PHASE_BOUNDS );
  phaseGrid -> periodicity () = config . PHASE_PERIODIC;      
  /***************************************************************/

  /* COMPUTE MORSE GRAPH *************************************/
  TIC;                                                       
  computeMorseGraph ( morsegraph, map,
                      SINGLECMG_INIT_PHASE_SUBDIVISIONS,
                      SINGLECMG_MIN_PHASE_SUBDIVISIONS, 
                      SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                      SINGLECMG_COMPLEXITY_LIMIT, "data.mg" );        
  std::cout << "Total Time for Finding Morse Sets ";         
#ifndef NO_REACHABILITY                                      
  std::cout << "and reachability relation: ";                
#else                                                      
  std::cout << ": ";                                         
#endif                                                       
  TOC;                                                       
  /***********************************************************/
#endif // endif for COMPUTE_MORSE_SETS



#ifdef COMPUTE_CONLEY_INDEX

  /* COMPUTE CONLEY MORSE GRAPH ***************************************/
  TIC;                                                             
  ConleyMorseGraph conleymorsegraph ( "data.mg" );
  computeConleyMorseGraph ( conleymorsegraph, map, "data.cmg" );    
  TOC;                                                                
  /********************************************************************/
#else
  ConleyMorseGraph & conleymorsegraph = morsegraph;
#endif


#ifdef DRAW_IMAGES
  /* DRAW IMAGES ***********************************************************/
  boost::shared_ptr<TreeGrid> cmg_treegrid =
     boost::dynamic_pointer_cast<TreeGrid> ( conleymorsegraph . phaseSpace () );

  if ( cmg_treegrid && cmg_treegrid -> dimension () == 2 ) {
    TIC;                                                                     
    std::cout << "Creating image file...\n";                                 
    DrawMorseSets ( * cmg_treegrid, conleymorsegraph );
    TOC;                                                                     
  }
  /*************************************************************************/
#endif

// Always output the Morse Graph
  std::cout << "Creating graphviz .dot file...\n";                         
  CreateDotFile ( "morsegraph.gv", conleymorsegraph );  
  
  return 0;
} /* main */

