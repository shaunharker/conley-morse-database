// zookeeper.cpp

//#define NOCONLEYINDEX
#include <boost/serialization/export.hpp>
#include "database/structures/Database.h"
#include "database/structures/Grid.h"
#include "database/structures/PointerGrid.h"
#include "database/structures/SuccinctGrid.h"
#include "database/structures/UniformGrid.h"
#include "database/structures/EdgeGrid.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/AbstractParameterSpace.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(UniformGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EdgeGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameter);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameterSpace);
BOOST_CLASS_EXPORT_IMPLEMENT(AbstractParameterSpace);


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/unordered_set.hpp"
#include "boost/unordered_map.hpp"
#include "../examples/BooleanSwitching/BooleanSwitchingParameterSpace.h"

#include "GI.h"


std::string conleyStringForZoo ( const Database & database, 
                                 const CI_Data & ci ) {
    // data
  	std::string ci_string;
  	// algo
  	std::stringstream ss;
  	ss << "(";
  	for ( int d = 0; d < ci . conley_index . size (); ++ d ) {
  		std::string s = ci . conley_index [ d ];
  		std::string t;
  		for ( int i = 0; i < s . size () - 1; ++ i ) {
  			//if ( s [ i ] == '.' ) continue;
  			//if ( s [ i ] == ' ' ) continue;
		  if ( s [ i ] == '\n' ) { t += std::string ( ", "); continue; }
  			t . push_back ( s [ i ] );
  		}
  		ss << t;
  		if ( d != ci . conley_index . size () - 1 ) ss << ", ";
  	}
  	ss << ")";
		ci_string = ss . str ();
		return ci_string;
}

std::string CI_Zoo ( const Database & database ) {
  std::stringstream result_ss;
	// Create data structure to get INCC from Conley Record Index
	// data
	boost::unordered_map < uint64_t, boost::unordered_set < uint64_t > > ci_to_incc;
	// algo
	const std::vector < uint64_t > & incc_conley = database . incc_conley ();
	for ( uint64_t incc = 0; incc < incc_conley . size (); ++ incc ) {
		ci_to_incc [ incc_conley [ incc ] ] . insert ( incc );
	}
	// Create list of pairs (frequency of ci, ci index) 
	//data
	std::vector < std::pair< long, std::string > > ci_zoo_data;
	//algo
	long count = 0;
  BOOST_FOREACH ( const CI_Data & ci, database . ciData () ) {
  	// Step 1. Obtain string from ci.
  	// data
  	std::string ci_string;
  	// algo
  	std::stringstream ss;
	  ss << "CI " << count << ": " << conleyStringForZoo ( database, ci ) << "\n";
		ci_string = ss . str ();
		// Step 2. Obtain Frequency of CI
		// data
		long ci_frequency = 0; 
		// algo
		BOOST_FOREACH ( uint64_t incc, ci_to_incc [ count ] ) {
			ci_frequency += database . incc_sizes () [ incc ];
		}
		// Step 3. Push result onto data list
		ci_zoo_data . push_back ( std::make_pair ( ci_frequency, ci_string ));
		++ count;
  }

  std::sort ( ci_zoo_data . rbegin (), ci_zoo_data . rend () ); // sort in descending order

  // DISPLAY CI ZOO DATA
  for ( uint64_t ci_zoo_index = 0; ci_zoo_index < ci_zoo_data . size (); ++ ci_zoo_index ) {
  	result_ss << "<p>" << ci_zoo_data [ ci_zoo_index ] . first << " " << ci_zoo_data [ ci_zoo_index ] . second << "</p>\n";
  }
  return result_ss . str ();
}

