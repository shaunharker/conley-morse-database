/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "toplexes/Adaptive_Cubical_Toplex.h"
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


#if 0
struct LeslieMap {
  
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  LeslieMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Adaptive_Cubical::Geometric_Description operator () 
    (const Adaptive_Cubical::Geometric_Description & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    interval x2 = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
    /*	interval x3 = interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]);
     interval x4 = interval (rectangle . lower_bounds [ 4 ], rectangle . upper_bounds [ 4 ]);
     interval x5 = interval (rectangle . lower_bounds [ 5 ], rectangle . upper_bounds [ 5 ]);
     interval x6 = interval (rectangle . lower_bounds [ 6 ], rectangle . upper_bounds [ 6 ]);*/
    /* Perform map computation */
#ifdef FISHMODEL
    interval z =  (parameter1 * x0 + parameter2 * x1 + (double) .026 * x2 );
    interval y0 = (double) 400.0 * z * exp ( (double) -0.1 * z);    
#else
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * exp ( (double) -0.1 * (x0 + x1) );     
#endif
    interval y1 = (double) 0.7 * x0;
    interval y2 = (double) 0.7 * x1;  
    /*	interval y3 = (double) 0.7 * x2;
     interval y4 = (double) 0.7 * x3;
     interval y5 = (double) 0.7 * x4;
     interval y6 = (double) 0.7 * x5;*/
	  
    
    /* Write output */
    Adaptive_Cubical::Geometric_Description return_value ( 3 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    /*	return_value . lower_bounds [ 3 ] = y3 . lower ();
     return_value . upper_bounds [ 3 ] = y3 . upper ();
     return_value . lower_bounds [ 4 ] = y4 . lower ();
     return_value . upper_bounds [ 4 ] = y4 . upper ();
     return_value . lower_bounds [ 5 ] = y5 . lower ();
     return_value . upper_bounds [ 5 ] = y5 . upper ();
     return_value . lower_bounds [ 6 ] = y6 . lower ();
     return_value . upper_bounds [ 6 ] = y6 . upper ();  */
    std::cout << "f(" << rectangle << ") = " << return_value << "\n";
    return return_value;
  } 
};
#else
  struct LeslieMap {

  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;

  LeslieMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Adaptive_Cubical::Geometric_Description operator () (
    const Adaptive_Cubical::Geometric_Description & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
#ifdef FISHMODEL
    interval z = (parameter1 * x0 + parameter2 * x1 );
    interval y0 = z * exp ( (double) -0.1 * z);    
#else
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * exp ( (double) -0.1 * (x0 + x1) );     
#endif
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    Adaptive_Cubical::Geometric_Description return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } 

};
#endif
  
#endif
