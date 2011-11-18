/*
 *  Clutching_Graph_Job.hpp
 */

#include <boost/foreach.hpp>
#include <algorithm>
#include <stack>
#include <vector>
#include <ctime>
#include <set>
#include "boost/iterator_adaptors.hpp"
#include "boost/iterator/counting_iterator.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/set.hpp"

#include "database/program/Configuration.h"
#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/UnionFind.hpp"
#include "database/structures/Database.h"

/* Prefix Tree Structure Used in Clutching Algorithm */

template < class T >
class PrefixNode {
  PrefixNode * left_;
  PrefixNode * right_;
  PrefixNode * parent_;
  std::vector<T> tag_;
public:
  PrefixNode ( void ) : left_ ( NULL ), right_ ( NULL ), parent_ ( NULL ), tag_ () {}
  PrefixNode ( PrefixNode * parent ) : left_ ( NULL ), right_ ( NULL ), parent_ ( parent ) {}
  ~PrefixNode ( void ) {
    if ( left_ != NULL ) delete left_;
    if ( right_ != NULL ) delete right_;
  }
  PrefixNode * left ( void ) {
    if ( left_ == NULL ) {
      left_ = new PrefixNode < T >;
      left_ -> parent_ = this;
    }
    return left_;
  }
  PrefixNode * right ( void ) {
    if ( right_ == NULL ) {
      right_ = new PrefixNode < T >;
      right_ -> parent_ = this;
    }
    return right_;
  }  
  PrefixNode * parent ( void ) {
    return parent_;
  } 
  std::vector<T> & tag ( void ) {
    return tag_;
  }
};

template < class T >
class PrefixTree {
  PrefixNode <T> * root_;
public:
  typedef std::vector<unsigned char> Prefix;
  typedef PrefixNode<T> * PrefixIterator;
  PrefixTree ( void ) : root_ ( NULL ) {}
  ~PrefixTree ( void ) {
    if ( root_ != NULL ) delete root_;
  }
  PrefixIterator insert ( const Prefix & p, const T & item ) {
    if ( root_ == NULL ) root_ = new PrefixNode < T >;
    PrefixNode <T> * node = root_;
    for ( unsigned int i = 0; i < p . size (); ++ i ) {
      if ( p [ i ] == 0 ) {
        node = node -> left ();
      } else {
        node = node -> right ();
      }
    }
    node -> tag () . push_back ( item );
    return node;
  }
};

/** Compute Clutching of Adjacent Parameter Boxes */

template<class CMGraph, class Toplex>
void Clutching( ClutchingRecord * result,
                const CMGraph &graph1,
                const CMGraph &graph2,
                const Toplex &toplex1,
                const Toplex &toplex2 ) {
  unsigned long effort = 0;
  typedef typename Toplex::Top_Cell Top_Cell;
  typedef typename CMGraph::Vertex Vertex;
  typedef std::vector<unsigned char> Prefix;
  typedef typename CMGraph::CellContainer CellContainer;
  /* Build a Prefix Tree */
  //std::cout << "Building Prefix Tree 1.\n";
  PrefixTree<Vertex> tree;
  std::stack < PrefixNode<Vertex> * > first_cells, second_cells;
  BOOST_FOREACH (Vertex v, graph1.Vertices()) {
    //std::cout << "Size of Cellset = " << graph1 . CellSet ( v ) . size () << "\n";
    
    // CREEPY BUG ON CONLEY2 -- I SUSPECT SUBTLE COMPILER OPTIMIZATION BUG INTERACTING WITH BOOST_FOREACH
#if 0
    BOOST_FOREACH ( Top_Cell t, graph1 . CellSet ( v ) ) { }// here 
#else
    for ( typename CellContainer::const_iterator it1 = graph1 . CellSet ( v ) . begin (), 
         it2 = graph1 . CellSet ( v ) . end (); it1 != it2; ++ it1 ) {
      Top_Cell t = *it1;
#endif
      
      Prefix p = toplex1 . prefix ( t );
      first_cells . push ( tree . insert ( p, v ) );
      ++ effort;
    }
  }
  //std::cout << "Building Prefix Tree 2.\n";
  
  BOOST_FOREACH (Vertex v, graph2.Vertices()) {
    BOOST_FOREACH ( Top_Cell t, graph2 . CellSet ( v ) ) {
      Prefix p = toplex2 . prefix ( t );
      second_cells . push ( tree . insert ( p, v ) );
      ++ effort;
    }
  }  
  /* Climb from each item place in tree and record which pairs we get */
  std::set < std::pair < Vertex, Vertex > > bipartite_graph;
  //std::cout << "Climbing Prefix Tree 1.\n";
  
  while ( not first_cells . empty () ) {
    ++ effort;
    PrefixNode < Vertex > * node = first_cells . top ();
    first_cells . pop ();
    Vertex a, b;
    a = node -> tag () [ 0 ];
    // If there are two items here, then we don't need to climb.
    if ( node -> tag () . size () > 1 ) {
      ++ effort;
      b = node -> tag () [ 1 ];
      bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) );
      continue;
    } 
    // Otherwise, we do need to climb.
    while ( node -> parent () != NULL ) {
      ++ effort;
      node = node -> parent ();
      if ( node -> tag () . size () > 0 ) {
        ++ effort;
        b = node -> tag () [ 0 ];
        bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) );
        //std::cout << "(" << a << ", " << b << ")\n";
        break; // Konstantin pointed this out
      }
    }
  }
  //std::cout << "Climbing Prefix Tree 2.\n";
  
  // Now an exact repeat for the other case
  while ( not second_cells . empty () ) {
    ++ effort;
    PrefixNode < Vertex > * node = second_cells . top ();
    second_cells . pop ();
    Vertex a, b;
    a = node -> tag () [ 0 ];
    // If there are two items here, then we don't need to climb.
    if ( node -> tag () . size () > 1 ) {
      ++ effort;
      b = node -> tag () [ 1 ];
      bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) ); // pointless, actually; already there
      continue;
    } 
    // Otherwise, we do need to climb.
    while ( node -> parent () != NULL ) {
      ++ effort;
      node = node -> parent ();
      if ( node -> tag () . size () > 0 ) {
        ++ effort;
        b = node -> tag () [ 0 ];
        bipartite_graph . insert ( std::pair < Vertex, Vertex > ( b, a ) ); // notice the reversal
        //std::cout << "(" << b << ", " << a << ")\n";
        break; // Konstantin pointed this out
      }
    }
  }
  
  // Report matching information
  BOOST_FOREACH ( const typename CMGraph::Edge & edge, bipartite_graph ) {
    result -> clutch_ . push_back ( edge );
  }
  
}
  
