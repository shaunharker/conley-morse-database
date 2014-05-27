
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

#include "Model.h"

#include "database/maps/Map.h"
#include "database/maps/ChompMap.h"
#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/RectGeo.h"

#include "database/tools/SingleOutput.h"
#include "database/numerics/simple_interval.h"

#include "database/program/Configuration.h"



/*************************/
/* Subdivision Settings  */
/*************************/

#include <boost/serialization/export.hpp>
#ifdef USE_SDSL
#include "database/structures/SuccinctGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
#include "database/structures/PointerGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);


/*************************/
/* FORWARD DECLARATIONS  */
/*************************/
void computeMorseGraph (MorseGraph & morsegraph,
                        boost::shared_ptr<const Map> map,
                        const int SINGLECMG_INIT_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_COMPLEXITY_LIMIT,
                        const char * outputfile = NULL );
void computeConleyMorseGraph (MorseGraph & morsegraph,
                              boost::shared_ptr<const Map> map,
                              const char * outputfile = NULL,
                              const char * inputfile = NULL );


/*******************************/
/* Computation of Morse Graph  */
/*******************************/
void computeMorseGraph ( MorseGraph & morsegraph,
                        boost::shared_ptr<const Map> map,
                        const int SINGLECMG_INIT_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                        const int SINGLECMG_COMPLEXITY_LIMIT,
                        const char * outputfile ) {
#ifdef CMG_VERBOSE
  std::cout << "SingleCMG: computeMorseGraph.\n";
#endif
  boost::shared_ptr < Grid > phase_space = morsegraph . phaseSpace ();
  clock_t start_time = clock ();
  Compute_Morse_Graph ( & morsegraph,
                       phase_space,
                       map,
                       SINGLECMG_INIT_PHASE_SUBDIVISIONS,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  clock_t stop_time = clock ();
  if ( outputfile != NULL ) {
    morsegraph . save ( outputfile );
  }
  std::ofstream stats_file ( "SingleCMG_statistics.txt" );
  stats_file << "Morse Graph calculation resource usage statistics.\n";
  stats_file << "The final grid has " << phase_space -> size () << " grid elements.\n";
  stats_file << "The computation took " << ((double)(stop_time-start_time)/(double)CLOCKS_PER_SEC)
             << " seconds.\n";
  stats_file << "All memory figures are in bytes:\n";
  stats_file << "grid_memory_use = " << phase_space -> memory () << "\n";
  stats_file << "max_graph_memory = " << max_graph_memory << "\n";
  stats_file << "max_scc_memory_internal = " << max_scc_memory_internal << "\n";
  stats_file << "max_scc_memory_external = " << max_scc_memory_external << "\n";
  stats_file . close ();
}

/**************************************/
/* Computation of Conley Morse Graph  */
/**************************************/
void computeConleyMorseGraph ( MorseGraph & morsegraph,
                              boost::shared_ptr<const Map> map,
                              const char * outputfile,
                              const char * inputfile ) {
  std::cout << "SingleCMG: computeConleyMorseGraph.\n";
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
    ChompMap chomp_map ( map );
    chomp::ConleyIndex ( conley . get (),
                        *phase_space,
                        subset,
                        chomp_map );
    
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
int main ( int argc, char * argv [] ) {
  /* INITIALIZE THE MODEL */
  Model model;
  model . initialize ( argc, argv );
  boost::shared_ptr<const Map> map = model . map ();

#ifdef COMPUTE_MORSE_SETS

  MorseGraph morsegraph ( model . phaseSpace () ); 

  /* INITIALIZE THE PHASE SPACE SUBDIVISION PARAMETERS FROM CONFIG FILE */
  Configuration config;
  config . loadFromFile ( "./" );
  int SINGLECMG_INIT_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_INIT;
  int SINGLECMG_MIN_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MIN;
  int SINGLECMG_MAX_PHASE_SUBDIVISIONS = config . PHASE_SUBDIV_MAX;
  int SINGLECMG_COMPLEXITY_LIMIT= config . PHASE_SUBDIV_LIMIT;

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
