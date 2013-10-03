/* Cubic Julia Map */

#ifndef CMDP_NEWTONJULIA_H
#define CMDP_NEWTONJULIA_H

//#include <boost/numeric/interval.hpp>
//#include "database/structures/RectGeo.h"
#include "chomp/Rect.h"
#include "database/numerics/simple_interval.h"
#include <boost/shared_ptr.hpp>
#include <vector>

struct ModelMap {
  typedef simple_interval<double> interval;
  typedef chomp::Rect Rect;
  interval parameter1, parameter2;
  
  ModelMap ( const Rect & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  //Rect operator () ( const boost::shared_ptr<Geo> & geo ) const {   
  //  return operator () ( * boost::dynamic_pointer_cast<Rect> ( geo ) );
  //}

  Rect operator () 
    ( const Rect & rectangle ) const {    
    /* Read input */
    interval x = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval y = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval xpow2 = square(x);
    interval xpow3 = x*xpow2;
    interval xpow4 = square(xpow2);
    interval xpow5 = xpow2*xpow3;
    interval ypow2 = square(y);
    interval ypow3 = y*ypow2;
    interval ypow4 = square(ypow2);
    interval ypow5 = ypow2*ypow3;

    interval u = xpow2 - ypow2 + 2.0*x*ypow4 
                + 2.0*xpow5 + 4.0 * xpow3 * ypow2;
    interval v = -2.0 * x * y + 2.0 * ypow5 
                 + 4.0*xpow2*ypow3 + 2.0 * xpow4 * y;
    interval w = (xpow2 + ypow2);
    w = 3.0 * w * w;
    u = u / w;
    v = v / w;
    /* Write output */
    Rect return_value ( 2 );
    return_value . lower_bounds [ 0 ] = u . lower ();
    return_value . upper_bounds [ 0 ] = u . upper ();
    return_value . lower_bounds [ 1 ] = v . lower ();
    return_value . upper_bounds [ 1 ] = v . upper ();
    return return_value;
  } 
  bool good ( void ) const { return true; }
};

#endif
