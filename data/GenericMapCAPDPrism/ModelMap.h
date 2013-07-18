/* 

Generic Map File 

This file is fully commented and can serve as a basis for more complicated map.
For illustration, we implement the 2D Leslie Map, using CAPD and Prism

*/

#ifndef MODELMAP_H
#define MODELMAP_H

#undef None
#include "capd/capdlib.h"
#include "capd/dynsys/DynSysMap.h"
#include "capd/dynset/C0PpedSet.h"

#include "chomp/Rect.h"
#include "chomp/Prism.h"

#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  typedef chomp::Prism Prism;
  typedef capd::intervals::Interval<double> interval;


  std::vector < interval > parameter;

  capd::IMap f;
  capd::dynsys::DynSysMap<capd::IMap> * map;

  // constructor
  ModelMap ( const Rect & rectangle ) {

    using namespace capd;

    // Retreive parameters intervals 
    for ( unsigned int i=0; i<rectangle.dimension(); ++i ) {
      parameter . push_back ( interval (rectangle . lower_bounds [ i ], rectangle . upper_bounds [ i ]) );
    }

    //=============================================================================
    // Define the map 
    // Syntax :  
    // "par:list_of_parameters;var:list_of_space_variables;fun:list_of_equations;";
    // list separator is a comma
    f = "par:a,b;var:x,y;fun:(a*x+b*y)*exp(-0.1*(x+y)),0.7*x;";
    
    // Set the parameters 
    f . setParameter ( "a", parameter [ 0 ] );
    f . setParameter ( "b", parameter [ 1 ] );

    //=============================================================================

    map = new capd::dynsys::DynSysMap<capd::IMap> ( f );

    return;
  }


  Prism ppedMethod ( const Rect & rectangle ) const {
    using namespace capd;
    
    // Put input into IVector structure "x0"
    int D = rectangle.dimension();
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


  Prism operator ()
    ( const Rect & rectangle ) const { 

    return ppedMethod ( rectangle );

    }

};

#endif
