/* Integrator for Database using CAPD */

#ifndef CMDP_INTEGRATOR_H
#define CMDP_INTEGRATOR_H

#include "capd/krak/krak.h"
#include "capd/capdlib.h"


#include "chomp/Rect.h"

#include <cmath>
#include <stdexcept>
#include <fstream>

struct LorenzMap {
  capd::IMap f;
  double step;
  interval timestep;
  int order;
  capd::ITaylor * T;
  capd::ITimeMap * timemap;
  
  LorenzMap ( const chomp::Rect & rectangle ) {
    //interval parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    //parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
		using namespace capd;
    
    //IC2Map f=
    //f = "par:c;var:x,y;fun:x,c*y;";
    //f = "var:x,y;fun:x,y;";
    //f = "par:c;var:x,y;fun:y,-x+c*(1-x*x)*y;";
    
    f = "par:a,b,c;var:x,y,z;fun:a*(y-x),b*x-y-x*z,-c*z+x*y;";
    f.setParameter("a", interval(10.00) ); // 10
    f.setParameter("b", interval(13.00) ); // 28
    f.setParameter("c", interval(2.6667) ); // 8/3
    
    timestep = interval(1.0)/10.0;
    step=.01;
    order = 3;
    //f.setParameter("c",interval(1.0));
    //f.setParameter("c", parameter1 );
    
    T = new ITaylor (f,order,step);
    timemap = new ITimeMap ( *T );
    return;
  }
  
  ~LorenzMap ( void ) {
    delete T;
    delete timemap;
	}
  
  chomp::Prism operator () ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    /* Read input */
    IVector x0 ( 3 );
    for ( int d = 0; d < 3; ++ d )
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ], 
                           rectangle . upper_bounds [ d ] );
    C0RectSet rect ( x0 );
    
    //Rect return_value ( 3 );
    Prism P ( 3 );
    try{
      /* Compute forward image */
      //rect.move(*T);
      
      /* Write output */
      //IVector w = (*timemap) ( timestep, rect );
      for (int i =0; i<10; ++i ) {
        rect.move(*T);
      }
      IMatrix B = rect . get_B ();
      IVector r = rect . get_r ();
      IVector x = rect . get_x ();
      // x + Br
      IMatrix D ( 3, 3 );
      for ( int d = 0; d < 3; ++ d ) {
        D (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),  
                               abs ( r [ d ] . leftBound () ) );
      }
      B = B * D;
                              
      for ( int d = 0; d < 3; ++ d ) P . c ( d ) = (x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
      for ( int i = 0; i < 3; ++ i ){
        for ( int j = 0; j < 3; ++ j ) {
          P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
        }
      }
      
      // B r = B D Dinv r = B D e    where e = [-1,1]^3
      // D e = r
  
      //std::cout << rectangle << " --> " << w << "\n";
      /*
      return_value . lower_bounds [ 0 ] = w[ 0 ] . leftBound();
      return_value . upper_bounds [ 0 ] = w[ 0 ] . rightBound();
      return_value . lower_bounds [ 1 ] = w[ 1 ] . leftBound();
      return_value . upper_bounds [ 1 ] = w[ 1 ] . rightBound();
      return_value . lower_bounds [ 2 ] = w[ 2 ] . leftBound();
      return_value . upper_bounds [ 2 ] = w[ 2 ] . rightBound();
       */
    }
    catch (std::exception& e)
    {	
      /*
    	return_value . lower_bounds [ 0 ] = -25.0;
      return_value . upper_bounds [ 0 ] = 25.0;
      return_value . lower_bounds [ 1 ] = -40.0;
      return_value . upper_bounds [ 1 ] = 40.0;
      return_value . lower_bounds [ 2 ] = -5.0;
      return_value . upper_bounds [ 2 ] = 60.0;
       */
      for ( int d = 0; d < 3; ++ d ) P . A ( d, d ) = 100.0;
      
    }
    return P;//return_value;
  }
  
};

#endif
