/* 

Generic Map File 

This file can serve as a basis for more complicated map.
For illustration, we implement the 2D Leslie Map using the CAPD package

*/

#ifndef MODELMAP_H
#define MODELMAP_H

#undef None
#undef MAX
#undef MIN
#include "capd/capdlib.h"
#include "capd/dynsys/DynSysMap.h"
#include "chomp/Rect.h"

#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  typedef capd::interval interval;


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

  Rect operator () 
    ( const Rect & rectangle ) const {    
    using namespace capd;


    IVector box ( rectangle.dimension() );
    for ( unsigned int d = 0; d < rectangle.dimension(); ++d ) {
      box [ d ] = interval ( rectangle . lower_bounds [ d ], 
                            rectangle . upper_bounds [ d ] );
    }

    IVector imagebox = f ( box ) ;

    Rect result ( rectangle . dimension() );

    for ( unsigned int i = 0; i < rectangle.dimension(); ++i ) {
      result . lower_bounds [ i ] = imagebox [ i ] . leftBound ( );
      result . upper_bounds [ i ] = imagebox [ i ] . rightBound ( );
    }

    return result;
  }
  bool good ( void ) const { 
    return true;
  }
};

#endif
