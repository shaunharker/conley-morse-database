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
  
  ModelMap ( const Rect & rectangle ) {
    ModelMap ( const Rect & rectangle ) {
      using namespace capd;
      f = "par:a,b;var:x,y;fun:(a*x+b*y)*exp(-(x+y)/10),7*x/10;";
      f.setParameter("a", interval(rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] ) );
      
      f.setParameter("b", interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]) );
      map = new capd::dynsys::DynSysMap<capd::IMap> ( f );
      return;
    }
  std::vector < chomp::Prism > operator ()
  ( const chomp::Rect & rectangle ) const {
    /* Read input */
    using namespace capd;
    
    std::vector < chomp::Prism > results;
    
    // Put input into IVector structure "x0"
    IVector x0 ( 2 );
    for ( int d = 0; d < 2; ++ d ) {
      x0 [ d ] = interval ( rectangle . lower_bounds [ d ],
                           rectangle . upper_bounds [ d ] );
    }
    
    
    
    // Determine a direction to split
    // Step 1. Compute Center and Radius of Jacobian
    IMatrix J = map[x0]; // Jacobian
    IMatrix JCenter ( 2, 2 );
    IMatrix JRadius ( 2, 2 );
    capd::vectalg::split ( J, JCenter, JRadius )
    
    IMatrix Jinv = capd::matrixAlgorithms::gaussInverseMatrix ( JCenter );
    //Jinv = capd::vectalg::abs ( Jinv );
    for ( int d1 = 0; d1 < 2; ++ d1 ) {
      for ( int d2 = 0; d2 < 2; ++ d2 ) {
        Jinv ( d1 + 1, d2 + 1 ) = capd::intervals::abs ( Jinv ( d1 + 1, d2 + 1 ) );
      }
    }
    
    JRadius = capd::vectalg::rightObject<IMatrix,IMatrix>( JRadius );
    
    IVector ones ( 2 );
    for ( int d = 0; d < 2; ++ d ) {
      ones [ d ] = 1.0;
    }
    IVector V = Jinv * ones;
    
    JRadius . transpose ();
    IVector result = JRadius * V;
    
    double best = 0;
    int choice = 0;
    for ( int d = 0; d < 2; ++ d ) {
      if ( result [ d ] . rightBound () > best ) {
        choice = d;
        best = result [ d ] . rightBound ();
      }
    }
    
    // Produce split boxes
    IVector box1 = x0;
    IVector box2 = x0;
    
    box1 [ choice ] =
    interval ( box1 [ choice ] . leftBound (), box1 [ choice ] . mid () . rightBound () );
    
    box2 [ choice ] =
    interval ( box2 [ choice ] . mid () . leftBound (), box2 [ choice ] . rightBound () );
    
    std::stack < IVector > workstack;
    workstack . push ( box1 );
    workstack . push ( box2 );
    
    // C0RectSet
    while ( not workstack . empty () ) {
      
      IVector box = workstack . top ();
      workstack . pop ();
      capd::dynset::C0PpedSet<IMatrix> rect ( box );
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
        results . push_back ( P );
        
        }
        return results;
        
        } 
        
        };
        
#endif
