/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.
/// Each Morse set is dubdivided depending on what the decide_subdiv
/// function object says (see the Decide_Subdiv_Level_Size class for the actual
/// specification on how this should work).
/// The decision on whether the Conley indices should be computed
/// is made by the decide_conley_index function object
/// (see the Decide_Conley_Index_Size class for an idea on how this works).
/// If a cached box information object is provided (the pointer is non-null)
/// then the cached information is used or - if empty - it is computed.
/// The computed Conley-Morse graph is stored in the data structure provided
/// (which must be initially empty), together with Morse sets
/// and Conley indices, for which memory is dynamically allocated
/// (remember to release this memory later on to avoid memory leaks).
template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map ,
  class Decide_Subdiv , class Decide_Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( Conley_Morse_Graph * conley_morse_graph ,
  Toplex * phase_space ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  const Decide_Subdiv & decide_subdiv ,
  const Decide_Conley_Index & decide_conley_index ,
  Cached_Box_Information * cached_box_information );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph.hpp"
#endif

#endif
