/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "database/numerics/simple_interval.h"
#include <vector>

struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval alphaIm, alphaRe, p;
  // f(z) = z + p * ( alpha \bar{z} - |z|^2 z )
  // p > 0
  // f(x, y) = ( x + p * ( alpha_x x + alpha_y y - (x*x+y*y) x),
  //             y + p * ( - alpha_x y + alpha_y x - (x*x+y*y) y )
  ModelMap ( const chomp::Rect & rectangle ) {
    alphaRe = interval ( rectangle.lower_bounds[0], rectangle.upper_bounds[0]);
    alphaIm = interval ( rectangle.lower_bounds[1], rectangle.lower_bounds[1]);
    p = interval ( .3, .3 );// rectangle.lower_bounds[2], rectangle.upper_bounds[0]);
    return;
  }

  bool good ( void ) const {
    return true;
  }
  
  chomp::Rect operator () 
  ( const chomp::Rect & rectangle ) const {
    using namespace chomp;
    interval x ( rectangle.lower_bounds[0], rectangle.upper_bounds[0]);
    interval y ( rectangle.lower_bounds[1], rectangle.upper_bounds[1]);
    interval normsquare = x*x + y*y;
    interval xnew = x + p * ( alphaRe * x + alphaIm * y + (-1.0)*normsquare * x );
    interval ynew = y + p * ( -1.0 * alphaRe * y + alphaIm * x + (-1.0)* normsquare * y );
    
    Rect result ( 2 );
    result . lower_bounds[0] = xnew . lower ();
    result . upper_bounds[0] = xnew . upper ();
    result . lower_bounds[1] = ynew . lower ();
    result . upper_bounds[1] = ynew . upper ();
    
    return result;
  } 
  
};

#endif