std::string dotFile ( const Database & database, uint64_t mgcc, uint64_t order_index, double frequency ) {
	std::stringstream ss;
	ss << "digraph MGCC" << order_index << " { \n";
	
	MGCC_Record const& mgcc_record = database . MGCC_Records () [ mgcc ];
  uint64_t mgccp_index = mgcc_record . mgccp_indices [ 0 ];
  const MGCCP_Record & mgccp_record = database . MGCCP_Records () [ mgccp_index ];
  uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  const MorseGraphRecord & morsegraph_record = 
  	database . morsegraphData () [ morsegraph_index ];
  uint64_t dag_index = morsegraph_record . dag_index;
  const DAG_Data & dag = database . dagData () [ dag_index ];

  // Vertices
  for ( int i = 0; i < dag . num_vertices; ++ i ) {
  	CS_Data cs_data;
  	cs_data . vertices . push_back ( i );
  	uint64_t cs_index = database . csIndex ( cs_data );
  	INCCP_Record inccp_record;
  	inccp_record . cs_index = cs_index;
  	inccp_record . mgccp_index = mgccp_index;
  	uint64_t inccp_index = database.inccpIndex ( inccp_record );
  	uint64_t incc_index = database.inccp_to_incc () [ inccp_index ];
#ifndef NOCONLEYINDEX
  	uint64_t conley = database . incc_conley () [ incc_index ];
  	const CI_Data & ci = database . ciData () [ conley ];
    ss << i << " [label=\""<< incc_index << "\" href=\"javascript:void(click_node_on_graph('" 
       <<  conleyStringForZoo ( database, ci ) << "'," << order_index << "))\"]\n";
#else 
    ss << i << " [label=\""<< incc_index << "\" href=\"javascript:void(click_node_on_graph"
               "('Unknown Conley Index'," << order_index << "))\"]\n";
#endif 
  }  
 	
 	ss << "LEGEND [label=\"MGCC " << mgcc << "\\n " << 100.0*frequency << "% \" shape=\"rectangle\"]\n";

 	// Edges 
  typedef std::pair<int, int> Edge;
  BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
  	ss << e . first << " -> " << e . second << "; ";
  }
	ss << "}\n";
	return ss. str ();
}

DAG makeDAG ( const Database & database, uint64_t mgcc ) {
	DAG result;
	MGCC_Record const& mgcc_record = database . MGCC_Records () [ mgcc ];
  uint64_t mgccp_index = mgcc_record . mgccp_indices [ 0 ];
  const MGCCP_Record & mgccp_record = database . MGCCP_Records () [ mgccp_index ];
  uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  const MorseGraphRecord & morsegraph_record = 
  	database . morsegraphData () [ morsegraph_index ];
  uint64_t dag_index = morsegraph_record . dag_index;
  const DAG_Data & dag = database . dagData () [ dag_index ];
  // Vertices
  result . num_vertices_ = dag . num_vertices;
  result . labels_ . resize ( dag . num_vertices );
  for ( int i = 0; i < dag . num_vertices; ++ i ) {
  	CS_Data cs_data;
  	cs_data . vertices . push_back ( i );
  	uint64_t cs_index = database . csIndex ( cs_data );
  	INCCP_Record inccp_record;
  	inccp_record . cs_index = cs_index;
  	inccp_record . mgccp_index = mgccp_index;
  	uint64_t inccp_index = database.inccpIndex ( inccp_record );
  	uint64_t incc_index = database.inccp_to_incc () [ inccp_index ];
#ifndef NOCONLEYINDEX
  	uint64_t conley = database . incc_conley () [ incc_index ];
  	const CI_Data & ci = database . ciData () [ conley ];
    result . labels_ [ i ] = conleyStringForZoo ( database, ci );
#else 
    result . labels_ [ i ] = std::string ( "Unknown Conley Index" );
#endif
  }  
 	// Edges 
  typedef std::pair<int, int> Edge;
  BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
  	DAG::Edge new_edge ( e . first, e . second );
  	result . edges_ . insert ( new_edge );
  }
	return result;
}

std::string CMGdotFile ( const DAG & dag, uint64_t order_index, double frequency ) {
	std::stringstream ss;
	ss << "digraph CMG" << order_index << " { \n";
  // Vertices
  for ( int i = 0; i < dag . num_vertices_; ++ i ) {
    ss << i << " [label=\""<< dag . labels_ [ i ] << "\"];\n";
  }  
 	ss << "LEGEND [label=\"CMG " << order_index << "\\n " << 100.0*frequency << "% \" shape=\"rectangle\"]\n";
 	// Edges 
  BOOST_FOREACH ( const DAG::Edge & e, dag . edges_ ) {
  	ss << e . first << " -> " << e . second << "; ";
  }
	ss << "}\n";
	return ss. str ();
}


