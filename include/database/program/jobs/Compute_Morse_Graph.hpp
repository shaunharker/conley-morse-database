/* Compute_Morse_Graph.hpp */
#ifndef _CMDP_COMPUTE_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_MORSE_GRAPH_HPP_

#include <vector>
#include "boost/foreach.hpp"
#include "database/algorithms/GraphTheory.h"
#include "database/structures/MapGraph.h"

#ifdef DO_CONLEY_INDEX
#include "chomp/ConleyIndex.h"
#endif
template < class Toplex, class CellContainer > 
void subdivide ( Toplex & phase_space, CellContainer & morse_set );

template < class Morse_Graph, class Toplex, class Map >
void Compute_Morse_Graph (Morse_Graph * MG, Toplex * phase_space, const Map & interval_map,
                          const unsigned int Min, const unsigned int Max, const unsigned int Limit) {
  typedef std::vector<typename Toplex::Top_Cell> CellContainer;
  typedef MapGraph<Toplex,Map,CellContainer> Graph;  
  std::vector < CellContainer > morse_sets;
  // Produce initial Morse Set
  morse_sets . push_back ( CellContainer () );
  std::insert_iterator < CellContainer > ii ( morse_sets . back (), morse_sets . back () . begin () );
  phase_space -> cover ( ii, phase_space -> bounds () );
  
  // Do Subdivisions
#ifdef CMG_VERBOSE
  std::cout << "Subdivisions.\n";
#endif
  for ( unsigned int depth = 0; depth < Min; ++ depth ) {  
#ifdef CMG_VERBOSE
    std::cout << "Depth " << depth << ": \n";
#endif
    Graph G ( morse_sets, * phase_space, interval_map );
    compute_morse_sets <Morse_Graph,Graph,CellContainer> ( &morse_sets, G );
    BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) subdivide ( phase_space, morse_set );
  }
  
  
  
  // Eliminate Spurious Sets
  {
#ifdef CMG_VERBOSE
    std::cout << "Eliminate Spurious Sets.\n";
#endif
    // TODO: think about eliminating this step (make sure you can!)
    // (Because you are forcing this analysis twice.)
    // The problem is I might generate a couple spurious sets at reach stage if I do!
    // Resolution: get a finer interface so I can ask for reachability without redoing morse
    // theory. For now: ignore problem.
    Graph G ( morse_sets, * phase_space, interval_map );
    compute_morse_sets <Morse_Graph,Graph,CellContainer> ( &morse_sets, G );
    
    BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) { 
      std::vector < CellContainer > spurious_sets;
      spurious_sets . push_back ( morse_set );
      if ( morse_set . size () < Limit )
        for ( unsigned int d = Min; d < Max && not spurious_sets . empty (); ++ d ) {
#ifdef CMG_VERBOSE
          std::cout << "Depth " << d << ": \n";
#endif
          Graph G ( spurious_sets, * phase_space, interval_map );
          compute_morse_sets <Morse_Graph,Graph,CellContainer> ( &spurious_sets, G );
          
          BOOST_FOREACH ( CellContainer & spur, spurious_sets ) {
            if ( spur . size () > Limit ) { 
              d = Max;
              break;
            }
            subdivide ( phase_space, spur );
          }
        }
      if ( not spurious_sets . empty () ) {
#ifdef CMG_VERBOSE
        std::cout << "Not revealed to be spurious.\n";
#endif
        phase_space -> coarsen ( morse_set );
#ifdef DO_CONLEY_INDEX
        ConleyIndex_t output;
        ConleyIndex ( &output,
                     *phase_space, 
                     morse_set,
                     interval_map );
#endif
      } else {
#ifdef CMG_VERBOSE
        std::cout << "Revealed as spurious.\n";
#endif
      }
    }
  }
  
  // Perform reachability analysis
#ifdef CMG_VERBOSE
  std::cout << "Reachability Analysis.\n";
#endif
  
  {
    Graph G ( * phase_space, interval_map );
    compute_morse_sets<Morse_Graph,Graph,CellContainer> ( &morse_sets, G, MG );
  }
}

template < class Toplex, class CellContainer >
void subdivide ( Toplex & phase_space, CellContainer & morse_set ) {
  CellContainer new_morse_set;
  std::insert_iterator<CellContainer> ii ( new_morse_set, new_morse_set . begin () );
  phase_space -> subdivide (ii, morse_set);
  std::swap ( new_morse_set, morse_set );
}
#endif
