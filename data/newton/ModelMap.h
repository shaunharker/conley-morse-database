/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "database/maps/simple_interval.h"
#include <vector>

struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval a, b, c, phi;
  interval Bxx, Bxy, Byx, Byy;
  /*
   parameters: c, a, b, theta
   x |-> (x' A x, x' B x )
   A = [ 1 0
   0 c ];
   B = R^{-1}_\theta [ a 0; 0 b ] R_\theta
   */
  ModelMap ( const chomp::Rect & rectangle ) {
    //a = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    //b = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    //c = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
    //phi = interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]);
    c = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] );
    phi = interval ( rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ] );
    a = interval ( -.5, -.5 );
    b = interval ( .5, .5 );
    return;
  }
  chomp::Rect operator () 
  ( const chomp::Rect & rectangle ) const {    
    /* Read input */
    chomp::Rect testoutput = rectangle;
    testoutput . lower_bounds [ 0 ] -= .1;
    testoutput . upper_bounds [ 0 ] += .1;
    return testoutput;

    interval theta = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    
    interval x = interval ( 1.0 ) + ( interval ( 1.0 ) - c ) * sin ( theta ) * sin ( theta );
    interval y = (a + b ) * 0.5 + ( a - b ) * 0.5 * cos ( 2.0 * ( phi + theta ) );
    
    const chomp::Real pi = 3.1415926535897932384626433832795;
     
    double A = x . lower ();
    double B = x . upper ();
    double C = y . lower ();
    double D = y . upper ();
    
    double t1 = atan2 ( C, A );
    double t2 = atan2 ( C, B );
    double t3 = atan2 ( D, B );
    double t4 = atan2 ( D, A );
    
    chomp::Rect return_value ( 1 );
    
    if ( C*D < 0.0 ) {
      if ( A*B < 0.0 ) {
        // [0,2pi)
        return_value . lower_bounds [ 0 ] = -pi;
        return_value . upper_bounds [ 0 ] = pi;
      } else {
        if ( A < 0 ) {
          // QUADRANT II and III
          return_value . lower_bounds [ 0 ] = t3;
          return_value . upper_bounds [ 0 ] = t2 + 2*pi;
          
        } else {
          // QUADRANT I and IV
          return_value . lower_bounds [ 0 ] = t1;
          return_value . upper_bounds [ 0 ] = t4;
          
        }
      }
    } else {
      if ( A*B < 0.0 ) {
        if ( C < 0 ) {
          // QUADRANT III and IV
          return_value . lower_bounds [ 0 ] = t4;
          return_value . upper_bounds [ 0 ] = t3;
          
        } else {
          // QUADRANT I and II
          return_value . lower_bounds [ 0 ] = t2;
          return_value . upper_bounds [ 0 ] = t1;
          
        }
      } else {
        if ( A < 0 ) {
          if ( C < 0 ) {
            // QUADRANT III
            return_value . lower_bounds [ 0 ] = t4;
            return_value . upper_bounds [ 0 ] = t2;
            
          } else {
            // QUADRANT II
            return_value . lower_bounds [ 0 ] = t3;
            return_value . upper_bounds [ 0 ] = t1;
            
          }
        } else {
          if ( C < 0 ) {
            // QUADRANT IV
            return_value . lower_bounds [ 0 ] = t1;
            return_value . upper_bounds [ 0 ] = t3;
            
          } else {
            // QUADRANT I
            return_value . lower_bounds [ 0 ] = t2;
            return_value . upper_bounds [ 0 ] = t4;
            
          }            
        }
      } 
    }

    return return_value;
  } 
  
};

#endif
