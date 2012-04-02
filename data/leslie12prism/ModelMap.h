/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "chomp/Prism.h"

#include "database/maps/simple_interval.h"
#include <vector>

#define PRISM_MAP

struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval p0, p1;
  
  ModelMap ( const Rect & rectangle ) {
    p0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    p1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Prism operator () ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);

    /*
    interval y0 = (p0 * x0 + p1 * x1 ) * exp ( -0.1 * (x0 + x1 ) );
    interval y1 = (double).7 * x0;

    Prism p ( 2 );

    p . c ( 0 ) = (y0 . lower () + y0 . upper ()) / 2.0;
    p . c ( 1 ) = (y1 . lower () + y1 . upper ()) / 2.0;

    uVector h ( 2 );
    h ( 0 ) = (y0 . upper () - y0 . lower ()) / 2.0;
    h ( 1 ) = (y1 . upper () - y1 . lower ()) / 2.0;

    p . A ( 0, 0 ) = h ( 0 );
    p . A ( 0, 1 ) = 0.0;
    p . A ( 1, 0 ) = 0.0;
    p . A ( 1, 1 ) = h ( 1 );
    
    return p;
    */
    
    // Calculate original center point c0
    uVector c0 ( 2 );
    c0 ( 0 ) = x0 . mid ();
    c0 ( 1 ) = x1 . mid ();

    uVector h ( 2 );
    h ( 0 ) = x0 . radius ();
    h ( 1 ) = x1 . radius ();

    // Calculate image center point c = f(c0)
    interval a = p0 * c0(0) + p1 * c0(1);
    Real e = exp ( -0.1 * (c0(0) + c0(1)) );
                  
    interval fx = a * e; //( p0 * c0(0) + p1 * c0(1) ) 
                         //* exp ( -0.1 * (c0(0) + c0(1)) );
    interval fy = 0.7 * c0(0);
    
    // Calculate derivatives
    //interval fx_x = (p0 - .1 * ( p0 * c0(0) + p1 * c0(1) ) ) * exp ( -.1 * ( c0(0) + c0(1) ) );
    //interval fx_y = (p1 - .1 * ( p0 * c0(0) + p1 * c0(1) ) ) * exp ( -.1 * ( c0(0) + c0(1) ) );
    
    interval fx_x = (p0 - .1 * a ) * e;
    interval fx_y = (p1 - .1 * a ) * e;
    interval fy_x = .7;
    interval fy_y = 0.0;
    /*
    interval fx_xx = ( - .1 * 2.0 * p0 + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
                     * exp ( -.1 * ( c0(0) + c0(1) ) );
    interval fx_xy = ( - .1 * (p0 + p1) + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
    * exp ( -.1 * ( c0(0) + c0(1) ) );
    interval fx_yy = ( - .1 * 2.0 * p1 + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
    * exp ( -.1 * ( c0(0) + c0(1) ) );
    */
    
    // Calculate output prism
    Prism p ( 2 );
    p . c ( 0 ) = fx . mid ();
    p . c ( 1 ) = fy . mid ();
    p . A ( 0, 0 ) =  fx_x . mid () * h ( 0 ) * 2.0;
    p . A ( 1, 0 ) =  fy_x . mid () * h ( 0 ) * 2.0;
    p . A ( 0, 1 ) =  fx_y . mid () * h ( 1 ) * 2.0;
    p . A ( 1, 1 ) =  fx_y . mid () * h ( 1 ) * 2.0;
  
    return p;
  } 
  
};

#endif