/** Main function for clutching graph job.
 *
 *  This function is called from worker, and compare graph structure
 *  for each two adjacent boxes.
 */
template <class Toplex, class ParameterToplex, class ConleyIndex >
void Clutching_Graph_Job ( Message * result , const Message & job ) {
  typedef ConleyMorseGraph< std::vector < typename Toplex::Top_Cell >, ConleyIndex> CMGraph;
  
  // Read Job Message

  size_t job_number;
  std::vector < typename Toplex::Top_Cell > cell_names;
  std::vector<typename ParameterToplex::Geometric_Description> geometric_descriptions;
  std::vector<std::pair<size_t, size_t> > adjacencies;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Prism PHASE_BOUNDS;
  
  job >> job_number;
  job >> cell_names;
  job >> geometric_descriptions;
  job >> adjacencies;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> PHASE_BOUNDS;
  
  // Prepare data structures
  std::map < typename Toplex::Top_Cell, Toplex> phase_space_toplexes;  
  std::map < typename Toplex::Top_Cell, CMGraph> conley_morse_graphs;
  std::map < typename Toplex::Top_Cell, size_t > cell_index;
  std::vector < ClutchingRecord > clutching_graphs;
  

  // Compute Morse Graphs
  for ( unsigned int i = 0; i < cell_names . size (); ++ i ) {
    //Prepare phase space and map
    typename Toplex::Top_Cell cell = cell_names [ i ];
    cell_index [ cell ] = i;
    phase_space_toplexes [ cell ] . initialize ( PHASE_BOUNDS );
    GeometricMap map ( geometric_descriptions [ i ] );
    // perform computation
    Compute_Morse_Graph 
    ( & conley_morse_graphs  [ cell ],
      & phase_space_toplexes [ cell ], 
      map, 
      PHASE_SUBDIV_MIN, 
      PHASE_SUBDIV_MAX, 
      PHASE_SUBDIV_LIMIT);
  }
  
  // Compute Clutching Graphs
  typedef std::pair < size_t, size_t > Adjacency;
  BOOST_FOREACH ( const Adjacency & A, adjacencies ) {
    clutching_graphs . push_back ( ClutchingRecord () );
    Clutching < CMGraph, Toplex > ( & clutching_graphs . back (),
                                      conley_morse_graphs [ A . first ],
                                      conley_morse_graphs [ A . second ],
                                      phase_space_toplexes [ A . first ],
                                      phase_space_toplexes [ A . second ] );
    clutching_graphs . back () . id1_ = A . first;
    clutching_graphs . back () . id2_ = A . second;
  }
  
  // Create Database
  Database database;
  
  // Create Parameter Box Records
  typedef std::pair < typename Toplex::Top_Cell, CMGraph > indexed_cmg_t;
  BOOST_FOREACH ( const indexed_cmg_t & cmg, conley_morse_graphs ) {
    database . insert ( ParameterBoxRecord (cmg . first, 
                                            geometric_descriptions [ cell_index [ cmg.first ] ],   
                                            cmg . second ) );
  }
  // Create Clutching Records
  BOOST_FOREACH ( const ClutchingRecord & clutch, clutching_graphs ) {
    database . insert ( clutch );
  }
  
  // Return Result
  *result << job_number;
  *result << database;
}
