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
  // Union-find structure store Morse Graph Continuation Classes
  UnionFind < int > mgcc_uf; 
  // Map keyed by mgcc representative to give mgcc reps of neighboring classes
  std::map < int, std::set < int > > mgcc_nb; // Morse Graph Continuation Class nbr
  // Union-find structure store Morse Set Continuation Classes
  UnionFind < intpair > mscc_uf;
  // map to store parameter records by their id_ field
  std::map < int, ParameterBoxRecord > indexed_box_records;
  // map to store populations of continuation classes (keyed by id of mgcc_uf representative)
  std::map < int, int> cont_class_pop;
  // map to store morse graph edges (keyed by id of mgcc_uf representative)
  std::map < int, std::set < Edge > > mg_edges;
  // set of clutching edges between mscc_uf representatives
  std::set < Edge > clutch_edges;


  // METHODS
  void QueryData ( const Database & database ) : database ( database ) {
    // Loop through box records. 
    //       Index the Box records, and create a union-find structure for morse sets and boxes
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      mgcc_uf . Add ( record . id_ );
      for ( int i = 0; i < record . num_morse_sets_; ++ i ) {
        mscc_uf . Add ( intpair ( record . id_, i ) );
      }
      indexed_box_records [ record . id_ ] = record;
    }
    
    
    // debug
    BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
      BOOST_FOREACH ( const intpair & edge, record . clutch_ ) {
        if ( indexed_box_records [ record . id1_ ] . num_morse_sets_ <= edge . first ||
            indexed_box_records [ record . id2_ ] . num_morse_sets_ <= edge . second ) {
          std::cout << "Clutching record (" << record . id1_ << ", " << record . id2_ << "):\n";
          std::cout << "  Size of box " << record . id1_ << " is " << indexed_box_records [ record . id1_ ] . num_morse_sets_ << "\n";
          std::cout << "  Size of box " << record . id2_ << " is " << indexed_box_records [ record . id2_ ] . num_morse_sets_ << "\n";
          
          std::cout << "  Edge = (" << edge.first << ", " << edge . second << ")\n";
        }
      }
    }
    
    
    // Loop through clutch records
    //     Determine isomorphisms and perform unions based on results
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
        
    // count populations of morse graph continuation classes
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      int rep = mgcc_uf . Representative ( record . id_ );
      if ( cont_class_pop . find ( rep ) == cont_class_pop . end () ) cont_class_pop [ rep ] = 0;
      // prepare non-empty entries
      if ( mg_edges . count ( rep ) == 0 ) mg_edges [ rep ] = std::set < Edge > ();
      
      ++ cont_class_pop [ rep ];
      // check for violations in assumption
      for ( int i = 0; i < record . num_morse_sets_; ++ i ) {
        intpair rep = mscc_uf . Representative ( intpair ( record . id_ , i ) );
        if ( rep . first == record . id_ && rep . second != i ) {
          std::cout << "Violation in assumptions.\n";
        }
      }
    }
    
    // Generate morse graph edges
    BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
      BOOST_FOREACH ( const intpair & edge, record . partial_order_ ) {
        int rep_id = mgcc_uf . Representative ( record . id_ );
        intpair source = mscc_uf . Representative ( intpair ( record . id_, edge . first ) );
        intpair target = mscc_uf . Representative ( intpair ( record . id_, edge . second ) );
        if ( source == target ) continue;
        mg_edges [ rep_id ] . insert ( Edge ( source, target ) );
      }
    }
    
    // transitive reduction -- n^4 , should be n^3
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
      intpair source = mgcc_uf . Representative ( record . id1_ );
      intpair target = mgcc_uf . Representative ( record . id2_ );
      mgcc_nb [ source ] . insert ( target );
      mgcc_nb [ target ] . insert ( source );
    }
  }
  
  void save_clutching_graph ( const char * filename ) {
    std::ofstream ofs ( filename );
    
    // opening brace
    ofs << "digraph G {\n";
    
    int CUTOFF = 0;
    // morse graphs
    ofs << "node [label=\"\",width=.1,height=.1]\n";
    BOOST_FOREACH ( const mg_edges_t & entry, mg_edges ) {
      if ( cont_class_pop [ entry . first ] < CUTOFF ) continue; // ignore small ones
      if ( mgcc_uf . Representative ( entry . first ) != entry . first ) std::cout << "Assumption violated\n";
      
      ofs << " subgraph cluster" << entry . first << " {\n";
      ofs << " label=\"" << cont_class_pop [ entry . first ] << "\";\n";
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
      if ( cont_class_pop [ edge . first . first ] < CUTOFF ) continue; // ignore small ones
      if ( cont_class_pop [ edge . second . first ] < CUTOFF ) continue; // ignore small ones
      
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
  
  void query_mgcc ( int mgcc ) {
    std::cout << "MGCC " << mgcc << "\n";
    // Number of parameter boxes
    std::cout << "   number of parameter boxes: " << cont_class_pop [ mgcc ] << "\n";
    // Neighboring classes
    std::cout << "   neighboring classes: ";
    BOOST_FOREACH ( int nb, mgcc_nb [ mgcc ] ) {
      std::cout << nb << " ";
    }
    std::cout << "\n";
    // Number of Morse Sets
    std::cout << "   number of morse sets: " << indexed_box_records [ mgcc ] . num_morse_sets_ << "\n";
    // Morse Set Continuation Classes
    std::cout << "   morse sets continuation classes: ";
    for ( int i = 0; i < indexed_box_records [ mgcc ] . num_morse_sets_; ++ i ) {
      intpair mscc ( mgcc, i );
      mscc = mscc_uf . Representative ( mscc );
      std::cout << "(" << mscc . first << ", " << mscc . second << ") ";
    }
    std::cout << "\n";
    // Morse Graph
    
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
  
  // Exit Program
  return 0;
}
