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

/*
	Adaptative 4th order Runge-Kutta. Two consecutive iterates 
	are within some length scale.
	We assume Point is really a point.
*/
template < class Point, class ODEMap >
Point RK4adapt ( double & dt, double lscale, const Point & x, const ODEMap & f ) { 

	double dtguess ( dt );
	Point newx;
	double distance;
	double lscale_sqr;

  Point k1, k2, k3, k4; 

  lscale_sqr = lscale*lscale;

  distance = 3.0*lscale_sqr;
  // we don't mind if the two iterates are too close
  // could be improve to be avoid many unnecessary computation 

  while ( distance > lscale_sqr ) {

  	// Compute the next iterate for the given dt
  	k1 = f ( x );
  	k2 = f ( x + (dtguess/2.0) * k1 );
  	k3 = f ( x + (dtguess/2.0) * k2 );
  	k4 = f ( x + dtguess * k3 );

	newx = x + (dtguess/6.0) * ( k1 + 2.0 * k2 + 2.0 * k3 + k4 );

	distance = 0.0;
// since it is a point, just the distance with the lower bounds
        for ( unsigned int i=0; i< x.dimension(); ++i ) {
            distance += ( newx.lower_bounds[i] - x.lower_bounds[i] ) *
	                ( newx.lower_bounds[i] - x.lower_bounds[i] );
	}
    
	if ( distance > 0.5 * lscale_sqr ) {
	  dtguess = .75*dtguess;
	}
  }

  return newx;
}

#endif
