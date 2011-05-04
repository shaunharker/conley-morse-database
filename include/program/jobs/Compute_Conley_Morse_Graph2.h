/*
 *  Compute_Conley_Morse_Graph2.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.
template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map >
void Compute_Conley_Morse_Graph2 ( Conley_Morse_Graph * conley_morse_graph ,
  Toplex * phase_space ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box);


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph2.hpp"
#endif

#endif
