// zookeeper.cpp

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
#include "boost/shared_ptr.hpp"
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

DAG makeDAG ( const Database & database, uint64_t mgcc );

//

// split a string into different fields delimited by a colon ":"
// return the first field found
std::string extractSymbol ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  return "\""+fields[0]+"\"";
}
std::string extractSymbolWithoutQuotes ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  return fields[0];
}

std::string extractConditionalString ( const std::string & string ) {
  std::vector<std::string> fields;
  boost::split ( fields, string, boost::is_any_of (":") );
  // convention for the condition string from AnnotationConditions.h
  // symbol : description : extra information
  // the conditional string is simply symbol : description
  return fields[0] + ":" + fields[1];
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
    std::cout << "condition string format not implemented\n";
    abort();
  }
}


void insertMorseSetIntoDatabase ( sqlite3 * db,
                  int morsegraphid,
                  int morsegraphfileid,
                  int morsesetid,
                  const std::vector < std::string > & myannotations ) {
  bool done;
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
  }
  std::vector <SQLColumnData > data;
  data . push_back ( SQLColumnData("MORSEGRAPHID", morsegraphid) );
  data . push_back ( SQLColumnData("MORSEGRAPHFILEID", morsegraphfileid) );
  data . push_back ( SQLColumnData("MORSESETID", morsesetid) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION0STRING),fp) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION1STRING),fpoff) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION2STRING),fpon) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION3STRING),fc) );
  data . push_back ( SQLColumnData(extractSymbol(CONDITION4STRING),xc) );
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
  BOOST_FOREACH ( int64_t string_index, ar . string_indices ) {
    annotation_string . push_back ( database . stringData () [ string_index ] );
  }
  result . annotation = annotation_string;
  //
  // Annotation of the morse graph vertices
  //
  std::vector<uint64_t> const& annotation_index_by_vertex =  morsegraph_record . annotation_index_by_vertex;
  std::vector<std::vector<std::string> > mystring;
  BOOST_FOREACH ( uint64_t index, annotation_index_by_vertex ) {
    Annotation_Record const &arv = database . annotationData () [index];
		// int64_t string_index = * arv . string_indices . begin ();
    std::vector<std::string> newstring;
    BOOST_FOREACH ( int64_t string_index, arv . string_indices ) {
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
  BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
    DAG::Edge new_edge ( e . first, e . second );
    result . edges_ . insert ( new_edge );
  }
  return result;
}



std::string dotFile ( const Database & database,
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
      insertMorseSetIntoDatabase ( sqldb, mgcc, order_index, incc_index, annotation_vertex );
    } else {
      std::cout << "No annotation for vertex : " << i << "\n";
    }
    ss << i << " [label=\""<< incc_index << mystring << "\" href=\"javascript:void(click_node_on_graph"
    "('Unknown Conley Index'," << order_index << "))\"]\n";
#endif
  }


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
  ss << "LEGEND [label=\"MGCC " << mgcc << "\\n " << 100.0*frequency << "% \" shape=\"";
  ss << shapestr << "\"]\n";

 	// Edges
  typedef std::pair<int, int> Edge;
  BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
    ss << e . first << " -> " << e . second << "; ";
  }
  ss << "}\n";
  return ss. str ();
}



void MGCC_Zoo ( const Database & database,
                sqlite3 * sqldb ) {
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
        //		std::cout << frequency << " " << mgcc << " " << dotFile ( database, mgcc, mgcc_zoo_index, (double) frequency / (double) total_count, legend ) << "\n";
		// Produce filename
		// data
		std::string filename;
		// algo
		std::stringstream ss;
		ss << "MGCC" << mgcc_zoo_index << ".gv";
		filename = ss . str ();
		// Create File
		std::ofstream outfile ( filename . c_str () );
		outfile << dotFile ( database, mgcc, mgcc_zoo_index , (double) frequency / (double) total_count, sqldb );
		outfile . close ();
	}
}




