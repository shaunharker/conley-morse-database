/* WavePool Map */

#ifndef CMDP_WAVEPOOL_H
#define CMDP_WAVEPOOL_H


#include "chomp/Rect.h"
#include "database/numerics/RK4.h"
#include <cmath>


class WavePoolODEMap {
public:
  double alpha1, K1;
  double alpha3, K3;
  double alpha4, K4;
  double alpha21, K21;
  double alpha22, K22;
  double n;
  typedef chomp::Rect Point;
  WavePoolODEMap ( const chomp::Rect & rectangle ) {
    alpha1 = rectangle.lower_bounds[0];
    K1 = rectangle.lower_bounds[1];
    alpha3 = rectangle.lower_bounds[2];
    K3 = rectangle.lower_bounds[3];
    alpha4 = rectangle.lower_bounds[4];
    K4 = rectangle.lower_bounds[5];
    alpha21 = rectangle.lower_bounds[6];
    K21 = rectangle.lower_bounds[7];
    alpha22 = rectangle.lower_bounds[8];
    K22 = rectangle.lower_bounds[9];
    n = rectangle.lower_bounds[10];
    
  }
  Point operator () ( const chomp::Rect & in ) const {
    double a = in . lower_bounds [ 0 ];
    double b = in . lower_bounds [ 1 ];
    double c = in . lower_bounds [ 2 ];
    double d = in . lower_bounds [ 3 ];
    
    // HERE WE USE n = 4
    double na = -a + (K1*K1*K1*K1 * alpha1)/(b*b*b*b + K1*K1*K1*K1);
    double nb = -b + (a*a*a*a * alpha21)/(a*a*a*a + K21*K21*K21*K21) + (d*d*d*d * alpha22)/(d*d*d*d + K22*K22*K22*K22);
    double nc = -c + (a*a*a*a * alpha3)/(a*a*a*a + K3*K3*K3*K3);
    double nd = -d + (c*c*c*c * alpha4)/(c*c*c*c + K4*K4*K4*K4);
    
    Point out ( 4 );
    out . lower_bounds [ 0 ] = na;
    out . upper_bounds [ 0 ] = na;
    out . lower_bounds [ 1 ] = nb;
    out . upper_bounds [ 1 ] = nb;
    out . lower_bounds [ 2 ] = nc;
    out . upper_bounds [ 2 ] = nc;
    out . lower_bounds [ 3 ] = nd;
    out . upper_bounds [ 3 ] = nd;
    
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
  WavePoolODEMap * f;
  MapRect ( void ) {}
  
  MapRect ( const chomp::Rect & rectangle, double dt, int steps, int samples ) :
  dt(dt), steps(steps), samples(samples) {
    f = new WavePoolODEMap ( rectangle );
  }
  ~MapRect ( void ) {
    if ( f != NULL ) delete f;
  }
  
  mutable boost::unordered_map<chomp::Rect, chomp::Rect> lookup;
  chomp::Rect pointMap ( const chomp::Rect & input ) const {
    static size_t cache_hits = 0;
    static size_t cache_miss = 0;
    if ( lookup . count ( input ) == 0 ) {
      chomp::Rect x = input;
      for ( int i = 0; i < steps; ++ i ) x = RK4 ( dt, x, *f );
      lookup [ input ] = x;
      ++ cache_miss;
      //if ( cache_miss % 1000 == 0 ) std::cout << "cache MISS = " << cache_miss << "\n";
      //if ( cache_miss % 100000 == 0 ) std::cout << "MISS RATE = " << (double)cache_miss/(double)(cache_hits + cache_miss) << "  " << "Miss =" << cache_miss << "  HIT = " << cache_hits << " Total=" << cache_hits + cache_miss << "\n";

      
    } else {
      ++ cache_hits;
      //if ( cache_hits % 100000 == 0 ) std::cout << "cache hits = " << cache_hits << "\n";

    }

    return lookup [ input ];
  }
  
  //std::vector<chomp::Rect>  operator () ( const chomp::Rect & rectangle ) const {
  chomp::Rect  operator () ( const chomp::Rect & rectangle ) const {
    const int D = 4;   // dimension of phase space
    const int N = 81;  // N == 3^D

    // Choose N points to integrate forward
    static chomp::Rect points[N];
    int i = 0;
    for ( double u = 0; u < 1.1; u += (1.0/2.0) ) {
      for ( double v = 0; v < 1.1; v += (1.0/2.0) ) {
        for ( double w = 0; w < 1.1; w += (1.0/2.0) ) {
          for ( double s = 0; s < 1.1; s += (1.0/2.0) ) {
            chomp::Rect & x = points [ i ++ ];
            x = rectangle;
            x . lower_bounds [ 0 ] = x . upper_bounds [ 0 ] =
            u * x . lower_bounds [ 0 ] + (1.0 - u ) * x . upper_bounds [ 0 ];
            x . lower_bounds [ 1 ] = x . upper_bounds [ 1 ] =
            v * x . lower_bounds [ 1 ] + (1.0 - v ) * x . upper_bounds [ 1 ];
            x . lower_bounds [ 2 ] = x . upper_bounds [ 2 ] =
            w * x . lower_bounds [ 2 ] + (1.0 - w ) * x . upper_bounds [ 2 ];
            x . lower_bounds [ 3 ] = x . upper_bounds [ 3 ] =
            s * x . lower_bounds [ 3 ] + (1.0 - s ) * x . upper_bounds [ 3 ];
          }
        }
      }
    }
    
    // Integrate
    for ( i = 0 ; i < N; ++ i ) points [ i ] = pointMap ( points [ i ] );
      
    // Find bounding box
    chomp::Rect result = points [ 0 ];
    for ( i = 1 ; i < N; ++ i ) {
      for ( int d = 0; d < D; ++ d ) {
        result . lower_bounds [ d ] = std::min ( result . lower_bounds [ d ], points [ i ] . lower_bounds [ d ] );
        result . upper_bounds [ d ] = std::max ( result . upper_bounds [ d ], points [ i ] . upper_bounds [ d ] );
      }
    }
    
    return result;
    
  }
  
  bool good ( void ) const { return true; }
};

#endif
