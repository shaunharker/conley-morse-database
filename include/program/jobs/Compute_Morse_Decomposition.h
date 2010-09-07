/*
 *  Compute_Morse_Decomposition.h
 */

#ifndef _CMDP_COMPUTE_MORSE_DECOMPOSITION_
#define _CMDP_COMPUTE_MORSE_DECOMPOSITION_

/// Computes the Morse decomposition of a given set
/// using the SCC algorithm. The Conley Morse graph object
/// given in the input must be initially empty, and is filled in
/// the associated data structures with all the necessary information:
/// the maximal lengths of paths that begin at the Morse sets and end
/// in any of the vertices which have an outgoing edge from the domain;
/// the maximal lengths of paths that enter the Morse sets and begin
/// at any vertex that has an icoming edge from outside the domain;
/// the maximal lengtsh of paths connecting pairs of Morse sets;
/// and the maximal length of a path that begins at any vertex
/// with an edge incoming from outside the domain and ends at any vertex
/// that has an edge outgoing from the domain.
/// The provided vectors must be initially empty.
template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , long > * path_bounds ,
  long * through_path_bound ,
  const typename Toplex::Toplex_Subset & domain ,
  const Combinatorial_Map & combinatorial_map );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Morse_Decomposition.hpp"
#endif

#endif
