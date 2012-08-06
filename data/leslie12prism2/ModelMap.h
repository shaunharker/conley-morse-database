/* Leslie Map */

#ifndef CMDP_LESLIEMAPP2_H
#define CMDP_LESLIEMAPP2_H

#include "capd/capdlib.h"
#include "capd/dynsys/DynSysMap.h"
#include "capd/dynset/C0PpedSet.hpp"

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "database/maps/simple_interval.h"
#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  capd::IMap f;
  capd::dynsys::DynSysMap<capd::IMap> * map;
  
  ModelMap ( const Rect & rectangle ) {
       f = "par:a,b;var:x,y;fun:(a*x+b*y)*exp(-(x+y)/10),7*x/10;";
    f.setParameter("a", interval(rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] ) );
    
    f.setParameter("b", interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]) ); 
    map = new capd::dynsys::DynSysMap<capd::IMap> ( f );
    return;
  }
  chomp::Prism operator () 
  ( const chomp::Rect & rectangle ) const {    
    /* Read input */
    using namespace capd;
    IVector x0 ( 2 );
    for ( int d = 0; d < 2; ++ d ) {
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ], 
                           rectangle . upper_bounds [ d ] );
    }
    // C0RectSet
    capd::dynset::C0PpedSet<IMatrix> rect ( x0 );
    
    /* Perform map computation */
    Prism P ( 2 );

    try {
      rect.move(*map);
      IMatrix B = rect . get_B ();
      IVector r = rect . get_r ();
      IVector x = rect . get_x ();
      // x + Br
      IMatrix D ( 2, 2 );
      for ( int d = 0; d < 2; ++ d ) {
        D (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),  
                                 abs ( r [ d ] . leftBound () ) );
      }
      B = B * D;
      
      for ( int d = 0; d < 2; ++ d ) P . c ( d ) = ( x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
      for ( int i = 0; i < 2; ++ i ){
        for ( int j = 0; j < 2; ++ j ) {
          P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
        }
      }
      
    } catch (std::exception& e) {
      
      std::cout << "Threw exception" << e.what() << "\n" ;
      abort ();
    }  
    return P;
    
  } 
  
};

#endif
