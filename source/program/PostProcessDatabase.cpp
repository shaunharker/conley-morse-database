/*
 *  PostProcessDatabase.cpp
 */


#include <iostream>
#include <fstream>

#include "data_structures/Database.h"
#include "data_structures/UnionFind.hpp"

#include "boost/foreach.hpp"

#include "tools/visualization.h"

bool CheckIsomorphism (const ParameterBoxRecord & r1, 
                       const ParameterBoxRecord & r2,
                       const ClutchingRecord & c ) {
  if ( r1 . num_morse_sets_ != r2 . num_morse_sets_ ) return false;
  /* Method. First, generate a one-to-one correspondence from c. 
             If that cannot be done, return false.
             Next, use the correspondence to check for isomorphism by
             first translating each each in r1 to r2 and checking for its
             presence, and then vice-versa. */
  // cheat. for now, just check for a one-to-one
  std::map < int, int > forward;
  std::map < int, int > backward;
  typedef std::pair < int, int > Edge;
  BOOST_FOREACH ( const Edge & e, c . clutch_ ) {
    if ( forward . count ( e . first ) != 0 ) return false;
    if ( backward . count ( e . second ) != 0 ) return false;
    forward [ e . first ] = e . second;
    backward [ e . second ] = e . first;
  }
  // Now check for isomorphism
  
  return true;
}

void draw2DClasses (GraphicsWindow & window, 
                    std::map < int, int > & color_choice,
                    const Database & database,
                    const UnionFind < int > & classes ) {
  double l0 = 1000;
  double u0 = -1000;
  double l1 = 1000;
  double u1 = -1000;
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    l0 = std::min ( l0, record . ge_ . lower_bounds_ [ 0 ] );
    u0 = std::max ( u0, record . ge_ . upper_bounds_ [ 0 ] );
    l1 = std::min ( l1, record . ge_ . lower_bounds_ [ 1 ] );
    u1 = std::max ( u1, record . ge_ . upper_bounds_ [ 1 ] );
  }
  
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    double x = (record . ge_ . lower_bounds_ [ 0 ] - l0 ) / ( u0 - l0 );
    double y = (record . ge_ . lower_bounds_ [ 1 ] - l1 ) / ( u1 - l1 );
    double w = (record . ge_ . upper_bounds_ [ 0 ] - record . ge_ . lower_bounds_ [ 0 ] ) / ( u0 - l0 );
    double h = (record . ge_ . upper_bounds_ [ 1 ] - record . ge_ . lower_bounds_ [ 1 ] ) / ( u1 - l1 );
    window . rect ( color_choice [ classes . Representative ( record . id_ ) ], 
                   (int) ( 512.0 * x ), 
                   (int) ( 512.0 * ( (double) 1.0 - y ) ), 
                   (int) ( 512.0 * w ), 
                   (int) ( 512.0 * h ) );
  }
  
  window . wait ();
  char c;
  std::cin >> c;
}

void ContinuationClasses ( const Database & database ) {
  
  UnionFind < int > classes;
  std::map < int, ParameterBoxRecord > indexed_box_records;
  std::map < int, int> color_choice;
  
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    classes . Add ( record . id_ );
    color_choice [ record . id_ ] = rand () % 256;
    indexed_box_records [ record . id_ ] = record;
  }
  
  
  BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
    // CHECK IF ISOMORPHISM
    if ( CheckIsomorphism (indexed_box_records [ record . id1_ ], 
                           indexed_box_records [ record . id2_ ], record ) ) {
      classes . Union ( record . id1_, record . id2_ );
    }
  }
  
  GraphicsWindow window ( "Parameter Space\n" );
  draw2DClasses ( window, color_choice, database, classes );
}

class QueryData {
public:
  // TYPEDEFS
  typedef std::pair < int, int > intpair;
  typedef std::pair < intpair, intpair > Edge;
  typedef std::pair < const int, std::set < Edge > > mg_edges_t;
  
  // DATA
  Database database;
  
