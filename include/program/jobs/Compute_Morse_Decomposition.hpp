/*
 *  Compute_Morse_Decomposition.hpp
 */

#ifndef _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_
#define _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_


template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , long > * path_bounds ,
  long * through_path_bound ,
  const typename Toplex::Toplex_Subset & domain ,
  const Combinatorial_Map & combinatorial_map ) {
  // --- not yet implemented ---
  // suggested outline:
  // 1) create a subgraph of the combinatorial map
  //    restricted to the given domain
  // 2) mark those points in the domain which have edges coming in from outside
  // 3) mark those points in the domain which have edges going out to outside
  // 4) call Zin's function for computing SCCs
  //    and the strict upper bounds for the path lengths
  // 5) copy and adjust the data returned by Zin's function
  //    to the Conley-Morse graph object
  return;
} /* Compute_Morse_Decomposition */


#endif
