/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.
template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map >
void Compute_Conley_Morse_Graph ( Conley_Morse_Graph * conley_morse_graph ,
  Toplex * phase_space,
  const typename Parameter_Toplex::Geometric_Description & parameter_box,
  bool should_compute_reachability = false,
  bool should_compute_conley_index = false);


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph.hpp"
#endif

#endif