  /******* MGCC QUERY DATA ********/
  /// Union-find structure store Morse Graph Continuation Classes
  UnionFind < int > mgcc_uf; 
  /// Set of mgcc Representatives
  std::set < int > mgcc_rep;
  /// Map keyed by mgcc representative to give mgcc reps of neighboring classes
  std::map < int, std::set < int > > mgcc_nb; // Morse Graph Continuation Class nbr
  /// Map keyed by mgcc representative to give parameter boxes (pb's) in mgcc
  std::map < int, std::set < int > > mgcc_pb; // Morse Graph Continuation Class Parameter Boxes
  /// map to store morse graph edges (keyed by id of mgcc_uf representative)
  std::map < int, std::set < Edge > > mg_edges;
  
  /******* MSCC QUERY DATA ********/
  /// Union-find structure store Morse Set Continuation Classes
  UnionFind < intpair > mscc_uf;
  /// Set of Morse Set Continuation Classes (by representatives)
  std::set < intpair > mscc_rep; 
  /// Map keyed by mscc representative to give parameter boxes (pb's) in mscc
  std::map < intpair, std::set < int > > mscc_pb; // Morse Graph Continuation Class Parameter Boxes
  /// Map keyed by mscc representative to give reps of mgcc's it is part of
  std::map < intpair, std::set < int > > mscc_mgcc; // Morse Graph Continuation Class Parameter Boxes

  /// set of clutching edges between mscc_uf representatives
  std::set < Edge > clutch_edges;
  
  /******* PB QUERY DATA ********/
  /// map to store parameter records by their id_ field
  std::map < int, ParameterBoxRecord > indexed_box_records;

  // METHODS
  QueryData ( const Database & database ) : database ( database ) {
    // Loop through box records. 
    //       Index the Box records, and create a union-find structure for morse sets and boxes
    // initializes mgcc_ug, mgcc_rep, mscc_uf, and creates indexed_box_records
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      mgcc_uf . Add ( record . id_ );
      for ( int i = 0; i < record . num_morse_sets_; ++ i ) {
        mscc_uf . Add ( intpair ( record . id_, i ) );
      }
      indexed_box_records [ record . id_ ] = record;
    }
  
