/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "chomp/Prism.h"

#include "database/numerics/simple_interval.h"
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
	interval x2 = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
	interval x3 = interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]);
	interval x4 = interval (rectangle . lower_bounds [ 4 ], rectangle . upper_bounds [ 4 ]);
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
    uVector c0 ( 5 );
    c0 ( 0 ) = x0 . mid ();
    c0 ( 1 ) = x1 . mid ();
	c0 ( 2 ) = x2 . mid ();
	c0 ( 3 ) = x3 . mid ();
	c0 ( 4 ) = x4 . mid ();

    uVector h ( 5 );
    h ( 0 ) = x0 . radius ();
    h ( 1 ) = x1 . radius ();
	h ( 2 ) = x2 . radius ();
	h ( 3 ) = x3 . radius ();
	h ( 4 ) = x4 . radius ();
  
    // Calculate image center point c = f(c0)
	interval a = 15.0 * c0(0) + 15.0 * c0(1) + 15.0 * c0(2) + 15.0 * c0(3) + 15.0 * c0(4);
	Real e = exp ( -0.1 * (c0(0) + c0(1) + c0(2) + c0(3) + c0(4)) );
                  
    interval f0 = a * e; //( p0 * c0(0) + p1 * c0(1) ) 
                         //* exp ( -0.1 * (c0(0) + c0(1)) );
    interval f1 = 0.7 * c0(0);
	interval f2 = 0.7 * c0(1);
	interval f3 = 0.7 * c0(2);
	interval f4 = 0.7 * c0(3);

    // Calculate derivatives
    //interval fx_x = (p0 - .1 * ( p0 * c0(0) + p1 * c0(1) ) ) * exp ( -.1 * ( c0(0) + c0(1) ) );
    //interval fx_y = (p1 - .1 * ( p0 * c0(0) + p1 * c0(1) ) ) * exp ( -.1 * ( c0(0) + c0(1) ) );
    
    interval f0_0 = (15.0 - .1 * a ) * e;
    interval f0_1 = (15.0 - .1 * a ) * e;
	interval f0_2 = (15.0 - .1 * a ) * e;
	interval f0_3 = (15.0 - .1 * a ) * e;
	interval f0_4 = (15.0 - .1 * a ) * e;
    interval f1_0 = .7;
    interval f1_1 = 0.0;
	interval f1_2 = 0.0;
	interval f1_3 = 0.0;
	interval f1_4 = 0.0;
	interval f2_0 = 0.0;
	interval f2_1 = .7;
	interval f2_2 = 0.0;
	interval f2_3 = 0.0;
	interval f2_4 = 0.0;
	interval f3_0 = 0.0;
	interval f3_1 = 0.0;
	interval f3_2 = .7;
	interval f3_3 = 0.0;
	interval f3_4 = 0.0;
	interval f4_0 = 0.0;
	interval f4_1 = 0.0;
	interval f4_2 = 0.0;
	interval f4_3 = 0.0;
	interval f4_4 = 0.7;
    /*
    interval fx_xx = ( - .1 * 2.0 * p0 + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
                     * exp ( -.1 * ( c0(0) + c0(1) ) );
    interval fx_xy = ( - .1 * (p0 + p1) + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
    * exp ( -.1 * ( c0(0) + c0(1) ) );
    interval fx_yy = ( - .1 * 2.0 * p1 + .001 * ( p0 * c0(0) + p1 * c0(1) ) ) 
    * exp ( -.1 * ( c0(0) + c0(1) ) );
    */
    
    // Calculate output prism
    Prism p ( 5 );
    p . c ( 0 ) = f0 . mid ();
    p . c ( 1 ) = f1 . mid ();
	p . c ( 2 ) = f2 . mid ();
	p . c ( 3 ) = f3 . mid ();
	p . c ( 4 ) = f4 . mid ();
    p . A ( 0, 0 ) =  f0_0 . mid () * h ( 0 ) * 1.25;
    p . A ( 1, 0 ) =  f1_0 . mid () * h ( 0 ) * 1.25;
	p . A ( 2, 0 ) =  f2_0 . mid () * h ( 0 ) * 1.25;
	p . A ( 3, 0 ) =  f3_0 . mid () * h ( 0 ) * 1.25;
	p . A ( 4, 0 ) =  f4_0 . mid () * h ( 0 ) * 1.25;
    p . A ( 0, 1 ) =  f0_1 . mid () * h ( 1 ) * 1.25;
    p . A ( 1, 1 ) =  f1_1 . mid () * h ( 1 ) * 1.25;
	p . A ( 2, 1 ) =  f2_1 . mid () * h ( 1 ) * 1.25;
	p . A ( 3, 1 ) =  f3_1 . mid () * h ( 1 ) * 1.25;
	p . A ( 4, 1 ) =  f4_1 . mid () * h ( 1 ) * 1.25;
	p . A ( 0, 2 ) =  f0_2 . mid () * h ( 2 ) * 1.25;
	p . A ( 1, 2 ) =  f1_2 . mid () * h ( 2 ) * 1.25;
	p . A ( 2, 2 ) =  f2_2 . mid () * h ( 2 ) * 1.25;
	p . A ( 3, 2 ) =  f3_2 . mid () * h ( 2 ) * 1.25;
	p . A ( 4, 2 ) =  f4_2 . mid () * h ( 2 ) * 1.25;
	p . A ( 0, 3 ) =  f0_3 . mid () * h ( 3 ) * 1.25;
	p . A ( 1, 3 ) =  f1_3 . mid () * h ( 3 ) * 1.25;
	p . A ( 2, 3 ) =  f2_3 . mid () * h ( 3 ) * 1.25;
	p . A ( 3, 3 ) =  f3_3 . mid () * h ( 3 ) * 1.25;
	p . A ( 4, 3 ) =  f4_3 . mid () * h ( 3 ) * 1.25;
	p . A ( 0, 4 ) =  f0_4 . mid () * h ( 4 ) * 1.25;
	p . A ( 1, 4 ) =  f1_4 . mid () * h ( 4 ) * 1.25;
	p . A ( 2, 4 ) =  f2_4 . mid () * h ( 4 ) * 1.25;
	p . A ( 3, 4 ) =  f3_4 . mid () * h ( 4 ) * 1.25;
	p . A ( 4, 4 ) =  f4_4 . mid () * h ( 4 ) * 1.25;
    return p;
  } 
  
};

#endif
