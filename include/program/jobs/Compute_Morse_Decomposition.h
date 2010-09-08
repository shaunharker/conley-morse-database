/*
 *  Compute_Morse_Decomposition.h
 */

#ifndef _CMDP_COMPUTE_MORSE_DECOMPOSITION_
#define _CMDP_COMPUTE_MORSE_DECOMPOSITION_


#include <map>


/// Computes the Morse decomposition of a given set using the SCC algorithm.
/// The Conley Morse graph object given in the input must be empty.
/// For each Morse set, computes the least strict upper bound for the length
/// of a path that begins in the Morse set and end at any of the vertices
/// which have an outgoing edge from the domain ("exit path").
/// For each Morse set, computes the least strict upper bound for the length
/// of a path that ends in the Morse set and begins at any vertex that has
/// an icoming edge from outside the domain ("entrance path").
/// For each connection between two Morse sets, computes the maximal length
/// of a path connecting them.
/// It also computes the least strict upper bound for the length of a path
/// that begins at any vertex with an edge incoming from outside the domain
/// and ends at any vertex that has an edge outgoing from the domain.
/// The provided vectors must be initially empty.
/// Note: The length 0 means "no path." The length 1 means "the shortest
/// path possible."
template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > * connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , size_t > * path_bounds ,
  size_t * through_path_bound ,
  const typename Toplex::Toplex_Subset & domain ,
  const Combinatorial_Map & combinatorial_map );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Morse_Decomposition.hpp"
#endif

#endif