    // Loop through clutch records
    //     Determine isomorphisms and perform unions based on results
    // finishes: mgcc_uf, mscc_uf, mgcc_rep, mscc_rep
    BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
      if ( CheckIsomorphism (indexed_box_records [ record . id1_ ], 
                             indexed_box_records [ record . id2_ ], record ) ) {
        mgcc_uf . Union ( record . id1_, record . id2_ );
        BOOST_FOREACH ( const intpair & edge, record . clutch_ ) {
          mscc_uf . Union ( intpair ( record . id1_, edge . first ), 
                           intpair ( record . id2_, edge . second ) );
        }
      }
    }
        
    // Prepare mgcc_rep, mgcc_pb, mscc_rep, mscc_pb, and mscc_mgcc
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      int mgrep = mgcc_uf . Representative ( record . id_ );
      mgcc_rep . insert ( mgrep );
      mgcc_pb [ mgrep ] . insert ( record . id_ ); // record the parameter box
      // prepare non-empty entries
      if ( mg_edges . count ( mgrep ) == 0 ) mg_edges [ mgrep ] = std::set < Edge > ();      
      // check for violations in assumption
      for ( int i = 0; i < record . num_morse_sets_; ++ i ) {
        intpair morseset = intpair ( record . id_ , i );
        intpair msrep = mscc_uf . Representative ( morseset );
        mscc_rep . insert ( msrep ); 
        mscc_pb [ msrep ] . insert ( record . id_ );
        mscc_mgcc [ msrep ] . insert ( mgrep );
        if ( msrep . first == record . id_ && msrep . second != i ) {
          std::cout << "Violation in assumptions.\n";
        }
      }
    }
    
    // Generate morse graph edges: 
    // initializes mg_edges  (but does not perform transitive reduction)
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      BOOST_FOREACH ( const intpair & edge, record . partial_order_ ) {
        int rep_id = mgcc_uf . Representative ( record . id_ );
        intpair source = mscc_uf . Representative ( intpair ( record . id_, edge . first ) );
        intpair target = mscc_uf . Representative ( intpair ( record . id_, edge . second ) );
        if ( source == target ) continue;
        mg_edges [ rep_id ] . insert ( Edge ( source, target ) );
      }
    }
    
    // transitive reduction -- n^4 , should be n^3.
    // finishes mg_edges
    BOOST_FOREACH ( mg_edges_t & entry , mg_edges ) {
      std::set < Edge > & graph = entry . second;
      std::set < Edge > newgraph = graph;
      BOOST_FOREACH ( const Edge & e1, graph ) {
        BOOST_FOREACH ( const Edge & e2, graph ) {
          if ( e1 . second == e2 . first ) newgraph . erase ( Edge ( e1 . first, e2 . second ) );  
        }
      }
      std::swap ( graph, newgraph );
    }
    
    // Generate clutching edges
    // creates clutch_edges
    BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
      BOOST_FOREACH ( const intpair & edge, record . clutch_ ) {
        intpair source = mscc_uf . Representative ( intpair ( record . id1_, edge . first ) );
        intpair target = mscc_uf . Representative ( intpair ( record . id2_, edge . second ) );
        if ( source . first == target . first ) continue;
        if ( source . first > target . first ) std::swap ( source, target );
        clutch_edges . insert ( Edge ( source, target ) );
      }
    }
    
    // Generate Morse Graph Continuation Class neighbor information
    BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
      int source = mgcc_uf . Representative ( record . id1_ );
      int target = mgcc_uf . Representative ( record . id2_ );
      mgcc_nb [ source ] . insert ( target );
      mgcc_nb [ target ] . insert ( source );
    }
  }
  
  void save_clutching_graph ( const char * filename ) {
    std::ofstream ofs ( filename );
    
    // opening brace
    ofs << "digraph G {\n";
    
    unsigned int CUTOFF = 0;
    // morse graphs
    ofs << "node [label=\"\",width=.1,height=.1]\n";
    BOOST_FOREACH ( const mg_edges_t & entry, mg_edges ) {
      if ( mgcc_pb [ entry . first ] . size () < CUTOFF ) continue; // ignore small ones
      if ( mgcc_uf . Representative ( entry . first ) != entry . first ) std::cout << "Assumption violated\n";
      
      ofs << " subgraph cluster" << entry . first << " {\n";
      ofs << " label=\"" << mgcc_pb [ entry . first ] . size () << "\";\n";
      for ( int i = 0; i < indexed_box_records [ entry . first ] . num_morse_sets_; ++ i ) {
        ofs << "node B" << entry . first << "N" << i << ";\n";
      }
      BOOST_FOREACH ( const Edge & edge, entry . second ) {
        ofs << "B" << edge . first . first << "N" << edge . first . second;
        ofs << " -> ";
        ofs << "B" << edge . second . first << "N" << edge . second . second;
        ofs << "\n";
      }
      ofs << "}\n";
    }
    
    //clutching edges
    ofs << "edge [color=red,dir=none];\n";
    BOOST_FOREACH ( const Edge & edge, clutch_edges ) {
      if ( mgcc_pb [ edge . first . first ] . size ()< CUTOFF ) continue; // ignore small ones
      if ( mgcc_pb [ edge . second . first ] . size () < CUTOFF ) continue; // ignore small ones
      
      // write edge to file
      ofs << "B" << edge . first . first << "N" << edge . first . second;
      ofs << " -> ";
      ofs << "B" << edge . second . first << "N" << edge . second . second;
      ofs << "\n";
    }
    
    //closing brace
    ofs << "}\n";
    ofs . close ();
  }
  
  void list_mgcc ( void ) {
    std::cout << "List of Morse Graph Continuation Class (MGCC) representatives:\n";
    BOOST_FOREACH ( int mgrep, mgcc_rep ) {
  	  std::cout << mgrep << " ";
    }
    std::cout << "\n";
  }

  void list_mscc ( void ) {
    std::cout << "List of Morse Set Continuation Class (MSCC) representatives:\n";
    BOOST_FOREACH ( int msrep, mgcc_rep ) {
  	  std::cout << msrep << " ";
    }
    std::cout << "\n";
  }
  
  void query_mgcc ( int mgcc ) {
    // Data associated with MGCC
    std::cout << "MGCC " << mgcc << " with representative ";
    mgcc = mgcc_uf . Representative ( mgcc );
    std::cout << mgcc << "\n";

    // list of parameter boxes
    std::cout << "   list of (" << mgcc_pb [ mgcc ] . size () << ") parameter boxes: ";
    BOOST_FOREACH ( int pb, mgcc_pb [ mgcc ] ) std::cout << pb << " ";
    std::cout << "\n";

    // Neighboring classes
    std::cout << "   list of (" << mgcc_nb [ mgcc ] . size () << ") neighboring classes: ";
    BOOST_FOREACH ( int nb, mgcc_nb [ mgcc ] ) std::cout << nb << " ";
    std::cout << "\n";
    
    // Morse Set Continuation Classes
    std::cout << "   list of (" << indexed_box_records [ mgcc ] . num_morse_sets_ << ") morse set continuation classes: ";
    for ( int i = 0; i < indexed_box_records [ mgcc ] . num_morse_sets_; ++ i ) {
      intpair mscc ( mgcc, i );
      mscc = mscc_uf . Representative ( mscc );
      std::cout << "(" << mscc . first << ", " << mscc . second << ") ";
    }
    std::cout << "\n";
    
    // Morse Graph
    std::cout << "   morse graph: ";
    BOOST_FOREACH ( Edge edge, mg_edges [ mgcc ] ) {
      intpair source = edge . first;
      intpair target = edge . second;
      std::cout << "(" << source . first << ", " << source . second << ")";
      std::cout << "->";
      std::cout << "(" << target . first << ", " << target . second << ")";
      std::cout << " ";
    }
    std::cout << "\n";
  }

  void query_mscc ( intpair mscc ) {
    // Data associated with MGCC
    std::cout << "MSCC (" << mscc . first << ", " << mscc . second << ") " << " with representative ";
    mscc = mscc_uf . Representative ( mscc );
    std::cout << "(" << mscc . first << ", " << mscc . second << ")\n";
    
    // list of parameter boxes
    std::cout << "   list of (" << mscc_pb [ mscc ] . size () << ") parameter boxes: ";
    BOOST_FOREACH ( int pb, mscc_pb [ mscc ] ) std::cout << pb << " ";
    std::cout << "\n";
    
    // list of morse graph continuation classes
    std::cout << "   list of (" << mscc_mgcc [ mscc ] . size () << ") morse graph continuation classes boxes: ";
    BOOST_FOREACH ( int mgcc, mscc_mgcc [ mscc ] ) std::cout << mgcc << " ";
    std::cout << "\n"; 
    
    // conley index
    
  }
  
  void interact ( void ) {
    list_mgcc ();
    while ( 1 ) {
      std::string input;
      std::cout << "Enter mgcc or mscc\n";
      std::cin >> input;
      if ( input . compare ( "mgcc" ) ) {
				int mgcc;
				std::cout << "Enter a Morse Graph Continuation Class (MGCC) .\n";
				std::cout << "\nparameter box # = ";
				std::cin >> mgcc;
				if ( mgcc < 0 ) break;
        query_mgcc ( mgcc );
      } 
      
      if ( input . compare ( "mscc" ) ) {
				intpair mscc;
				std::cout << "Enter a Morse Set Continuation Class (MSCC).\n";
				std::cout << "\nparameter box # = ";
				std::cin >> mscc . first ;
				std::cout << "\nset # = ";
				std::cin >> mscc . second ;
				std::cout << "\n";
				query_mscc ( mscc );
			}
			
			if ( input . compare ( "exit" ) ) {
				break;
			}
			if ( input . compare ( "quit" ) ) {
				break;
			}
    }  
  }
};




int main ( int argc, char * argv [] ) {
  // Check command line arguments
  if ( argc != 2 ) {
    std::cout << "usage: PostProcessDatabase filename.dat\n";
    exit ( 1 );
  }
  
  // Create Database Object
  Database database;
  
  // Load Database File
  database . load ( argv [ 1 ] );

  // Compute Continuation Classes
  QueryData classdata ( database );
  classdata . save_clutching_graph ( "continuationclutch.gv" );
  
  // Interact
  classdata . interact ();
  
  // Exit Program
  return 0;
}
