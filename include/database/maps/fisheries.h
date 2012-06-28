/* Leslie Map */

#ifndef CMDP_FISHMAP_H
#define CMDP_FISHMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Toplex.h"
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

struct FishMap3 {
  using namespace chomp;
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

  FishMap3 ( const Rect & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () (
    const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    interval x2 = interval ( rectangle . lower_bounds [ 2 ], 
                            rectangle . upper_bounds [ 2 ]);
    
    /* Perform map computation */
    Real p = exp ( (double) -0.5 );
    interval y0 = p * ( 2.6 * x2 ) * exp ( (double) -0.1 * (2.6 * x2) );
    interval y1 = p * x0;
    interval y2 = p * x1;
    
    /* Write output */
    Rect return_value ( 3 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    return return_value;
  } /* MapPopModel::compute */

};

struct FishMap4 {
  using namespace chomp;
  typedef simple_interval<double> interval;
  FishMap4 ( const Rect & rectangle ) {}
  Rect operator () (
                                                       const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
                            rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    interval x2 = interval ( rectangle . lower_bounds [ 2 ], 
                            rectangle . upper_bounds [ 2 ]);
    interval x3 = interval ( rectangle . lower_bounds [ 3 ], 
                            rectangle . upper_bounds [ 3 ]);
    
    /* Perform map computation */
    Real p = .606530659712;
    const Real scaling = 5.0 / ( .026 + .046 );
    interval y0 = scaling * ( .026 * x2 + .046 * x3 ) * exp ( (double) -0.1 * scaling * (.026 * x2 + .046 * x3 ) );
    interval y1 = p * x0;
    interval y2 = p * x1;
    interval y3 = p * x2;
    
    /* Write output */
    Rect return_value ( 5 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    return_value . lower_bounds [ 3 ] = y3 . lower ();
    return_value . upper_bounds [ 3 ] = y3 . upper ();
    
    return return_value;
  } /* MapPopModel::compute */
  
};

struct FishMap5 {
  using namespace chomp;
  typedef simple_interval<double> interval;
  FishMap5 ( const Rect & rectangle ) {}
  Rect operator () (
                                                       const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
                            rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    interval x2 = interval ( rectangle . lower_bounds [ 2 ], 
                            rectangle . upper_bounds [ 2 ]);
    interval x3 = interval ( rectangle . lower_bounds [ 3 ], 
                            rectangle . upper_bounds [ 3 ]);
    interval x4 = interval ( rectangle . lower_bounds [ 4 ], 
                            rectangle . upper_bounds [ 4 ]);
    
    /* Perform map computation */
    Real p = .606530659712;
    const Real scaling = 2.0 / ( .026 + .046 + .110 );
    interval y0 = scaling * ( .026 * x2 + .046 * x3 + .110 * x4) * exp ( (double) -0.1 * scaling * (.026 * x2 + .046 * x3 + .110 * x4) );
    interval y1 = p * x0;
    interval y2 = p * x1;
    interval y3 = p * x2;
    interval y4 = p * x4;
    
    /* Write output */
    Rect return_value ( 5 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    return_value . lower_bounds [ 3 ] = y3 . lower ();
    return_value . upper_bounds [ 3 ] = y3 . upper ();
    return_value . lower_bounds [ 4 ] = y4 . lower ();
    return_value . upper_bounds [ 4 ] = y4 . upper ();
    return return_value;
  } /* MapPopModel::compute */
  
};


#endif
