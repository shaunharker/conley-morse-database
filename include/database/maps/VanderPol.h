/* Integrator for Database using CAPD */

#ifndef CMDP_VANDERPOL_H
#define CMDP_VANDERPOL_H

#include "capd/krak/krak.h"
#include "capd/capdlib.h"


#include "chomp/Rect.h"

#include <cmath>
#include <stdexcept>
#include <fstream>

struct VanderPol {
  using namespace chomp;
  capd::IMap f;
  double step;
  interval timestep;
  int order;
  capd::ITaylor * T;
  capd::ITimeMap * timemap;
  
  VanderPol ( const Rect & rectangle ) {
    //interval parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    //parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
		using namespace capd;
    
    //IC2Map f=
    //f = "par:c;var:x,y;fun:x,c*y;";
    //f = "var:x,y;fun:x,y;";
    //f = "par:c;var:x,y;fun:y,-x+c*(1-x*x)*y;";
    
    f = "par:pa;var:x,y;fun:pa*(x-(x*x*x/3.0)-y),x/pa;";
    f.setParameter("pa", interval(3.00) ); // 10
    
    timestep = interval(0.1);
    step=.025;
    order = 15;
    //f.setParameter("c",interval(1.0));
    //f.setParameter("c", parameter1 );
    
    T = new ITaylor (f,order,step);
    timemap = new ITimeMap ( *T );
    return;
  }
  
  ~VanderPol ( void ) {
    delete T;
    delete timemap;
	}
  
  //Rect 
  Prism operator () ( const Rect & rectangle ) const {
    using namespace capd;
    
    /* Read input */
    IVector x0 ( 2 );
    for ( int d = 0; d < 2; ++ d )
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ], 
                           rectangle . upper_bounds [ d ] );
    C0RectSet rect ( x0 );
    
    //Rect return_value ( 2 );
    Prism P ( 2 );
    try{
      /* Compute forward image */
      //rect.move(*T);
      
      /* Write output */
      IVector w = (*timemap) ( timestep, rect );
      
      
      IMatrix B = rect . get_B ();
      IVector r = rect . get_r ();
      IVector x = rect . get_x ();
      // x + Br
      IMatrix D ( 2, 2 );
      for ( int d = 0; d < 2; ++ d ) {
        D (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),  
                               abs ( r [ d ] . leftBound () ) ) ;
      }
      B = B * D;
                              
      for ( int d = 0; d < 2; ++ d ) P . c ( d ) = (x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
      for ( int i = 0; i < 2; ++ i ){
        for ( int j = 0; j < 2; ++ j ) {
          P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
        }
      }
      
      // B r = B D Dinv r = B D e    where e = [-1,1]^3
      // D e = r
  
       

      //std::cout << rectangle << " --> " << w << "\n";
      //std::cout << P . A << " , " << P . c << "\n";
      //std::cout << abs ( 2.3) << "\n";
      //std::cout << "capd rect: " << rect << "\n";
      /*
      return_value . lower_bounds [ 0 ] = w[ 0 ] . leftBound();
      return_value . upper_bounds [ 0 ] = w[ 0 ] . rightBound();
      return_value . lower_bounds [ 1 ] = w[ 1 ] . leftBound();
      return_value . upper_bounds [ 1 ] = w[ 1 ] . rightBound();
      */
       
    } catch (std::exception& e) {	
      
      /*
    	return_value . lower_bounds [ 0 ] = -5.0;
      return_value . upper_bounds [ 0 ] = 5.0;
      return_value . lower_bounds [ 1 ] = -8.0;
      return_value . upper_bounds [ 1 ] = 8.0;
       
      //std::cout << rectangle << " - :( -> " << return_value << "\n";
*/
      for ( int d = 0; d < 2; ++ d ) P . A ( d, d ) = 100.0;
      
    }
    return P;//return_value;
  }
  
};

#endif
