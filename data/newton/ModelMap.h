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
    a = interval (-1.0,-1.0 );
    b = interval ( 1.0, 1.0 );
    return;
  }

  bool good ( void ) const {
    //std::cout << "Checking if map is good\n";
    interval cnew = c;
    if ( c . lower () <= 0.0 && c . upper () > 0.0 ) {
      cnew . upper_ = 0.0;
    }
    if ( c . lower () > 0.0 ) return true;
    cnew = -1.0 * cnew;
    if ( cnew . lower () == -0.0 ) cnew . lower_ = 0.0;
    if ( cnew . upper () == -0.0) cnew . upper_ = 0.0;
 
    interval mu;
    interval nu;
    interval alpha;
    interval beta;
    if ( phi . lower () > 3.1415926535/4.0 && phi . lower () < 3.0*3.1415926535/4.0 ) {
      mu = pow ( cnew, 0.5 );
      nu = cot ( phi );
      alpha = a;
      beta = b;
      //std::cout << "Case 1.\n";
    } else {
      mu = pow ( cnew, 0.5 );
      nu = tan ( phi );
      alpha = b;
      beta = a;
      //std::cout << "Case 2.\n";
    }

    std::cout << "Good:\n";
    std::cout << "a = [" << a . lower () << ", " << a .upper () << "]\n";
    std::cout << "b = [" << b . lower () << ", " << b .upper () << "]\n";
    std::cout << "c = [" << c . lower () << ", " << c .upper () << "]\n";
    std::cout << "phi = [" << phi . lower () << ", " << phi .upper () << "]\n\
";
    std::cout << "mu = [" << mu . lower () << ", " << mu .upper () << "]\n";
    std::cout << "nu = [" << nu . lower () << ", " << nu .upper () << "]\n";

    bool result = true;
    interval check;
    check = pow ( (interval(1.0,1.0) - mu * nu ), 2.0) * alpha +
      pow ( mu + nu, 2.0 ) * beta;
    
    double tol = 0.0;

    std::cout << "check1 = [" << check . lower () << ", " << check .upper () \
	      << "]\n";
    if ( check . lower () <= tol && check . upper () >= -tol ) { 
      result = false;
    }
    check = pow ( (interval(1.0,1.0) + mu * nu ), 2.0) * alpha +
      pow ( nu - mu, 2.0 ) * beta;

    std::cout << "check2 = [" << check . lower () << ", " << check .upper () \
	      << "]\n";
    if ( check . lower () <= tol && check . upper () >= -tol ) {
      result = false;
    }
    //    std::cout << "No singularity.\n";
    return result;

    
  }
  chomp::Rect operator () 
  ( const chomp::Rect & rectangle ) const {    
    //chomp::Rect myresult = rectangle;
    //myresult . lower_bounds [ 0 ] -= .1;
    //myresult . upper_bounds [ 0 ] += .1;
    //return myresult;
    /* Read input */
    //chomp::Rect testoutput = rectangle;
    //testoutput . lower_bounds [ 0 ] -= .1;
    //testoutput . upper_bounds [ 0 ] += .1;
    //return testoutput;
    
    interval theta = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    
    interval x = interval ( 1.0 ) + ( c - interval ( 1.0 ) ) * sin ( theta ) * sin ( theta );
    interval y = (a + b ) * 0.5 + ( a - b ) * 0.5 * cos ( 2.0 * ( phi + theta ) );

    //interval x = cos(theta)*cos(theta) + c*sin(theta)*sin(theta);
    //interval y = a*cos(theta + phi)*cos(theta + phi) + b*sin(theta + phi)*sin(theta + phi);
    
    //interval x = cos(theta);
    //interval y = sin(theta);

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
    if ( A*B <= 0.0 && C*D <= 0.0 ) {
      return_value . lower_bounds [ 0 ] = -pi;
      return_value . upper_bounds [ 0 ] = pi;
      //std::cout << "Singularity.\n";
    }

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
