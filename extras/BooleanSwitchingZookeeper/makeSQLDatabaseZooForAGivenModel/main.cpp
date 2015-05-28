
// We add to a global SQL database a new database record

#define NOCONLEYINDEX
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
#include <memory>
#include "boost/unordered_set.hpp"
#include "boost/unordered_map.hpp"
#include "../examples/BooleanSwitching/Parameter/BooleanSwitchingParameterSpace.h"

#include "GI.h"

#include "../examples/BooleanSwitching/AnnotationConditions.h"
#include <boost/algorithm/string.hpp>


// SQL header
#include "sql.h"


typedef boost::unordered_map < std::string, std::string > Legend;

// Shape to identify if a morse graph is good or not
#define MGshapegood "doubleoctagon"
#define MGshapebad "rectangle"


// insert a record in the SQL database for a given morse set
void processMorseSet (  sqlite3 * db,
                        int morsegraphid,
                        int morsegraphfileid,
                        int morsesetid,
                        const std::vector < std::string > & myannotations );

/// makeDAG
DAG makeDAG ( const Database & database, uint64_t mgcc );


// split a string into different fields delimited by a colon ":"
// return the first field found
std::string extractSymbol ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  if ( fields . size () >= 1 ) return "\""+fields[0]+"\"";
  else return "\"\""; // DEBUG
}
std::string extractSymbolWithoutQuotes ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  if ( fields . size () >= 1 ) return fields[0];
  else return std::string (); // DEBUG
}

std::string extractConditionalString ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  // convention for the condition string from AnnotationConditions.h
  // symbol : description : extra information
  // the conditional string is simply symbol : description
  if ( fields . size () >= 2 ) return fields[0] + ":" + fields[1];
  else return std::string (); // DEBUG
}

std::string constructLabel ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  if ( fields.size() == 2 ) {
    return "&#92;n " + extractSymbolWithoutQuotes(string);
  }
  if ( fields.size() == 3 ) {
    std::string str;
    str = "&#92;n " + extractSymbolWithoutQuotes(string) + "&#92;n { ";
    std::vector<std::string> extrafields;
    boost::split ( extrafields, fields[2], boost::is_any_of (" ") );
    // Warning : need -2 because there is an extra space at the end of the string
    for ( unsigned int i=0; i<extrafields.size()-2; ++i ) {
      str += extrafields[i] + ", ";
    }
    str += extrafields[extrafields.size()-2] + " }";
    return str;
  } else {
    std::cout << "string = " << string << "\n";
    std::cout << "condition string format not implemented\n";
    return std::string (); // TODO: Arnaud, could you take a look at this? 9-3-2014
    //abort();
  }
}

struct sqlMSdatatype {
  int fp=0;
  int fpon=0;
  int fpoff=0;
  int fc=0;
  int xc=0;
};


void updateMorseSetSQLcolumns ( sqlMSdatatype & dt,
                    const std::vector < std::string > & myannotations) {
  for ( unsigned int i=0; i<myannotations.size(); ++i ) {
    if ( extractConditionalString(myannotations[i]) == CONDITION0STRING ) {
      dt.fp = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION1STRING ) {
      dt.fpoff = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION2STRING ) {
      dt.fpon = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION3STRING ) {
      dt.fc = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION4STRING ) {
      dt.xc = 1;
    }
  }
}



void insertMorseSetIntoDatabase ( sqlite3 * db,
                                 int permutationid,
                  int morsegraphid,
//                  int morsesetid,
//                  const std::vector < std::string > & myannotations ) {
                  const sqlMSdatatype & sqldt ) {
/*  bool done;
  done = false;
  int fp,fpon,fpoff,fc,xc;
  fp=0;
  fpon=0;
  fpoff=0;
  fc=0;
  xc=0;
  for ( unsigned int i=0; i<myannotations.size(); ++i ) {
    if ( extractConditionalString(myannotations[i]) == CONDITION0STRING ) {
      fp = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION1STRING ) {
      fpoff = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION2STRING ) {
      fpon = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION3STRING ) {
      fc = 1;
    }
    if ( extractConditionalString(myannotations[i]) == CONDITION4STRING ) {
      xc = 1;
    }
  }*/
  std::vector <SQLColumnData > data;
  data . push_back ( SQLColumnData("PERMUTATIONID", permutationid) );
  data . push_back ( SQLColumnData("MORSEGRAPHID", morsegraphid) );
//  data . push_back ( SQLColumnData("MORSESETID", morsesetid) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION0STRING),sqldt.fp) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION1STRING),sqldt.fpoff) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION2STRING),sqldt.fpon) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION3STRING),sqldt.fc) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION4STRING),sqldt.xc) );
  insertMorseSetRecord ( db, "morsesets", data );
}

//
// annotation convention   symbol : text
// example : "FP : Morse set is a fixed point"
// we scan the string to find the symbol to label the node in graphviz
//
std::string makeLabel ( const std::vector < std::string > & myannotations ) {
  std::string output;
  output = "";
  if ( myannotations.size() == 0 ) {
    std::cout << "No annotations.\n";
    abort();
  }
  for ( unsigned int i=0; i<myannotations.size(); ++i ) {
    // we concatenate all the labels from all the annotations
    output += constructLabel(myannotations[i]);
  }
  return output;
}