void MGCC_Zoo ( const Database & database ) {
	// Sort mgcc by frequency
	// data
	std::vector < std::pair< long, uint64_t > > mgcc_sorted_by_frequency;
	uint64_t total_count = 0;
	// algo
	for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
		long frequency = 0;
		const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
		BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
			const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
			frequency += mgccp_record . parameter_indices . size ();
		}
		mgcc_sorted_by_frequency . push_back ( std::make_pair ( frequency, mgcc ) );
		total_count += frequency;
	}
	std::sort ( mgcc_sorted_by_frequency . rbegin (), mgcc_sorted_by_frequency . rend () ); // sort in descending order

	// DISPLAY MGCC ZOO DATA
	for ( uint64_t mgcc_zoo_index = 0; mgcc_zoo_index < mgcc_sorted_by_frequency . size (); ++ mgcc_zoo_index ) {
		long frequency = mgcc_sorted_by_frequency [ mgcc_zoo_index ] . first;
		uint64_t mgcc = mgcc_sorted_by_frequency [ mgcc_zoo_index ] . second;
		//if ( frequency < 10 ) break;
		std::cout << frequency << " " << mgcc << " " << dotFile ( database, mgcc, mgcc_zoo_index, (double) frequency / (double) total_count ) << "\n";
		// Produce filename
		// data
		std::string filename;
		// algo
		std::stringstream ss;
		ss << "MGCC" << mgcc_zoo_index << ".gv";
		filename = ss . str ();
		// Create File
		std::ofstream outfile ( filename . c_str () );
		outfile << dotFile ( database, mgcc, mgcc_zoo_index , (double) frequency / (double) total_count);
		outfile . close ();
	}
}

void CMG_Zoo ( const Database & database ) {
	// Sort mgcc by frequency
	// data
	boost::unordered_map<DAG, int64_t> cmgs_and_count;
	uint64_t total_count = 0;
	// algo
	for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
		long frequency = 0;
		const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
		BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
			const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
			frequency += mgccp_record . parameter_indices . size ();
		}
		DAG dag = makeDAG ( database, mgcc );
    if ( dag . num_vertices_ == 0 ) std::cout << "detected empty dag\n";
		cmgs_and_count [ dag ] += frequency;
		total_count += frequency;
	}
	std::vector < std::pair < int64_t, DAG > > data_to_sort;
	typedef std::pair < DAG, int64_t > value_t;
	BOOST_FOREACH ( value_t v, cmgs_and_count ) {
		data_to_sort . push_back ( std::make_pair ( v . second, v . first ) );
	}

  std::cout << "data_to_sort . size () ==  " << data_to_sort . size () << "\n";
	std::sort ( data_to_sort . rbegin (), data_to_sort . rend () ); // sort in descending order
  std::cout << "data_to_sort sorted.\n";
	// DISPLAY CMG ZOO DATA
	for ( uint64_t cmg_zoo_index = 0; cmg_zoo_index < data_to_sort . size (); ++ cmg_zoo_index ) {
		long frequency = data_to_sort [ cmg_zoo_index ] . first;
		DAG dag = data_to_sort [ cmg_zoo_index ] . second;
		// Produce filename
		// data
		std::string filename;
		// algo
		std::stringstream ss;
		ss << "CMG" << cmg_zoo_index << ".gv";
		filename = ss . str ();
		// Create File
		std::ofstream outfile ( filename . c_str () );
		outfile << CMGdotFile ( dag, cmg_zoo_index , (double) frequency / (double) total_count);
		outfile . close ();
	}
}

