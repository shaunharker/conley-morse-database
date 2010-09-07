/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.
/// The toplex representation of the phase space is subdivided
/// at most the given number of times.
/// The computed Conley-Morse graph is stored in the data structure provided
/// (which must be initially empty), together with Morse sets
/// and Conley indices, for which memory is dynamically allocated
/// (remember to release this memory later on to avoid memory leaks).
template < class Toplex , class Parameter_Toplex ,
  class Map , class Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  int subdivisions ,
  Cached_Box_Informatin * cached_box_information );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph.hpp"
#endif

#endif