DAG makeDAG ( const Database & database, uint64_t mgcc ) {
	DAG result;
	MGCC_Record const& mgcc_record = database . MGCC_Records () [ mgcc ];
  uint64_t mgccp_index = mgcc_record . mgccp_indices [ 0 ];
  const MGCCP_Record & mgccp_record = database . MGCCP_Records () [ mgccp_index ];
  uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  const MorseGraphRecord & morsegraph_record =
  database . morsegraphData () [ morsegraph_index ];
  //
  // Annotation of the morse graph
  //
  int64_t const&annotation_index = morsegraph_record . annotation_index;
  Annotation_Record const& ar = database . annotationData () [annotation_index];
	// int64_t string_index = * ar . string_indices . begin ();
  std::vector<std::string> annotation_string;
  for ( int64_t string_index : ar . string_indices ) {
    annotation_string . push_back ( database . stringData () [ string_index ] );
  }
  result . annotation = annotation_string;
  //
  // Annotation of the morse graph vertices
  //
  std::vector<uint64_t> const& annotation_index_by_vertex =  morsegraph_record . annotation_index_by_vertex;
  std::vector<std::vector<std::string> > mystring;
  for ( uint64_t index : annotation_index_by_vertex ) {
    Annotation_Record const &arv = database . annotationData () [index];
		// int64_t string_index = * arv . string_indices . begin ();
    std::vector<std::string> newstring;
    for ( int64_t string_index : arv . string_indices ) {
      newstring . push_back ( database . stringData () [ string_index ] );
    }
    mystring . push_back ( newstring );
  }
  result . annotation_vertex = mystring;
  //
  //
  //
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
    //    result . labels_ [ i ] = std::string ( "Unknown Conley Index" );
    result . labels_ [ i ] = std::string ( "" );
#endif
  }
 	// Edges
  typedef std::pair<int, int> Edge;
  for ( const Edge & e : dag . partial_order ) {
    DAG::Edge new_edge ( e . first, e . second );
    result . edges_ . insert ( new_edge );
  }
  return result;
}



std::string dotFile ( const Database & database,
                     int & permutationid,
                      uint64_t mgcc,
                      uint64_t order_index,
                      double frequency,
                      sqlite3 * sqldb ) {
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

  DAG mydag = makeDAG ( database, mgcc );

  SQLMorseGraphData sqldata;
  sqldata . permutationId = permutationid;
  sqldata . morseGraphFileId = order_index;
  sqldata . morseGraphId = mgcc;
  sqldata . percentage = frequency;
  insertMorseGraphRecord ( sqldb, "MORSEGRAPHS", sqldata );
 
  sqlMSdatatype msdt;
 
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

    // string for the vertex
    std::vector<std::string> annotation_vertex = mydag . annotation_vertex[i];
    
    std::string mystring = "";
    if ( !annotation_vertex.empty() ) {
      mystring = makeLabel ( annotation_vertex );
//      insertMorseSetIntoDatabase ( sqldb, mgcc, order_index, incc_index, annotation_vertex );
//      insertMorseSetIntoDatabase ( sqldb, permutationid, mgcc, incc_index, annotation_vertex );
  
updateMorseSetSQLcolumns (msdt, annotation_vertex );
    
    } else {
      std::cout << "No annotation for vertex : " << i << "\n";
    }
    ss << i << " [label=\""<< incc_index << mystring << "\" href=\"javascript:void(click_node_on_graph"
    "('Unknown Conley Index'," << order_index << "))\"]\n";
#endif
  }
  insertMorseSetIntoDatabase ( sqldb, permutationid, mgcc, msdt );

  std::vector < std::string > annotationMG = mydag . annotation;
  std::string shapestr;
  for ( unsigned int i=0; i<annotationMG.size(); ++i ) {
    if ( annotationMG[i] == "GOOD" ) {
      shapestr = MGshapegood;
    }
    if ( annotationMG[i] == "BAD" ) {
      shapestr = MGshapebad;
    }
  }
  ss << "LEGEND [label=\"MGCC " << mgcc << "\\n " << 100.0*frequency << "% \""
     << " href=\"inequalities.html?mgcc=" << order_index << "\" " 
     << "shape=\"" << shapestr << "\"]\n";

 	// Edges
  typedef std::pair<int, int> Edge;
  for ( const Edge & e : dag . partial_order ) {
    ss << e . first << " -> " << e . second << "; ";
  }
  ss << "}\n";
  return ss. str ();
}