void Hasse_Zoo ( const Database & database ) {
	// Sort mgcc by frequency
	// data
	std::vector < std::pair < long, uint64_t > > dag_indices_by_frequency;
	uint64_t total_count = 0;

	// algo
	{
		dag_indices_by_frequency . resize ( database.dagData() . size () );
		for ( uint64_t dag = 0; dag < dag_indices_by_frequency . size (); ++ dag ) {
			dag_indices_by_frequency [ dag ] . first = 0;
			dag_indices_by_frequency [ dag ] . second = dag;
		}
		for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
			const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
			BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
				const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
				long frequency = mgccp_record . parameter_indices . size ();
				uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  			const MorseGraphRecord & morsegraph_record = 
  				database . morsegraphData () [ morsegraph_index ];
  			uint64_t dag_index = morsegraph_record . dag_index;
				dag_indices_by_frequency [ dag_index ] . first += frequency;
				total_count += frequency;
			}
		}
		std::sort ( dag_indices_by_frequency . rbegin (), dag_indices_by_frequency . rend () ); // sort in descending order
	}
	// DISPLAY HASSE ZOO DATA
	for ( uint64_t zoo_index = 0; zoo_index < dag_indices_by_frequency . size (); ++ zoo_index ) {
		long frequency = dag_indices_by_frequency [ zoo_index ] . first;
		uint64_t dag_index = dag_indices_by_frequency [ zoo_index ] . second;
		// Produce filename
		// data
		std::string filename;
		// algo
		{
			std::stringstream ss;
			ss << "HASSE" << zoo_index << ".gv";
			filename = ss . str ();
	  }
		// Create Body of File
		// data
		std::stringstream ss;
		// algo
		{

  		const DAG_Data & dag = database . dagData () [ dag_index ];

  		// debug
			if ( dag . num_vertices == 0 ) {
				std::cout << "EMPTY HASSE DIAGRAM. It seems a bug is present.)\n";
				std::cout << "database.dagData()[" << dag_index << "]\n";
				for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
					const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
					BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
						const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
						uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  					const MorseGraphRecord & morsegraph_record = 
  					database . morsegraphData () [ morsegraph_index ];
  					uint64_t other_dag_index = morsegraph_record . dag_index;
						if ( other_dag_index == dag_index ) {
							uint64_t pb = mgccp_record . parameter_indices [ 0 ];
							std::cout << mgcc << ", " << mgccp << ", " << pb << ": " <<  *database.parameter_space().parameter(pb) << "\n";
						}
					}
				}
				{
  				typedef std::pair<int, int> Edge;
  				BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
  					std::cout << e . first << " -> " << e . second << "; ";
  				}
  	  	}
        continue; //abort ();
			}
			// end debug

      ss << "digraph HASSE" << zoo_index << " { \n";
  		// Vertices
  		for ( int i = 0; i < dag . num_vertices; ++ i ) {
    		ss << i << " [label=\"" << i << "\"]\n";
  		}  
	 		ss << "LEGEND [label=\"HASSE " << zoo_index << "\\n " << 100.0*((double)frequency/(double)total_count) << "% \" shape=\"rectangle\"]\n";
  		{
  		typedef std::pair<int, int> Edge;
  		BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
  			ss << e . first << " -> " << e . second << "; ";
  		}
  	  }
			ss << "}\n";
	  }
		// Write File
		// data
		std::ofstream outfile ( filename . c_str () );
		// algo
		outfile << ss . str ();
		outfile . close ();
	}
}


void RAWHasseDebug ( const Database & database ) {
	for ( uint64_t dag_index = 0; dag_index < database . dagData() . size (); ++ dag_index ) {
		const DAG_Data & dag = database.dagData()[dag_index];
		std::cout << "(" << dag_index << ", " << dag . num_vertices << ")\n";
	}
}

int main ( int argc, char * argv [] ) {
	// Load database
  Database database; 
  database . load ( argv [ 1 ] );

  std::cout << "Successfully loaded database.\n";
  //RAWHasseDebug ( database );
  //return 0;
  
#ifndef NOCONLEYINDEX
  database . makeAttractorsMinimal ();
#endif
  database . performTransitiveReductions ();
  
  // Display CI Zoo
#ifndef NOCONLEYINDEX
  std::string ci_zoo_string = CI_Zoo ( database );
  std::ofstream outfile ( "CI.html" );
  outfile << "<!DOCTYPE html><html><body>" << ci_zoo_string << "</body></html>\n";
  outfile . close ();
#endif
  CMG_Zoo ( database );
  MGCC_Zoo ( database );
  Hasse_Zoo ( database );

  return 0;
}
