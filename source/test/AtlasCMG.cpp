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

#include <boost/shared_ptr.hpp>

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
#include "database/structures/RectGeo.h"

#include "database/tools/SingleOutput.h"
#include "database/numerics/simple_interval.h"

//#include "database/program/Configuration.h"

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

///

#include <iostream>
#include <stdlib.h>

#include "database/structures/Atlas.h"
#include "database/maps/AtlasMap.h"


int main ( void ) { 

	typedef uint64_t GridElement;

	// Initialize Atlas
	boost::shared_ptr<Atlas> atlas ( new Atlas );
	atlas -> importCharts ( "atlas.xml" );
	atlas -> list_charts ( );

	// Subdivide Atlas
	//for ( unsigned int i=0; i<10; ++i )	atlas -> subdivide ( );
	atlas -> list_charts ( );	

	// Initialize Parameter Box
	std::vector < double > param_lb, param_ub;
	param_lb . push_back ( 3.4 );
	param_ub . push_back ( 3.4 );
	RectGeo param ( 1, param_lb, param_ub );

	// Load Maps
	AtlasMap atlasmap;
	//atlasmap . importMaps( param, "maps.txt" );
	//atlasmap . list_maps ( );

	MorseGraph MG;

  Compute_Morse_Graph (   &MG,
                          atlas,
                          atlasmap,
                          10, 
                          12, 
                          100);

/*
	// Track an orbit (approximately)
	GridElement GE = rand() % atlas . size ( ) ;

	for ( unsigned int i=0; i<1000; ++i ) {
		std::vector < GridElement > imagesGE = 
			atlas . cover ( atlasmap ( atlas . geometry ( GE ) ) );	

		std::cout << "GE " << GE << " mapped to: \n";
		for ( int j = 0; j < imagesGE . size (); ++ j ) {
			std::cout << imagesGE [ j ] << " ";
		}
		std::cout << "\n";

		GE  = imagesGE [ rand() % imagesGE . size() ];

		std::cout << "Map : " << i << ", GE = " << GE << "\n";
		boost::shared_ptr<Geo> geo = atlas . geometry ( GE );
		boost::dynamic_pointer_cast<AtlasGeo> ( geo ) -> info();
	}
	*/
  return 0;
}
