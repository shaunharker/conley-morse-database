/*
 *  Strategies.h
 */

#ifndef _CMDP_STRATEGIES_
#define _CMDP_STRATEGIES_

#include <cstddef>


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
  /// and the Morse set to subdivide (0 for none, some large value,
  /// e.g. this_class::MaxValue, for no limit).
  /// TODO: Add more arguments that might help making better decisions.
  bool operator () ( size_t subdiv_level , const typename Toplex::Subset & morse_set ) const {
    return ( ( subdiv_level < max_subdiv_level_ ) && ( morse_set . size () < max_set_size_ ) );
  }

  /// The maximal value to be used for no-limit or beyond-the-limit vaues.
  static const size_t MaxValue = ~ static_cast < size_t > ( 0 );

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
    final_set_ ( final_set ) ,
    max_size_final_set_ ( max_size_final_set ) {
    return;
  }

  /// Makes a decision on whether the Conley index of a given Morse set
  /// should be computed at the moment the Morse set appeared
  /// right after the subdivision of a coarser Morse set.
  /// TODO: Add more arguments that might help making better decisions.
  bool compute_after_subdivision ( size_t subdiv_level ,
    const typename Toplex::Subset & morse_set ) const {
    return ( ( subdiv_level >= after_subdiv_ ) && ( morse_set . size () < max_size_after_subdiv_ ) );
  }

  /// Makes a decision on whether the Conley index of a given Morse set
  /// should be computed at the moment the Morse set becomes a member
  /// of the final Morse decomposition (no more subdivisions for it).
  /// TODO: Add more arguments that might help making better decisions.
  bool compute_final ( const typename Toplex::Subset & morse_set ) const {
    return ( final_set_ && ( morse_set . size () < max_size_final_set_ ) );
  }

  /// The maximal value to be used for no-limit or beyond-the-limit vaues.
  static const size_t MaxValue = ~ static_cast < size_t > ( 0 );

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


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Strategies.hpp"
#endif

#endif
