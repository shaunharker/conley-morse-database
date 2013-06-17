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
#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/UnionFind.hpp"
#include "database/structures/Database.h"
#include "database/structures/Tree.h"

#include "chomp/Rect.h"

void Clutching( BG_Data * result,
               const MorseGraph & graph1,
               const MorseGraph & graph2 ) {

  typedef MorseGraph::Vertex Vertex;
  std::set < std::pair < Vertex, Vertex > > bipartite_graph;

  size_t N1 = graph1 . NumVertices ();
  size_t N2 = graph2 . NumVertices ();
  
  typedef Tree::iterator iterator;

  // How this works:
  // We want to advance through the trees simultaneously, but they aren't all the same tree
  // If we explore a subtree in some trees that does not exist in others, we remain halted on the others
  // until the subtree finishes.
  //
  // initialize iterators
  
  // State machine:
  // State 0: Try to go left. If can't, set success to false and try to go right. Otherwise success is true and try to go left on next iteration.
  // State 1: Try to go right. If can't, set success to false and rise. Otherwise success is true and try to go left on next iteration.
  // State 2: Rise. If rising from the right, rise again on next iteration. Otherwise try to go right on the next iteration.
  std::vector < iterator > iters1 ( N1 );
  std::vector < iterator > iters2 ( N2 );
  for ( size_t i = 0; i < N1; ++ i ) iters1[i] = graph1 . grid ( i ) -> tree () . begin ();
  for ( size_t i = 0; i < N2; ++ i ) iters2[i] = graph2 . grid ( i ) -> tree () . begin ();
  std::vector < size_t > depth1 ( N1, 0 );
  std::vector < size_t > depth2 ( N2, 0 );
  size_t depth = 0;
  int state = 0;
  
  //std::cout << "Clutching Function.\n";
  //std::cout << "N1 = " << N1 << " and N2 = " << N2 << "\n";
  while ( 1 ) {
    if ( (depth == 0) && ( state == 2 ) ) break;
    //std::cout << "Position 0. depth = " << depth << " and state = " << state << "\n";
    bool success = false;
    Vertex set1 = N1;
    Vertex set2 = N2;
    for ( size_t i = 0; i < N1; ++ i ) {
      //std::cout << "Position 1. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
      // If node is halted, continue
      if ( depth1[i] == depth ) {
        iterator end = graph1 . grid ( i ) -> tree () . end ();
        switch ( state ) {
          case 0:
          {
            iterator left = graph1 . grid ( i ) -> tree () . left ( iters1 [ i ] );
            if ( left == end ) break;
            iters1[i] = left;
            ++ depth1[i];
            success = true;
            break;
          }
          case 1:
          {
            iterator right = graph1 . grid ( i ) -> tree () . right ( iters1 [ i ] );
            if ( right == end ) break;
            iters1[i] = right;
            ++ depth1[i];
            success = true;
            break;
          }
          case 2:
          {
            if ( graph1 . grid ( i ) -> tree () . isright ( iters1 [ i ] ) ) success = true;
            iters1[i] = graph1 . grid ( i ) -> tree () . parent ( iters1 [ i ] );
            -- depth1[i];
            break;
          }
        }
      }
      if ( graph1 . grid ( i ) -> tree () . isleaf ( iters1 [ i ] ) ) {
        if ( set1 != (Vertex)N1 ) std::cout << "Warning, morse sets are not disjoint.\n";
        set1 = (Vertex)i;
      }
      
    }
    for ( size_t i = 0; i < N2; ++ i ) {
      //std::cout << "Position 2. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
      // If node is halted, continue
      if ( depth2[i] == depth ) {
        iterator end = graph2 . grid ( i ) -> tree () . end ();
        switch ( state ) {
          case 0:
          {
            iterator left = graph2 . grid ( i ) -> tree () . left ( iters2 [ i ] );
            if ( left == end ) break;
            iters2[i] = left;
            ++ depth2[i];
            success = true;
            break;
          }
          case 1:
          {
            iterator right = graph2 . grid ( i ) -> tree () . right ( iters2 [ i ] );
            if ( right == end ) break;
            iters2[i] = right;
            ++ depth2[i];
            success = true;
            break;
          }
          case 2:
          {
            if ( graph2 . grid ( i ) -> tree () . isright ( iters2 [ i ] ) ) success = true;
            iters2[i] = graph2 . grid ( i ) -> tree () . parent ( iters2 [ i ] );
            -- depth2[i];
            break;
          }
        }
      }
      if ( graph2 . grid ( i ) -> tree () . isleaf ( iters2 [ i ] ) ) {
        if ( set2 != (Vertex)N2 ) std::cout << "Warning, morse sets are not disjoint.\n";
        set2 = (Vertex)i;
      }
    }
    //std::cout << "Position 3. depth = " << depth << " and state = " << state << "\n";
    //std::cout << ( success ? "success" : "failure" );
    switch ( state ) {
      case 0: // Tried to go left
        if ( success ) {
          // Success. Try to go left again.
          state = 0;
          ++ depth;
        } else {
          // Failure. Try to go right instead.
          state = 1;
        }
        break;
      case 1: // Tried to go right
        if ( success ) {
          // Sucess. Try to go left now.
          state = 0;
          ++ depth;
        } else {
          // Failure. Rise.
          state = 2;
        }
        break;
      case 2: // Rose
        -- depth;
        if ( success ) {
          // Rose from right, continue to rise
          state = 2;
        } else {
          // Rose from left, try to go right
          state = 1;
        }
        break;
    }
    // Gather all intersection information
    if ( (set1 != (Vertex)N1 ) && (set2 != (Vertex)N2 ) ) {
      //std::cout << "Record intersection (" << set1 << ", " << set2 << ")\n";

      bipartite_graph . insert ( std::pair < Vertex, Vertex > ( set1, set2 ) );
    }
  }
  
  // Advance iterators to end, collecting intersections
  // Return result
  //std::cout << "Collate Results.\n";
  BOOST_FOREACH ( const MorseGraph::Edge & edge, bipartite_graph ) {
    result -> edges . push_back ( edge );
  }
}

