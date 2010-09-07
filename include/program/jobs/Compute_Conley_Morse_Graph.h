/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_

/// A class for function objects that make decisions on whether
/// one should subdivide Morse sets further or not.
/// In this sample, the decision is based upon a predefined
/// strict upper bound on the subdivision level
/// and an apriori given strict upper bound
/// on the allowed size of Morse sets to subdivide.
template < class Toplex >
class Subdiv_Decide_Level_Size
{
public:
  /// The constructor.
  Subdiv_Decide_Level_Size ( size_t max_subdiv_level, typename Toplex::size_type max_set_size ):
    max_subdiv_level_ ( max_subdiv_level ), max_set_size_ ( max_set_size ) { }

  /// The decision making operator.
  /// It receives the following arguments:
  /// the current subdivision level (0 stands for the entire phase space
  /// represented as a single Morse set),
  /// the Morse set to subdivide,
  /// and more arguments that will be added in the future.
  bool operator () ( size_t subdiv_level, const typename Toplex::Toplex_Subset & morse_set ) const {
    return ( ( subdiv_level < max_subdiv_level_ ) && ( morse_set . size () < max_set_size_ ) );
  }

private:
  /// The strict upper bound for subdivision levels allowed.
  size_t max_subdiv_level_;

  /// The strict upper bound for the size of a Morse set
  /// which one is allowed to subdivide.
  typename Toplex::size_type max_set_size_;

}; /* class Subdiv_Decide_Level_Size */


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.
/// Each Morse set is dubdivided depending on what the subdiv_decide
/// function object says (see sample Subdiv_Decide class for the actual
/// specification on how this should work).
/// The computed Conley-Morse graph is stored in the data structure provided
/// (which must be initially empty), together with Morse sets
/// and Conley indices, for which memory is dynamically allocated
/// (remember to release this memory later on to avoid memory leaks).
template < class Toplex , class Parameter_Toplex ,
  class Map , class Conley_Index , class Subdiv_Decide , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  const Subdiv_Decide & subdiv_decide ,
  Cached_Box_Informatin * cached_box_information );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph.hpp"
#endif

#endif
