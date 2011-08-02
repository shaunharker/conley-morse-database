/* Compute_Morse_Graph.hpp */
#ifndef _CMDP_COMPUTE_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_MORSE_GRAPH_HPP_

#include <vector>
#include "boost/foreach.hpp"
#include "data_structures/GraphTheory.h"

#ifdef ILLUSTRATE
#include "algorithms/Homology.h"
#endif
template < class Toplex, class CellContainer > 
void subdivide ( Toplex & phase_space, CellContainer & morse_set );

template < class Morse_Graph, class Toplex, class Map >
void Compute_Morse_Graph (Morse_Graph * MG, Toplex * phase_space, const Map & interval_map,
                          const unsigned int Min, const unsigned int Max, const unsigned int Comp) {
  typedef std::vector<typename Toplex::Top_Cell> CellContainer;
  typedef CombinatorialMap<Toplex,CellContainer> Graph;
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
    Graph G = compute_combinatorial_map ( morse_sets, * phase_space, interval_map );
    compute_morse_sets <Morse_Graph,Toplex,CellContainer> ( &morse_sets, G );
    BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) subdivide ( phase_space, morse_set );
  }
  // Perform reachability analysis
#ifdef CMG_VERBOSE
  std::cout << "Reachability Analysis.\n";
#endif
  Graph G = compute_combinatorial_map<Toplex,Map,CellContainer> ( * phase_space , interval_map );
  compute_morse_sets<Morse_Graph,Toplex,CellContainer> ( &morse_sets, G, MG );
  
  // Eliminate Spurious Sets
#ifdef CMG_VERBOSE
  std::cout << "Eliminate Spurious Sets.\n";
#endif
  int mg_node = 0;
  std::set < int > subgraph_nodes;
  BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) { 
    std::vector < CellContainer > spurious_sets;
    spurious_sets . push_back ( morse_set );
    if ( morse_set . size () < Comp )
    for ( unsigned int d = Min; d < Max && not spurious_sets . empty (); ++ d ) {
#ifdef CMG_VERBOSE
      std::cout << "Depth " << d << ": \n";
#endif
      Graph G = compute_combinatorial_map ( spurious_sets, * phase_space, interval_map );
      compute_morse_sets <Morse_Graph,Toplex,CellContainer> ( &spurious_sets, G );
      BOOST_FOREACH ( CellContainer & spur, spurious_sets ) {
        if ( spur . size () > Comp ) { 
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
      subgraph_nodes . insert ( mg_node );
      // TODO REMOVE THIS
#ifdef ILLUSTRATE
      Conley_Index_t output;
      Conley_Index ( &output,
                     *phase_space, 
                     morse_set,
                     interval_map );
#endif
    } else {
#ifdef CMG_VERBOSE
      std::cout << "Revealed as spurious.\n";
#endif
    }
    ++ mg_node;
  }
  Morse_Graph submg = MG -> Subgraph ( subgraph_nodes );
  std::swap ( *MG, submg );
}

template < class Toplex, class CellContainer >
void subdivide ( Toplex & phase_space, CellContainer & morse_set ) {
  CellContainer new_morse_set;
  std::insert_iterator<CellContainer> ii ( new_morse_set, new_morse_set . begin () );
  phase_space -> subdivide (ii, morse_set);
  std::swap ( new_morse_set, morse_set );
}
#endif