/** Main function for clutching graph job.
 *
 *  This function is called from worker, and compare graph structure
 *  for each two adjacent boxes.
 */
template <class PhaseGrid>
void Clutching_Graph_Job ( Message * result , const Message & job ) {
  
  // Read Job Message

  size_t job_number;
  std::vector <size_t> box_names;
  std::vector < chomp::Rect > box_geometries;
  std::vector <std::pair<size_t, size_t> > box_adjacencies;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  chomp::Rect PHASE_BOUNDS;
  std::vector < bool > PERIODIC;
  job >> job_number;
  job >> box_names;
  job >> box_geometries;
  job >> box_adjacencies;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> PHASE_BOUNDS;
  job >> PERIODIC;
  
  // Prepare data structures
  std::map < size_t, boost::shared_ptr<PhaseGrid> > phase_space_grids;
  std::map < size_t, MorseGraph> conley_morse_graphs;
  std::map < size_t, size_t > box_index;
  std::vector < BG_Data > clutching_graphs;

  // Compute Morse Graphs
  //std::cout << "CLUTCHING JOB " << job_number << ": " << box_names . size () << " BEGINNING.\n";
  //std::cout << "--------- 1. Compute Morse Graphs ---------\n";
  for ( unsigned int i = 0; i < box_names . size (); ++ i ) {
    //std::cout << " Processing parameter box " << i << "/" << box_names . size () << "\n";
    //Prepare phase space and map
    size_t box = box_names [ i ];
    box_index [ box ] = i;
    phase_space_grids [ box ] . reset ( new PhaseGrid );
    phase_space_grids [ box ] -> initialize ( PHASE_BOUNDS, PERIODIC );
    GeometricMap map ( box_geometries [ i ] );
    // perform computation
    Compute_Morse_Graph 
    ( & conley_morse_graphs  [ box ],
      phase_space_grids [ box ], 
      map, 
      PHASE_SUBDIV_MIN, 
      PHASE_SUBDIV_MAX, 
      PHASE_SUBDIV_LIMIT);
    //std::cout << "Morse Graph computed.\n";
    //if ( conley_morse_graphs [ box ] . NumVertices () == 0 ) std::cout << "WARNING. Box # " << box << " Clutching Job #" << job_number << ", box = " << box_geometries [ i ] << " had no morse sets.\n"; else std::cout << "SUCCESS. Box # " << box << " Clutching Job #" << job_number << ", box = " << box_geometries [ i ] << " had " << conley_morse_graphs [ box ] . NumVertices () << " morse sets.\n";
  }
  
  // Compute Clutching Graphs
  //std::cout << "--------- 2. Compute Clutching Graphs ---------\n";
  typedef std::pair < size_t, size_t > Adjacency;
  BOOST_FOREACH ( const Adjacency & A, box_adjacencies ) {
    clutching_graphs . push_back ( BG_Data () );
    Clutching ( & clutching_graphs . back (),
                conley_morse_graphs [ A . first ],
                conley_morse_graphs [ A . second ]);
  }
  
  // Create Database
  //std::cout << "--------- 3. Creating Database ---------\n";

  Database database;
  
  // Create Parameter Box Records
  typedef std::pair < size_t, MorseGraph > indexed_cmg_t;
  BOOST_FOREACH ( const indexed_cmg_t & cmg, conley_morse_graphs ) {
    database . insert ( cmg . first, DAG_Data ( cmg . second ) );
  }
  // Create Clutching Records
  for ( uint64_t i = 0; i < box_adjacencies . size (); ++ i ) {
    database . insert ( box_adjacencies [ i ] . first, box_adjacencies [ i ] . second, clutching_graphs [ i ] );
  }
  
  // Return Result
  //std::cout << "CLUTCHING JOB " << job_number << ": " << box_names . size () << " COMPLETE.\n";

  *result << job_number;
  *result << database;
}