int main ( int argc, char * argv [] ) {
	// Load database
  Database database;
  database . load ( argv [ 1 ] );
  std::cout << "Successfully loaded database.\n";
  
  // Open the SQL Database and create the main table if needed
  sqlite3 *sqlDB;

  char *zErrMsg = 0;
  int rc;
  rc = sqlite3_open ( "database.sql", &sqlDB );
  if ( rc ) {
    std::cout << "Cannot open the database : " << sqlite3_errmsg(sqlDB) <<"\n";
    exit (0);
  } else {
    std::cout << "Opened SQL database successfully\n";
  }

  /* THIS PART IS HARD CODED SHOULD BE DONE AUTOMATICALLY */
  std::vector<SQLColumn> columns;
  std::string symbol;
  std::string datatype;

  columns . push_back ( SQLColumn("MORSEGRAPHID","INT") );
  columns . push_back ( SQLColumn("MORSEGRAPHFILEID","INT") );
  columns . push_back ( SQLColumn("MORSESETID","INT") );

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

  createMainTableSQLDatabase ( sqlDB, "MORSESETS", columns );


  MGCC_Zoo ( database, sqlDB );

  return 0;
}



//
//
//
//
//// Go through the annotations text file
//// constuct a basic legend from them
//Legend makeLegend ( const char * path ) {
//    Legend allAnnotations;
//    std::vector < std::string > allAnnotationsString;
//    std::ifstream ifile;
//    std::string filename ( path );
//    filename += "/annotations.txt";
//    
//    std::cout << filename << "\n";
//    
//    ifile . open ( filename . c_str() );
//    //
//    std::string line;
//    while ( getline ( ifile, line ) ) {
//        allAnnotations [ line ] = extractSymbolWithoutQuotes(line);
//    }
//    return allAnnotations;
//}
//
//
////
//void saveLegend ( const Legend & legend ) {
//    
//    // Save the legend for the webpage in graphivz format
//    std::ofstream ofile;
//    ofile . open ( "legend.gv" );
//    ofile << "digraph {\n";
//    ofile << "rankdir=\"LR\"\n";
//    ofile << "node [shape=plaintext]\n";
//    ofile << "subgraph cluster_01 {\n";
//    ofile << "label = \"Legend\";\n";
//    ofile << "key [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder    =\"0\">\" \n";
//    
//    Legend::const_iterator ita;
//    // legend for the Morse Sets
//    //    for ( ita=legend.begin(); ita!=legend.end(); ++ita ) {
//    //        ofile << "<tr><td align=\"right\" port=\"i" << std::distance(legend.begin(),ita) << "\"> ";
//    //        ofile << ita -> second << " : " << ita -> first << "</td></tr>\n";
//    //    }
//    
//    ofile << "<tr><td align=\"right\" port=\"i1\"> FP : Fixed point </td></tr>\n";
//    ofile << "<tr><td align=\"right\" port=\"i2\"> FP on : Fixed point with all states on </td></tr>\n";
//    ofile << "<tr><td align=\"right\" port=\"i3\"> FP off : Fixed point with all states off </td></tr>\n";
//    ofile << "<tr><td align=\"right\" port=\"i4\"> FC : Morse set in which every variable makes a transition </td></tr>\n";
//    
//    ofile << "</table>>]\n";
//    // Legend for the Morse Graph
//    //    ofile << "LEGENDMG [label=\"*** AND !** AND *\" shape=\""<<MGshapegood<<"\"]";
//    ofile << "LEGENDMG [label=\"FC AND !(FP on) AND (FP off)\" shape=\""<<MGshapegood<<"\"]";
//    
//    ofile << "}\n";
//    ofile << "}\n";
//    ofile . close ( );
//}
//
//
//
//
//
//
//std::string conleyStringForZoo ( const Database & database,
//                                const CI_Data & ci ) {
//    // data
//  	std::string ci_string;
//  	// algo
//  	std::stringstream ss;
//  	ss << "(";
//  	for ( int d = 0; d < ci . conley_index . size (); ++ d ) {
//  		std::string s = ci . conley_index [ d ];
//  		std::string t;
//  		for ( int i = 0; i < s . size () - 1; ++ i ) {
//  			//if ( s [ i ] == '.' ) continue;
//  			//if ( s [ i ] == ' ' ) continue;
//  			if ( s [ i ] == '\n' ) { t += string ( ", "); continue; }
//  			t . push_back ( s [ i ] );
//  		}
//  		ss << t;
//  		if ( d != ci . conley_index . size () - 1 ) ss << ", ";
//  	}
//  	ss << ")";
//    ci_string = ss . str ();
//    return ci_string;
//}
//
//std::string CI_Zoo ( const Database & database ) {
//    std::stringstream result_ss;
//	// Create data structure to get INCC from Conley Record Index
//	// data
//	boost::unordered_map < uint64_t, boost::unordered_set < uint64_t > > ci_to_incc;
//	// algo
//	const std::vector < uint64_t > & incc_conley = database . incc_conley ();
//	for ( uint64_t incc = 0; incc < incc_conley . size (); ++ incc ) {
//		ci_to_incc [ incc_conley [ incc ] ] . insert ( incc );
//	}
//	// Create list of pairs (frequency of ci, ci index)
//	//data
//	std::vector < std::pair< long, std::string > > ci_zoo_data;
//	//algo
//	long count = 0;
//    BOOST_FOREACH ( const CI_Data & ci, database . ciData () ) {
//        // Step 1. Obtain string from ci.
//        // data
//        std::string ci_string;
//        // algo
//        std::stringstream ss;
//        ss << "CI " << count << ": " << conleyStringForZoo ( database, ci ) << "\n";
//		ci_string = ss . str ();
//		// Step 2. Obtain Frequency of CI
//		// data
//		long ci_frequency = 0;
//		// algo
//		BOOST_FOREACH ( uint64_t incc, ci_to_incc [ count ] ) {
//			ci_frequency += database . incc_sizes () [ incc ];
//		}
//		// Step 3. Push result onto data list
//		ci_zoo_data . push_back ( std::make_pair ( ci_frequency, ci_string ));
//		++ count;
//    }
//    
//    std::sort ( ci_zoo_data . rbegin (), ci_zoo_data . rend () ); // sort in descending order
//    
//    // DISPLAY CI ZOO DATA
//    for ( uint64_t ci_zoo_index = 0; ci_zoo_index < ci_zoo_data . size (); ++ ci_zoo_index ) {
//        result_ss << "<p>" << ci_zoo_data [ ci_zoo_index ] . first << " " << ci_zoo_data [ ci_zoo_index ] . second << "</p>\n";
//    }
//    return result_ss . str ();
//}
//
//std::string dotFile ( const Database & database,
//                     uint64_t mgcc,
//                     uint64_t order_index,
//                     double frequency,
//                     Legend & legend ) {
//	std::stringstream ss;
//	ss << "digraph MGCC" << order_index << " { \n";
//	
//	MGCC_Record const& mgcc_record = database . MGCC_Records () [ mgcc ];
//    uint64_t mgccp_index = mgcc_record . mgccp_indices [ 0 ];
//    const MGCCP_Record & mgccp_record = database . MGCCP_Records () [ mgccp_index ];
//    uint64_t morsegraph_index = mgccp_record . morsegraph_index;
//    const MorseGraphRecord & morsegraph_record =
//  	database . morsegraphData () [ morsegraph_index ];
//    uint64_t dag_index = morsegraph_record . dag_index;
//    const DAG_Data & dag = database . dagData () [ dag_index ];
//    
//    DAG mydag = makeDAG ( database, mgcc );
//    
//    // Vertices
//    for ( int i = 0; i < dag . num_vertices; ++ i ) {
//        CS_Data cs_data;
//        cs_data . vertices . push_back ( i );
//        uint64_t cs_index = database . csIndex ( cs_data );
//        INCCP_Record inccp_record;
//        inccp_record . cs_index = cs_index;
//        inccp_record . mgccp_index = mgccp_index;
//        uint64_t inccp_index = database.inccpIndex ( inccp_record );
//        uint64_t incc_index = database.inccp_to_incc () [ inccp_index ];
//#ifndef NOCONLEYINDEX
//        uint64_t conley = database . incc_conley () [ incc_index ];
//        const CI_Data & ci = database . ciData () [ conley ];
//        
//        ss << i << " [label=\""<< incc_index << "\" href=\"javascript:void(click_node_on_graph('"
//        <<  conleyStringForZoo ( database, ci ) << "'," << order_index << "))\"]\n";
//#else
//        
//        // string for the vertex
//        std::vector<std::string> annotation_vertex = mydag . annotation_vertex[i];
//        std::string mystring = "";
//        if ( !annotation_vertex.empty() ) {
//            //        for ( unsigned int j=0; j<annotation_vertex.size()-1; ++j ) {
//            //            Legend::iterator itl = legend.find(annotation_vertex[j]);
//            //            if ( itl == legend.end() ) {
//            //                std::cout << "Got one : " << annotation_vertex[j] << "\n";
//            //                mystring += annotation_vertex[j] + " , ";
//            //            } else {
//            //                mystring += betterLegend ( annotation_vertex[j] );
//            //                mystring += legend [ annotation_vertex[j] ] + ",";
//            //            }
//            //        }
//            //        Legend::iterator itl = legend.find(annotation_vertex[annotation_vertex.size()-1]);
//            //        if ( itl == legend.end() ) {
//            //            mystring += annotation_vertex [ annotation_vertex.size()-1 ];
//            //        } else {
//            //            mystring += betterLegend (annotation_vertex[annotation_vertex.size()-1]);
//            //            mystring += legend [ annotation_vertex[annotation_vertex.size()-1] ];
//            //          }
//            
//            mystring = betterLegend ( annotation_vertex );
//            
//            
//        } else {
//            std::cout << "No annotation for vertex : " << i << "\n";
//        }
//        
//        ss << i << " [label=\""<< incc_index << mystring << "\" href=\"javascript:void(click_node_on_graph"
//        "('Unknown Conley Index'," << order_index << "))\"]\n";
//        
//        
//        // ss << i << " [label=\""<< incc_index << "\" href=\"javascript:void(click_node_on_graph"
//        //               "('Unknown Conley Index'," << order_index << "))\"]\n";
//        
//#endif
//    }
// 	
//    
//    std::vector < std::string > annotationMG = mydag . annotation;
//    std::string shapestr;
//    for ( unsigned int i=0; i<annotationMG.size(); ++i ) {
//        if ( annotationMG[i] == "GOOD" ) {
//            shapestr = MGshapegood;
//        }
//        if ( annotationMG[i] == "BAD" ) {
//            shapestr = MGshapebad;
//        }
//    }
// 	ss << "LEGEND [label=\"MGCC " << mgcc << "\\n " << 100.0*frequency << "% \" shape=\"";
//    ss << shapestr << "\"]\n";
//    
// 	// Edges
//    typedef std::pair<int, int> Edge;
//    BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
//        ss << e . first << " -> " << e . second << "; ";
//    }
//	ss << "}\n";
//	return ss. str ();
//}
//
//
//std::string CMGdotFile ( const DAG & dag, uint64_t order_index, double frequency,
//                        Legend & legend ) {
//	std::stringstream ss;
//	ss << "digraph CMG" << order_index << " { \n";
//    // Vertices
//    for ( int i = 0; i < dag . num_vertices_; ++ i ) {
//        // string for the vertex
//        std::vector<std::string> annotation_vertex = dag . annotation_vertex[i];
//        std::string mystring = "";
//        
//        //      if ( !annotation_vertex . empty() ) {
//        //          for ( unsigned int j=0; j<annotation_vertex.size()-1; ++j ) {
//        //              mystring += legend [ annotation_vertex[j] ] + ",";
//        //          }
//        //        mystring += legend [ annotation_vertex[annotation_vertex.size()-1] ];
//        //      }
//        
//        //
//        //      for ( unsigned int j=0; j<annotation_vertex.size(); ++j ) {
//        //          std::cout << annotation_vertex[j] << " , ";
//        //      }
//        //      std::cout <<"\n";
//        //
//        //
//        //      if ( !annotation_vertex.empty() ) {
//        //          std::cout << "size is : " << annotation_vertex.size() << "\n";
//        //          for ( unsigned int j=0; j<annotation_vertex.size()-1; ++j ) {
//        //              std::cout << "we have : " << annotation_vertex[j] << "\n";
//        //              Legend::iterator itl = legend.find(annotation_vertex[j]);
//        //              if ( itl == legend.end() ) {
//        //                  std::cout << "Got one : " << annotation_vertex[j] << "\n";
//        //                  mystring += annotation_vertex[j] + ",";
//        //              } else {
//        //                  mystring += legend [ annotation_vertex[j] ] + ",";
//        //              }
//        //          }
//        //          mystring += legend [ annotation_vertex[annotation_vertex.size()-1] ];
//        //      }
//        //
//        
//        mystring = betterLegend ( annotation_vertex );
//        
//        
//        ss << i << " [label=\""<< dag . labels_ [ i ] << " " << mystring << "\"];\n";
//    }
//    
//    
// 	// ss << "LEGEND [label=\"CMG " << order_index << "\\n " << 100.0*frequency << "% \" shape=\"rectangle\"]\n";
//    
// 	ss << "LEGEND [label=\"CMG " << order_index << "\\n " << 100.0*frequency << "%  \\n";
//    std::vector < std::string > annotationMG = dag . annotation;
//    std::string shapestr;
//    for ( unsigned int i=0; i<annotationMG.size(); ++i ) {
//        if ( annotationMG[i] == "GOOD" ) {
//            shapestr = MGshapegood;
//        }
//        if ( annotationMG[i] == "BAD" ) {
//            shapestr = MGshapebad;
//        }
//    }
//    
//    ss << "\" shape=\"" << shapestr << "\"]\n";
//    
// 	// Edges
//    BOOST_FOREACH ( const DAG::Edge & e, dag . edges_ ) {
//        ss << e . first << " -> " << e . second << "; ";
//    }
//	ss << "}\n";
//	return ss. str ();
//}
//
//
//void MGCC_Zoo ( const Database & database,
//                Legend & legend ) {
//	// Sort mgcc by frequency
//	// data
//	std::vector < std::pair< long, uint64_t > > mgcc_sorted_by_frequency;
//	uint64_t total_count = 0;
//	// algo
//	for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
//		long frequency = 0;
//		const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
//		BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
//			const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
//			frequency += mgccp_record . parameter_indices . size ();
//		}
//		mgcc_sorted_by_frequency . push_back ( std::make_pair ( frequency, mgcc ) );
//		total_count += frequency;
//	}
//	std::sort ( mgcc_sorted_by_frequency . rbegin (), mgcc_sorted_by_frequency . rend () ); // sort in descending order
//    
//	// DISPLAY MGCC ZOO DATA
//	for ( uint64_t mgcc_zoo_index = 0; mgcc_zoo_index < mgcc_sorted_by_frequency . size (); ++ mgcc_zoo_index ) {
//		long frequency = mgcc_sorted_by_frequency [ mgcc_zoo_index ] . first;
//		uint64_t mgcc = mgcc_sorted_by_frequency [ mgcc_zoo_index ] . second;
//		//if ( frequency < 10 ) break;
//        //		std::cout << frequency << " " << mgcc << " " << dotFile ( database, mgcc, mgcc_zoo_index, (double) frequency / (double) total_count, legend ) << "\n";
//		// Produce filename
//		// data
//		std::string filename;
//		// algo
//		std::stringstream ss;
//		ss << "MGCC" << mgcc_zoo_index << ".gv";
//		filename = ss . str ();
//		// Create File
//		std::ofstream outfile ( filename . c_str () );
//		outfile << dotFile ( database, mgcc, mgcc_zoo_index , (double) frequency / (double) total_count, legend );
//		outfile . close ();
//	}
//}
//
//void CMG_Zoo ( const Database & database,
//              Legend & legend ) {
//	// Sort mgcc by frequency
//	// data
//	boost::unordered_map<DAG, int64_t> cmgs_and_count;
//	uint64_t total_count = 0;
//	// algo
//	for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
//		long frequency = 0;
//		const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
//		BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
//			const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
//			frequency += mgccp_record . parameter_indices . size ();
//		}
//		DAG dag = makeDAG ( database, mgcc );
//        if ( dag . num_vertices_ == 0 ) std::cout << "detected empty dag\n";
//		cmgs_and_count [ dag ] += frequency;
//		total_count += frequency;
//	}
//	std::vector < std::pair < int64_t, DAG > > data_to_sort;
//	typedef std::pair < DAG, int64_t > value_t;
//	BOOST_FOREACH ( value_t v, cmgs_and_count ) {
//		data_to_sort . push_back ( std::make_pair ( v . second, v . first ) );
//	}
//    
//    //  std::cout << "data_to_sort . size () ==  " << data_to_sort . size () << "\n";
//	std::sort ( data_to_sort . rbegin (), data_to_sort . rend () ); // sort in descending order
//    //  std::cout << "data_to_sort sorted.\n";
//	// DISPLAY CMG ZOO DATA
//	for ( uint64_t cmg_zoo_index = 0; cmg_zoo_index < data_to_sort . size (); ++ cmg_zoo_index ) {
//		long frequency = data_to_sort [ cmg_zoo_index ] . first;
//		DAG dag = data_to_sort [ cmg_zoo_index ] . second;
//		// Produce filename
//		// data
//		std::string filename;
//		// algo
//		std::stringstream ss;
//		ss << "CMG" << cmg_zoo_index << ".gv";
//		filename = ss . str ();
//		// Create File
//		std::ofstream outfile ( filename . c_str () );
//		outfile << CMGdotFile ( dag, cmg_zoo_index , (double) frequency / (double) total_count, legend);
//		outfile . close ();
//	}
//}
//
//void Hasse_Zoo ( const Database & database ) {
//	// Sort mgcc by frequency
//	// data
//	std::vector < std::pair < long, uint64_t > > dag_indices_by_frequency;
//	uint64_t total_count = 0;
//    
//	// algo
//	{
//		dag_indices_by_frequency . resize ( database.dagData() . size () );
//		for ( uint64_t dag = 0; dag < dag_indices_by_frequency . size (); ++ dag ) {
//			dag_indices_by_frequency [ dag ] . first = 0;
//			dag_indices_by_frequency [ dag ] . second = dag;
//		}
//		for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
//			const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
//			BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
//				const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
//				long frequency = mgccp_record . parameter_indices . size ();
//				uint64_t morsegraph_index = mgccp_record . morsegraph_index;
//                const MorseGraphRecord & morsegraph_record =
//  				database . morsegraphData () [ morsegraph_index ];
//                uint64_t dag_index = morsegraph_record . dag_index;
//				dag_indices_by_frequency [ dag_index ] . first += frequency;
//				total_count += frequency;
//			}
//		}
//		std::sort ( dag_indices_by_frequency . rbegin (), dag_indices_by_frequency . rend () ); // sort in descending order
//	}
//	// DISPLAY HASSE ZOO DATA
//	for ( uint64_t zoo_index = 0; zoo_index < dag_indices_by_frequency . size (); ++ zoo_index ) {
//		long frequency = dag_indices_by_frequency [ zoo_index ] . first;
//		uint64_t dag_index = dag_indices_by_frequency [ zoo_index ] . second;
//		// Produce filename
//		// data
//		std::string filename;
//		// algo
//		{
//			std::stringstream ss;
//			ss << "HASSE" << zoo_index << ".gv";
//			filename = ss . str ();
//        }
//		// Create Body of File
//		// data
//		std::stringstream ss;
//		// algo
//		{
//            
//            const DAG_Data & dag = database . dagData () [ dag_index ];
//            
//            
//            // debug
//			if ( dag . num_vertices == 0 ) {
//				std::cout << "EMPTY HASSE DIAGRAM. It seems a bug is present.)\n";
//				std::cout << "database.dagData()[" << dag_index << "]\n";
//				for ( uint64_t mgcc = 0; mgcc < database.MGCC_Records().size (); ++ mgcc ) {
//					const MGCC_Record & mgcc_record = database.MGCC_Records()[mgcc];
//					BOOST_FOREACH ( uint64_t mgccp, mgcc_record . mgccp_indices ) {
//						const MGCCP_Record & mgccp_record = database.MGCCP_Records()[mgccp];
//						uint64_t morsegraph_index = mgccp_record . morsegraph_index;
//                        const MorseGraphRecord & morsegraph_record =
//                        database . morsegraphData () [ morsegraph_index ];
//                        uint64_t other_dag_index = morsegraph_record . dag_index;
//						if ( other_dag_index == dag_index ) {
//							uint64_t pb = mgccp_record . parameter_indices [ 0 ];
//							std::cout << mgcc << ", " << mgccp << ", " << pb << ": " <<  *database.parameter_space().parameter(pb) << "\n";
//						}
//					}
//				}
//				{
//                    typedef std::pair<int, int> Edge;
//                    BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
//                        std::cout << e . first << " -> " << e . second << "; ";
//                    }
//                }
//                continue; //abort ();
//			}
//			// end debug
//            
//            ss << "digraph HASSE" << zoo_index << " { \n";
//            // Vertices
//            for ( int i = 0; i < dag . num_vertices; ++ i ) {
//                ss << i << " [label=\"" << i << "\"]\n";
//            }
//	 		ss << "LEGEND [label=\"HASSE " << zoo_index << "\\n " << 100.0*((double)frequency/(double)total_count) << "% \" shape=\"rectangle\"]\n";
//            {
//                typedef std::pair<int, int> Edge;
//                BOOST_FOREACH ( const Edge & e, dag . partial_order ) {
//                    ss << e . first << " -> " << e . second << "; ";
//                }
//            }
//			ss << "}\n";
//        }
//		// Write File
//		// data
//		std::ofstream outfile ( filename . c_str () );
//		// algo
//		outfile << ss . str ();
//		outfile . close ();
//	}
//}
//
//
//void RAWHasseDebug ( const Database & database ) {
//	for ( uint64_t dag_index = 0; dag_index < database . dagData() . size (); ++ dag_index ) {
//		const DAG_Data & dag = database.dagData()[dag_index];
//        //		std::cout << "(" << dag_index << ", " << dag . num_vertices << ")\n";
//	}
//}








