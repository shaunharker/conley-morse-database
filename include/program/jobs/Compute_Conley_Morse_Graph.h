/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_

#include "algorithms/Homology.h"


/// A class for function objects that make decisions on whether
/// one should subdivide a Morse set further or not,
/// based on the subdivision level and the size of the set.
/// The decision is based upon a predefined
/// strict upper bound on the subdivision level
/// and an apriori given strict upper bound
/// on the allowed size of Morse sets to subdivide.
template < class Toplex >
class Decide_Subdiv_Level_Size
{
public:
  /// The constructor.
  Decide_Subdiv_Level_Size ( size_t max_subdiv_level , typename Toplex::size_type max_set_size ) :
    max_subdiv_level_ ( max_subdiv_level ) , max_set_size_ ( max_set_size ) { }

  /// The decision making operator.
  /// It receives the following arguments:
  /// the current subdivision level (0 stands for the entire phase space
  /// represented as a single Morse set),
  /// the Morse set to subdivide,
  /// and more arguments that will be added in the future.
  bool operator () ( size_t subdiv_level , const typename Toplex::Toplex_Subset & morse_set ) const {
    return ( ( subdiv_level < max_subdiv_level_ ) && ( morse_set . size () < max_set_size_ ) );
  }

private:
  /// The strict upper bound for subdivision levels allowed.
  size_t max_subdiv_level_;

  /// The strict upper bound for the size of a Morse set
  /// which one is allowed to subdivide.
  typename Toplex::size_type max_set_size_;

}; /* class Decide_Subdiv_Level_Size */

// --------------------------------------------------

/// A class for function objects that make a decision on whether
/// the Conley index of a given Morse set should be computed,
/// based on the size of this set.
/// Different size limits can be set up for Morse sets computed
/// at intermediate steps and for those that become members
/// of the final Morse decomposition.
template < class Toplex >
class Decide_Conley_Index_Size
{
public:
  /// The constructor.
  /// Please, see the description of the private variables for the meaning
  /// of all the arguments and how to set them to desire specific effects.
  Decide_Conley_Index_Size ( size_t after_subdiv , size_t max_size_after_subdiv ,
    bool final_set , size_t max_size_final_set ) :
    after_subdiv_ ( after_subdiv ) ,
    max_size_after_subdiv_ ( max_size_after_subdiv ) ,
    max_size_final_set_ ( max_size_final_set ) ,
    final_set_ ( final_set ) { }

  /// Makes a decision on whether the Conley index of a given Morse set
  /// should be computed at the moment the Morse set appeared
  /// right after the subdivision of a coarser Morse set.
  inline bool compute_after_subdivision ( size_t subdiv_level ,
    const typename Toplex::Toplex_Subset & morse_set ) {
    return ( ( subdiv_level >= after_subdiv_ ) && ( morse_set . size () < max_size_after_subdiv_ ) );
  }

  /// Makes a decision on whether the Conley index of a given Morse set
  /// should be computed at the moment the Morse set becomes a member
  /// of the final Morse decomposition (no more subdivisions for it).
  inline bool compute_final ( const typename Toplex::Toplex_Subset & morse_set ) {
    return ( final_set_ && ( morse_set . size () < max_size_final_set_ ) );
  }

  /// The maximal value to be used for no-limit or beyond-the-limit vaues.
  const size_t MaxValue = ~0;

private:
  /// The first subdivision level after which the Conley index
  /// should be computed. Set to 0 to compute at every subdivision level.
  /// Set to a large number (e.g. this_class::MaxValue) to suppress
  /// Conley index computation at the intermediate stages.
  size_t after_subdiv_;

  /// The strict upper bound for the size of a Morse set that appears
  /// at the intermediate stage, for which the Conley index computeation
  /// will be taken into consideration.
  /// Set to 0 to suppress Conley index computation.
  /// Set to a huge value (e.g. this_class::MaxValue) for no limit.
  size_t max_size_after_subdiv_;

  /// Should the Conley indices be computed for Morse sets that become members
  /// of the final Morse decomposition? Note that if any of the indices
  /// was computed already at the intermediate stage then it is not re-computed.
  bool final_set_;

  /// The strict upper bound for the size of a Morse set that becomes a member
  /// of the final Morse decomposition, for which the Conley index computeation
  /// will be taken into consideration.
  /// Set to 0 to suppress Conley index computation.
  /// Set to a huge value (e.g. this_class::MaxValue) for no limit.
  size_t max_size_final_set_;

}; /* class Decide_Conley_Index_Size */

// --------------------------------------------------

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
template < class Toplex , class Parameter_Toplex , class Map ,
  class Decide_Subdiv , class Decide_Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index_t > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  const Decide_Subdiv & decide_subdiv ,
  const Decide_Conley_Index & decide_conley_index ,
  Cached_Box_Informatin * cached_box_information );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Compute_Conley_Morse_Graph.hpp"
#endif

#endif
