#ifndef RK4_H
#define RK4_H

template < class Point, class ODEMap >
Point RK4 ( double dt, const Point & x, const ODEMap & f ) { 

  Point k1, k2, k3, k4; 

  k1 = f ( x );
  k2 = f ( x + (dt/2.0) * k1 );
  k3 = f ( x + (dt/2.0) * k2 );
  k4 = f ( x + dt * k3 );

  return x + (dt/6.0) * ( k1 + 2.0 * k2 + 2.0 * k3 + k4 ); 
}



#endif
