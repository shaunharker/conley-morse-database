/* INTERVAL CLASS */

// note: this is extremely simplistic and doesn't provide rigorous bounds yet

#ifndef CMDP_SIMPLEINTERVAL_H
#define CMDP_SIMPLEINTERVAL_H

#include <cmath>

// boost interval doesn't seem to want to give me exp
// This simple alternative will work as long as we as approximate
// interval calculations are suitable for our purposes

template < class Real >
struct simple_interval {
  Real lower_;
  Real upper_;
  simple_interval ( void ) {}
  simple_interval ( Real lower_, Real upper_ ) : lower_(lower_), upper_(upper_) {}
  Real lower ( void ) { return lower_; }
  Real upper ( void ) { return upper_; }
};

template < class Real >
simple_interval<Real> operator * ( const Real lhs, const simple_interval<Real> & rhs ) {
  simple_interval<Real> result;
  result . lower_ = lhs * rhs . lower_;
  result . upper_ = lhs * rhs . upper_;
  return result;
}

template < class Real >
simple_interval<Real> operator * ( const simple_interval<Real> & lhs, const simple_interval<Real> & rhs ) {
  simple_interval<Real> result;
  Real a = lhs . lower_ * rhs . lower_;
  Real b = lhs . lower_ * rhs . upper_;
  Real c = lhs . upper_ * rhs . lower_;
  Real d = lhs . upper_ * rhs . upper_;
  result . lower_ = std::min ( std::min ( a, b ), std::min ( c, d ) );
  result . upper_ = std::max ( std::max ( a, b ), std::max ( c, d ) );
  return result;
}

template < class Real >
simple_interval<Real> operator + ( const simple_interval<Real> & lhs, const simple_interval<Real> & rhs ) {
  simple_interval<Real> result;
  result . lower_ = lhs . lower_ + rhs . lower_;
  result . upper_ = lhs . upper_ + rhs . upper_;
  return result;  
}

template < class Real >
simple_interval<Real> exp ( const simple_interval<Real> & exponent ) {
  simple_interval<Real> result;
  result . lower_ = std::exp ( exponent . lower_ );
  result . upper_ = std::exp ( exponent . upper_ );
  return result;  
}
  
#endif
