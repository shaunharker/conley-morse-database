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
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > * connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , size_t > * path_bounds ,
  long * through_path_bound ,
  const typename Toplex::Toplex_Subset & domain ,
  const Combinatorial_Map & combinatorial_map ) {

  // --- not yet implemented ---

  // suggested outline:
  // 1) create a subgraph of the combinatorial map
  //    restricted to the given domain
  // 2) mark those vertices in the domain which have edges coming in from outside
  // 3) mark those vertices in the domain which have edges going out to outside
  
  /* Naive construction of a subgraph, entrance_set, and exit set:
     subgraph is copied, entrance_set is assumed to be entire subgraph,
     and exit set is detected accurately*/
  DirectedGraph<Toplex> subgraph;
  typename Toplex::Subset Exit;
  typename Toplex::Subset & Entrance = domain;
  
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, domain ) {
    /* intersect should be defined along with Toplex */
    Toplex::Subset image = combinatorial_map ( cell );
    BOOST_FOREACH ( typename Toplex::Top_Cell image_cell, image ) {
      if ( domain . find ( image_cell ) == domain . end () ) {
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
