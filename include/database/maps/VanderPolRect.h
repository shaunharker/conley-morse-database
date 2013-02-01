/* Integrator for Database using CAPD */

#ifndef CMDP_INTEGRATOR_H
#define CMDP_INTEGRATOR_H

//#include "capd/krak/krak.h"
#include "capd/capdlib.h"


#include "chomp/Rect.h"

#include <cmath>
#include <stdexcept>
#include <fstream>

class VanderPolRect {
public:
  capd::IMap f;
  double step;
  capd::interval timestep;
  int order;
  capd::ITaylor * T;
  capd::ITimeMap * timemap;
  int dim_;
  
  VanderPolRect ( const chomp::Rect & rectangle ) {
    using namespace capd;
    dim_ = 2;
    f = "par:c;var:x,y;fun:y,-x+c*(1-x*x)*y;";
    f . setParameter ( "c", interval(rectangle.lower_bounds[0],
                                     rectangle.upper_bounds[0]));
    timestep = interval(1.0)/10.0;
    step = .001;
    order = 3;
    //f.setParameter("c",interval(1.0));
    //f.setParameter("c", parameter1 );
    
    T = new ITaylor (f,order,step);
    timemap = new ITimeMap ( *T );
    return;
  }
  
  ~VanderPolRect ( void ) {
    delete T;
    delete timemap;
  }
  
  chomp::Prism operator () ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    /* Read input */
    IVector x0 ( dim_ );
    for ( int d = 0; d < dim_; ++ d )
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ],
                           rectangle . upper_bounds [ d ] );
      C0RectSet rect ( x0 );
      
      //Rect return_value ( dim_ );
      Prism P ( dim_ );
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
        IMatrix D ( dim_, dim_ );
        for ( int d = 0; d < dim_; ++ d ) {
          D (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),
                                   abs ( r [ d ] . leftBound () ) );
        }
        B = B * D;
        
        for ( int d = 0; d < dim_; ++ d ) P . c ( d ) = (x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
        for ( int i = 0; i < dim_; ++ i ){
          for ( int j = 0; j < dim_; ++ j ) {
            P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
          }
        }
      }
    catch (std::exception& e)
    {
      std::cout << "Exception thrown.\n";
      for ( int d = 0; d < dim_; ++ d ) P . A ( d, d ) = 100.0;
      
    }
    return P;//return_value;
  }
  
};

#endif
