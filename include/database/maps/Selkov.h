/* Integrator for Database using CAPD */

#ifndef CMDP_INTEGRATOR_H
#define CMDP_INTEGRATOR_H


#include "chomp/Rect.h"
#include "database/numerics/RK4.h"
#include <cmath>


class SelkovODEMap {
public:
  double nu;
  double eta;
  double alpha;
  double gamma;

  SelkovODEMap ( const chomp::Rect & rectangle ) {
    nu = rectangle.lower_bounds[0];
    eta = rectangle.lower_bounds[1];
    alpha = rectangle.lower_bounds[2];
    gamma = rectangle.lower_bounds[3];

  }
  Point operator () ( const chomp::Rect & in ) const {
    double x = in . lower_bounds [ 0 ];
    double y = in . lower_bounds [ 1 ];
    double nx = (nu-x*y*y/(x*y*y+y*y+1.0));
    double ny = (alpha*x*y*y/(x*y*y+y*y+1.0)-eta*y);

    //double nx = -y+x*(1.0-x*x-y*y);
    //double ny = x+y*(1.0-x*x-y*y);

    Point out ( 2 );
    out . lower_bounds [ 0 ] = nx;
    out . upper_bounds [ 0 ] = nx;
    out . lower_bounds [ 1 ] = ny;
    out . upper_bounds [ 1 ] = ny;
    return out;
  }
};

// Generic name for the class MapRect
class MapRect {
public:
  typedef chomp::Rect Rect;
  typedef chomp::Rect image_type;
  double dt;
  int steps;
  int samples;
  SelkovODEMap * f;
  MapRect ( void ) {}
  
  MapRect ( const chomp::Rect & rectangle, double dt, int steps, int samples ) :
  dt(dt), steps(steps), samples(samples) {
    f = new SelkovODEMap ( rectangle );
  }
  ~MapRect ( void ) {
      if ( f != NULL ) delete f;
  }
  std::vector<chomp::Rect>  operator () ( const chomp::Rect & rectangle ) const {
    std::vector<chomp::Rect> result;
    for ( double u = 0; u < 1.1; u += (1.0/3.0) ) {
      for ( double v = 0; v < 1.1; v += (1.0/3.0) ) {
        chomp::Rect x = rectangle;
        x . lower_bounds [ 0 ] = x . upper_bounds [ 0 ] =
          u * x . lower_bounds [ 0 ] + (1.0 - u ) * x . upper_bounds [ 0 ];
        x . lower_bounds [ 1 ] = x . upper_bounds [ 1 ] =
          v * x . lower_bounds [ 1 ] + (1.0 - v ) * x . upper_bounds [ 1 ];
      for ( int i = 0 ; i < steps; ++ i ) x = RK4 ( dt, x, *f );
      result . push_back ( x );  
      }
    }
    return result;
  }
  
  bool good ( void ) const { return true; }
};

#endif

#if 0
for ( int sample = 0; sample < samples; ++ sample ) {
      chomp::Rect x = rectangle;
      double u = (double)(rand()%(1L<<32))/(double)(1L<<32);
      double v = (double)(rand()%(1L<<32))/(double)(1L<<32);
      x . lower_bounds [ 0 ] = x . upper_bounds [ 0 ] =
        u * x . lower_bounds [ 0 ] + (1.0 - u ) * x . upper_bounds [ 0 ];
      x . lower_bounds [ 1 ] = x . upper_bounds [ 1 ] =
        v * x . lower_bounds [ 1 ] + (1.0 - v ) * x . upper_bounds [ 1 ];
      for ( int i = 0 ; i < steps; ++ i ) x = RK4 ( dt, x, *f );
      result . push_back ( x );
    }
    #endif

