/* Angular Newton Map */

#ifndef CMDB_ANGULARNEWTONMAP_H
#define CMDB_ANGULARNEWTONMAP_H

#include "database/maps/Map.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/simple_interval.h"
#include <memory>
#include <vector>

struct ModelMap : public Map {
  
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
  ModelMap ( std::shared_ptr<Parameter> parameter ) {
    const RectGeo & rectangle = 
      * std::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    //a = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    //b = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    //c = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
    //phi = interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]);
    c = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] );
    phi = interval ( rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ] );
    a = interval (-1.0,-1.0 );
    b = interval ( 1.0, 1.0 );
    //a = interval ( rectangle.lower_bounds[2], rectangle.upper_bounds[2]);
    //b = interval ( rectangle.lower_bounds[3], rectangle.upper_bounds[3]);
    return;
  }

  std::shared_ptr<Geo> 
  operator () ( std::shared_ptr<Geo> geo ) const {   
    return std::shared_ptr<Geo> ( new RectGeo ( 
        operator () ( * std::dynamic_pointer_cast<RectGeo> ( geo ) ) ) );
  }

  bool good ( void ) const {

    interval x = (a - b)*(interval(1.0) - c)*cos(2.0*phi) + (a + b)*(interval(1.0) + c);
    interval y = 16.0*a*b*c;

    interval check = (x*x) - y;

    double tol = 1.0e-8;

    if( tol + check.upper() >= 0 && check.lower() - tol <= 0 )
      return false;
    return true;

  }
  RectGeo operator () 
  ( const RectGeo & rectangle ) const {    
    //RectGeo myresult = rectangle;
    //myresult . lower_bounds [ 0 ] -= .1;
    //myresult . upper_bounds [ 0 ] += .1;
    //return myresult;
    /* Read input */
    //RectGeo testoutput = rectangle;
    //testoutput . lower_bounds [ 0 ] -= .1;
    //testoutput . upper_bounds [ 0 ] += .1;
    //return testoutput;

    double tol = 1.0e-8;
    interval theta = interval (rectangle . lower_bounds [ 0 ] - tol, rectangle . upper_bounds [ 0 ] + tol);
    
    interval x = interval ( 1.0 ) + ( c - interval ( 1.0 ) ) * cos ( theta ) * cos ( theta );// 1 - 2 sin^2 = cos(2t)
    interval y = (a + b ) * 0.5 + ( a - b ) * 0.5 * cos ( 2.0 * ( phi + theta ) ); // -cos(2t - pi/2)= -sin(2t)

    //std::cout << "a = " << a . lower () << "\n";
    //std::cout << "b = " << b . lower () << "\n";
    //std::cout << "phi = " << phi . lower ()  << "\n";
    //std::cout << "theta = " << theta . lower () << " " << theta. upper () << "\n";
    //std::cout << "x = " << x . lower () << " " << x . upper () << "\n";
    //std::cout << "y = " << y . lower () << " " << y . upper () << "\n";
    //std::cout << "x = " << cos(2.0*theta) . lower() << "\n";
    //std::cout << "y = " << -1.0*sin(2.0*theta) . lower() << "\n";
    //interval x = cos(theta)*cos(theta) + c*sin(theta)*sin(theta);
    //interval y = a*cos(theta + phi)*cos(theta + phi) + b*sin(theta + phi)*sin(theta + phi);
    
    //interval x = cos(2.0*theta);
    //interval y = -1.0*sin(2.0*theta);

    const double pi = 3.1415926535897932384626433832795;
     
    double A = x . lower ();
    double B = x . upper ();
    double C = y . lower ();
    double D = y . upper ();
    
    double t1 = atan2 ( C, A );
    double t2 = atan2 ( C, B );
    double t3 = atan2 ( D, B );
    double t4 = atan2 ( D, A );
    
    RectGeo return_value ( 1 );
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
    return_value . lower_bounds [ 0 ] -= 1.0e-8;
    return_value . upper_bounds [ 0 ] += 1.0e-8;

    return return_value;
  } 
  
};

#endif
