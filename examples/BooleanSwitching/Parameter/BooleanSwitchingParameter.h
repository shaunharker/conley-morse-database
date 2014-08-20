/// BooleanSwitchingParameter
/// Author: Shaun Harker
/// Date: August 4, 2014
#ifndef BOOLEANSWITCHINGPARAMETER
#define BOOLEANSWITCHINGPARAMETER

#include <iostream>
#include <vector>
#include "boost/functional/hash.hpp"

/// class BooleanSwitchingParameter
///    A smart vertex class representing parameters of interest
///    for a Boolean Switching network, ignoring closest axis information
class BooleanSwitchingParameter : public Parameter {
public:
  std::vector<int> monotonic_function_; // given by preorder

  /// virtual deconstructor
  ~BooleanSwitchingParameter ( void ) {}
  /// constructor
  BooleanSwitchingParameter ( void ) {}
  BooleanSwitchingParameter ( int dimension ) {
    monotonic_function_ . resize ( dimension, 0 );
  }

  /// operator ==
  ///   check equality
  bool operator == ( const BooleanSwitchingParameter & rhs ) const {
    if ( rhs . monotonic_function_ . size () != monotonic_function_ . size () ) {
      return false;
    }
    for ( int d = 0; d < monotonic_function_ . size (); ++ d ) {
      if ( monotonic_function_ [ d ] != rhs . monotonic_function_ [ d ] ) return false;
    }
    return true;
  }

  /// hash_value
  ///   hash_value for class BooleanSwitchingParameter
  friend std::size_t hash_value ( const BooleanSwitchingParameter & p ) {
    std::size_t seed = 0;
    for ( int d = 0; d < p . monotonic_function_ . size (); ++ d ) {
      boost::hash_combine(seed, p.monotonic_function_[d] );
    }
    return seed;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version ) {
    ar & boost::serialization::base_object<Parameter>(*this);
    ar & monotonic_function_;
  }
private:
  virtual void print ( std::ostream & stream ) const {
    stream << "BooleanSwitchingParameter(";
    for ( int d = 0; d < monotonic_function_ . size (); ++ d ) {
      if ( d != 0 ) stream << ", ";
      stream << monotonic_function_[d];
    }
    stream << ")";
  }

};

BOOST_CLASS_EXPORT_KEY(BooleanSwitchingParameter);

#ifdef MAIN_CPP_FILE
BOOST_CLASS_EXPORT_IMPLEMENT(BooleanSwitchingParameter);
#endif

#endif
