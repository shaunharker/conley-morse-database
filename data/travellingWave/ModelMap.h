#ifndef CMDP_TRAVELWAVE_H
#define CMDP_TRAVELWAVE_H

#include "capd/capdlib.h"
//#include "capd/dynsys/DynSysMap.h"
//#include "capd/dynset/C0PpedSet.hpp"
//#include "capd/dynset/C0RectSet.hpp"
#include "capd/dynset/lib.h"
//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
//#include "database/maps/simple_interval.h"
#include "capd/intervals/lib.h"


#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  capd::IMap f;
  
  // ODES
  capd::interval step;
  int order;
  int num_steps;
  capd::ITaylor * T;
  capd::ITimeMap * timemap;
  
  int D;
  ModelMap ( const Rect & rectangle ) {
    using namespace capd;
    D = 4;
    f = "par:pa,pb,pc,pd;var:u,w,v,z;fun:w,((-pc/pd)*w)-((1/pd)*u*(1-u)*(u-v)),z,(-pc*z)-(v*((pa*u)-(pb+v)));";
    f.setParameter("pa", interval(rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] ) );
    f.setParameter("pb", interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]) );
    f.setParameter("pc", interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]) );
    f.setParameter("pd", interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]) );
    
    
    step= interval(1.0/128.0,1.0/128.0);
    order = 3;
    num_steps = 16;
    

    T = new ITaylor (f,order,step);
    timemap = new ITimeMap ( *T );
    
    return;
  }
  
  
  
  chomp::Rect intervalMethod ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    // Put input into IVector structure "x0"
    IVector box ( D );
    for ( int d = 0; d < D; ++ d ) {
      box [ d ] = interval ( rectangle . lower_bounds [ d ],
                            rectangle . upper_bounds [ d ] );
    }
    // INTERVAL METHOD CALCULATION
    IVector imethod = f ( box );
    chomp::Rect result ( D );
    for ( int d = 0; d < D; ++ d ) {
      result . lower_bounds [ d ] = imethod [ d ] . leftBound ();
      result . upper_bounds [ d ] = imethod [ d ] . rightBound ();
    }
    return result;
  }
  
  chomp::Prism ppedMethod ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    // Put input into IVector structure "x0"
    IVector box ( D );
    for ( int d = 0; d < D; ++ d ) {
      box [ d ] = interval ( rectangle . lower_bounds [ d ],
                            rectangle . upper_bounds [ d ] );
    }
    //capd::dynset::C0PpedSet<IMatrix> rect ( box );
    capd::dynset::C0Rect2Set<IMatrix> rect ( box );

    /* Perform map computation */
    Prism P ( D );
    try {
      
      for (int i =0; i< num_steps; ++i ) {
        rect.move(*T);
      }
      
      IMatrix B = rect . get_B ();
      IVector r = rect . get_r ();
      IVector x = rect . get_x ();
      // x + Br
      IMatrix Diag ( D, D );
      for ( int d = 0; d < D; ++ d ) {
        Diag (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),
                                    abs ( r [ d ] . leftBound () ) );
      }
      B = B * Diag;
      
      for ( int d = 0; d < D; ++ d ) P . c ( d ) = ( x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
      for ( int i = 0; i < D; ++ i ){
        for ( int j = 0; j < D; ++ j ) {
          P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
        }
      }
      
    } catch (std::exception& e) {
      
      std::cout << "Threw exception" << e.what() << "\n" ;
      abort ();
    }
    return P;
  }
  
    
  // OPERATOR () FUNCTION
  
  //std::pair < chomp::Rect, chomp::Prism > operator ()
  //( const chomp::Rect & rectangle ) const {
  //  return std::make_pair ( intervalMethod ( rectangle ), ppedMethod ( rectangle ) );
  //}

  chomp::Prism  operator ()
  ( const chomp::Rect & rectangle ) const {
    return ppedMethod ( rectangle );
  }
  
  bool good ( void ) const { return true; }
  
};


#endif
