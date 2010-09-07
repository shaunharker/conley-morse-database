/*
 *  Configuration.h
 */


#ifndef _CMDP_CONFIGURATION_
#define _CMDP_CONFIGURATION_


/// Makes a decision on whether the Conley index of a given Morse set
/// should be computed at the moment the Morse set appeared
/// right after the subdivision of a coarser Morse set.
template < class Toplex >
inline bool compute_Conley_index_after_subdivision ( size_t subdiv_level ,
  const typename Toplex::Toplex_Subset & morse_set ) {

  return true;
} /* compute_Conley_index_after_subdivision */

/// Makes a decision on whether the Conley index of a given Morse set
/// should be computed at the moment the Morse set becomes a member
/// of the final Morse decomposition (no more subdivisions for it).
template < class Toplex >
inline bool compute_Conley_index ( const typename Toplex::Toplex_Subset & morse_set ) {
  return true;
} /* compute_Conley_index */


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Configuration.hpp"
#endif

#endif
