/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include <cmath>

// boost interval doesn't seem to want to give me exp
// This simple alternative will work as long as we as approximate
// interval calculations are suitable for our purposes
// (i.e. the tolerance addition in the toplex will give use an enclosure)

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

struct LeslieMap {

  //using namespace boost;
  //using namespace numeric;
  /*
  typedef boost::numeric::interval<double, boost::numeric::interval_lib::
  policies<
    boost::numeric::interval_lib::save_state<
    boost::numeric::interval_lib::rounded_transc_opp<double> >,
    boost::numeric::interval_lib::checking_base<double> > > interval;
  */
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;

  LeslieMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    return;
  }
  Adaptive_Cubical::Geometric_Description operator () (
    const Adaptive_Cubical::Geometric_Description & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * 
      exp ( (double) -0.1 * (x0 + x1) );
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    Adaptive_Cubical::Geometric_Description return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } /* MapPopModel::compute */

};

#endif
