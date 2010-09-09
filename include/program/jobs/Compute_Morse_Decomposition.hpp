/*
 *  Compute_Morse_Decomposition.hpp
 */

#ifndef _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_
#define _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_

#include "data_structures/Directed_Graph.h" /* For DirectedGraph<Toplex> */

template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > * connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , size_t > * path_bounds ,
  size_t * through_path_bound ,
  const Toplex & phase_space ,
  const typename Toplex::Subset & set_to_decompose ,
  Combinatorial_Map & combinatorial_map ) {

  // --- not yet implemented ---

  // suggested outline:
  // 1) create a subgraph of the combinatorial map
  //    restricted to the given set to decompose
  // 2) mark those vertices in the set to decompose which have edges coming in from outside
  // 3) mark those vertices in the set to decompose which have edges going out to outside
  
  /* Naive construction of a subgraph, entrance_set, and exit set:
     subgraph is copied, entrance_set is assumed to be entire subgraph,
     and exit set is detected accurately*/
  DirectedGraph<Toplex> subgraph;
  typename Toplex::Subset Exit;
  const typename Toplex::Subset & Entrance = set_to_decompose;
  
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, set_to_decompose ) {
    /* intersect should be defined along with Toplex */
    typename Toplex::Subset image = combinatorial_map ( cell );
    BOOST_FOREACH ( typename Toplex::Top_Cell image_cell, image ) {
      if ( set_to_decompose . find ( image_cell ) == set_to_decompose . end () ) {
        Exit . insert ( cell );
      } else {
        subgraph [ cell ] . insert ( image_cell );
      } /* if-else */
    } /* boost_foreach */
  } /* boost_foreach */

  // 4) call Zin's function for computing SCCs
  //    and the strict upper bounds for the path lengths
  
  typename DirectedGraph<Toplex>::Components SCC = computeSCC ( subgraph );
  
  std::vector<size_t> ConnectingPathBounds;
  std::vector<size_t> EntrancePathBounds;
  std::vector<size_t> ExitPathBounds;
  
  computePathBounds( subgraph, SCC, Entrance, Exit, /* inputs */
                     ConnectingPathBounds,
                     EntrancePathBounds,
                     ExitPathBounds,
                     *through_path_bound /* outputs */);
                    
  // 5) copy and adjust the Morse sets returned by Zin's function
  //    to the Conley-Morse graph object
  
  
  
  return;
} /* Compute_Morse_Decomposition */


#endif
