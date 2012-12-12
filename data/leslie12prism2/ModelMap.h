/* Leslie Map */

#ifndef CMDP_LESLIEMAPP2_H
#define CMDP_LESLIEMAPP2_H

#include "capd/capdlib.h"
#include "capd/dynsys/DynSysMap.h"
#include "capd/dynset/C0PpedSet.hpp"

//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
//#include "database/maps/simple_interval.h"
#include "capd/intervals/lib.h"


#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  capd::IMap f;
  capd::dynsys::DynSysMap<capd::IMap> * map;
  int D;
  ModelMap ( const Rect & rectangle ) {
    using namespace capd;
    D = 2;
    f = "par:a,b;var:x,y;fun:(a*x+b*y)*exp(-(x+y)/10),7*x/10;";
    f.setParameter("a", interval(rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] ) );
    
    f.setParameter("b", interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]) );
    map = new capd::dynsys::DynSysMap<capd::IMap> ( f );
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
    capd::dynset::C0PpedSet<IMatrix> rect ( box );
    /* Perform map computation */
    Prism P ( D );
    try {
      rect.move(*map);
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
  
  std::vector < chomp::Rect > presubdivision ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    std::vector < chomp::Rect > result;
    // Put input into IVector structure "x0"
    IVector x0 ( D );
    for ( int d = 0; d < D; ++ d ) {
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ],
                           rectangle . upper_bounds [ d ] );
    }
    
    std::vector < IVector > workstack;
    // Determine a direction to split
    // Step 1. Compute Center and Radius of Jacobian
    IMatrix J = (*map) [x0]; // Jacobian
    IMatrix Diag ( D, D );
    for ( int d = 0; d < D; ++ d ) {
      Diag ( d + 1, d + 1 ) = x0 [d] . rightBound () - x0[d] . leftBound ();
    }
    //J = J * Diag;
    IMatrix JCenter ( D, D );
    IMatrix JRadius ( D, D );
    capd::vectalg::split ( J, JCenter, JRadius );
    
    JCenter = JCenter * Diag;
    IMatrix Jinv = capd::matrixAlgorithms::gaussInverseMatrix ( JCenter );
    //Jinv = capd::vectalg::abs ( Jinv );
    for ( int d1 = 0; d1 < D; ++ d1 ) {
      for ( int d2 = 0; d2 < D; ++ d2 ) {
        Jinv ( d1 + 1, d2 + 1 ) =
        capd::abs ( Jinv ( d1 + 1, d2 + 1 ) ) ;
      }
    }
    
    JRadius = capd::vectalg::rightObject<IMatrix,IMatrix>( JRadius );
    
    IVector ones ( D );
    for ( int d = 0; d < D; ++ d ) {
      ones [ d ] = 1.0;
    }
    IVector V = Jinv * ones;
    
    JRadius . transpose ();
    IVector IVresult = JRadius * V;
    
    double best = 0;
    int choice = 0;
    for ( int d = 0; d < D; ++ d ) {
      if ( IVresult [ d ] . rightBound () > best ) {
        choice = d;
        best = IVresult [ d ] . rightBound ();
      }
    }
    
    // Produce split boxes
    IVector box1 = x0;
    IVector box2 = x0;
    
    box1 [ choice ] =
    interval ( box1 [ choice ] . leftBound (), box1 [ choice ] . mid () . rightBound () );
    
    box2 [ choice ] =
    interval ( box2 [ choice ] . mid () . leftBound (), box2 [ choice ] . rightBound () );
    
    workstack . push_back ( box1 );
    workstack . push_back ( box2 );
    
    BOOST_FOREACH ( const IVector & iv, workstack ) {
      chomp::Rect item ( D );
      for ( int d = 0; d < D; ++ d ) {
        item . lower_bounds [ d ] = iv [ d ] . leftBound ();
        item . upper_bounds [ d ] = iv [ d ] . rightBound ();
      }
      result . push_back ( item );
    }
    return result;
  }
  
  // OPERATOR () FUNCTION
  std::pair < chomp::Rect, chomp::Prism > operator ()
  ( const chomp::Rect & rectangle ) const {
    return std::make_pair ( intervalMethod ( rectangle ), ppedMethod ( rectangle ) );
  }

  //chomp::Prism  operator ()
  //( const chomp::Rect & rectangle ) const {
  //  return ppedMethod ( rectangle );
  //}
  
  bool good ( void ) const { return true; }
  
};


#endif