void MGCC_Zoo ( Database const& database,
                sqlite3 * sqldb,
               int & permutationid,
                std::shared_ptr<BooleanSwitchingParameterSpace> parameter_space = 
                  std::shared_ptr<BooleanSwitchingParameterSpace> () ) {

	// Sort mgcc by frequency
	// data
	std::vector < std::pair< long, uint64_t > > mgcc_sorted_by_frequency;
	uint64_t total_count = 0;
	// algo
	for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
		long frequency = 0;
		const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
		for ( uint64_t mgccp : mgcc_record . mgccp_indices ) {
			const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
			frequency += mgccp_record . parameter_indices . size ();
		}
		mgcc_sorted_by_frequency . push_back ( std::make_pair ( frequency, mgcc ) );
		total_count += frequency;
	}
	std::sort ( mgcc_sorted_by_frequency . rbegin (), mgcc_sorted_by_frequency . rend () ); // sort in descending order

	// DISPLAY MGCC ZOO DATA
	for ( uint64_t order_index = 0; order_index < mgcc_sorted_by_frequency . size (); ++ order_index ) {
		long frequency = mgcc_sorted_by_frequency [ order_index ] . first;
		uint64_t mgcc = mgcc_sorted_by_frequency [ order_index ] . second;
    // Create Dot File
    {
		std::string filename;
		std::stringstream ss;
		ss << "MGCC" << order_index << ".gv";
		filename = ss . str ();
		std::ofstream outfile ( filename . c_str () );
		outfile << dotFile ( database, permutationid, mgcc, order_index , (double) frequency / (double) total_count, sqldb );
		outfile . close ();
    }
    // Create Parameter File
    if ( parameter_space ) {
      std::string filename;
      std::stringstream ss;
      ss << "MGCC" << order_index << ".txt";
      filename = ss . str ();
      std::ofstream outfile ( filename . c_str () );
      outfile << "MGCC " << mgcc << ": There are " << frequency << " parameters.\n";
      const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
      for ( uint64_t mgccp : mgcc_record . mgccp_indices ) {
        const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
        for ( uint64_t pi : mgccp_record . parameter_indices ) {
          outfile << "{\n";
          outfile << parameter_space -> prettyPrint ( parameter_space -> parameter ( pi ) );
          outfile << "}\n";
        }
      }
      outfile . close ();
    }
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


void initializeSQLDB ( sqlite3 *db ) {
  //
  createPermutationTableSQL ( db, "PERMUTATIONS" );
  createMorseGraphTableSQL ( db, "MORSEGRAPHS" );
  /* THIS PART IS HARD CODED SHOULD BE DONE AUTOMATICALLY */
  std::vector<SQLColumn> columns;
  std::string symbol;
  std::string datatype;
  columns . push_back ( SQLColumn("PERMUTATIONID","INT") );
  columns . push_back ( SQLColumn("MORSEGRAPHID","INT") );
  //columns . push_back ( SQLColumn("MORSESETID","INT") );
  datatype = "INT";
  symbol = extractSymbol ( CONDITION0STRING );
  columns . push_back ( SQLColumn (symbol,datatype) );
  
  symbol = extractSymbol ( CONDITION1STRING );
  columns . push_back ( std::pair<std::string,std::string> (symbol,datatype) );
  
  symbol = extractSymbol ( CONDITION2STRING );
  columns . push_back ( std::pair<std::string,std::string> (symbol,datatype) );
  
  symbol = extractSymbol ( CONDITION3STRING );
  columns . push_back ( std::pair<std::string,std::string> (symbol,datatype) );
  
  symbol = extractSymbol ( CONDITION4STRING );
  columns . push_back ( std::pair<std::string,std::string> (symbol,datatype) );
  
  createMorseSetTableSQL ( db, "MORSESETS", columns );
}


// Arguments :
//
// path/database.mdb path/network network.txt pathglobalSQLDatabase index nameofpermutation

int main ( int argc, char * argv [] ) {
  
  
  // Create the master database
  char *zErrMsg = 0;
  int rc;
  sqlite3 *masterDB;
  std::string masterDatabaseDirectory(argv[argc-3]);
  std::string masterDatabaseFilename = masterDatabaseDirectory + "/database.db";
  rc = sqlite3_open ( masterDatabaseFilename.c_str(), &masterDB );
  if ( rc ) {
    std::cout << "Cannot open the database : " << sqlite3_errmsg(masterDB) <<"\n";
    exit (0);
  } else {
    std::cout << "Opened SQL database successfully\n";
  }
  
  initializeSQLDB ( masterDB );
  
    //
    // insert permutation record
    SQLPermutationData pdata;
    pdata . permutationString = std::string(argv[argc-1]);
    pdata . permutationId = atoi(argv[argc-2]);
    insertPermutationRecord ( masterDB, "PERMUTATIONS", pdata );
    //
    // Load database
    Database database;
    std::string databasefilename = argv [ 1 ];
    database . load ( databasefilename.c_str() );
    std::cout << "Successfully loaded database.\n";
    //
  
#ifndef NOCONLEYINDEX
    database . makeAttractorsMinimal ();
#endif
    database . performTransitiveReductions ();
    
    std::shared_ptr<BooleanSwitchingParameterSpace> parameter_space;
    
//    if ( argc > 3 ) {
      // Load Network and Create Parameter Space
      parameter_space . reset ( new BooleanSwitchingParameterSpace );
      parameter_space -> initialize ( argc - 3, argv + 1 );
//    }
  
    MGCC_Zoo ( database, masterDB, pdata.permutationId, parameter_space );
    Hasse_Zoo ( database );
    
    parameter_space . reset ();
  
  return 0;
}









