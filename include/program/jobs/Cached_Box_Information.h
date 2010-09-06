/*
 *  Cached_Box_Information.h
 */

#ifndef _CMDP_CACHED_BOX_INFORMATION_
#define _CMDP_CACHED_BOX_INFORMATION_

#include <boost/serialization/serialization.hpp>


/// Class whose objects store cached information for single box computation.
/// This information is gathered while constructing the Morse decomposition,
/// and is reused while doing the same computations again.
/// In general, this information may include the Conley indices
/// computed for various Morse sets, the existence or non-existence
/// of connections between Morse sets, or even the Morse sets themselves.
class Cached_Box_Information
{
public:

private:
  /// Some dummy information, unused for the moment.
  int info;

  friend class boost::serialization::access;

  /// The serialization method.
  template < class Archive >
  void serialize ( Archive & ar , const unsigned int version ) {
    ar & info;
  }
}; /* class Cached_Box_Information */


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Cached_Box_Information.hpp"
#endif

#endif
