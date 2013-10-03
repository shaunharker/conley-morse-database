/* Integrator for Database using CAPD */

#ifndef CMDB_ODE_MAP_H
#define CMDB_ODE_MAP_H

//#include "capd/krak/krak.h"
#include "capd/capdlib.h"

#include "chomp/Rect.h"
//#include "database/structures/RectGeo.h"

#include <stdexcept>
#include <memory>
#include <boost/shared_ptr.hpp>

class OdeMap {
public:
  
  typedef chomp::Rect Rect;
  typedef Rect image_type;
  
  int dim;
  mutable int order;
  mutable double integrationTime;
  
  mutable boost::shared_ptr<capd::IMap> f;
  mutable boost::shared_ptr< capd::ITaylor > solver;
  mutable boost::shared_ptr< capd::ITimeMap> timeMap;
  
  mutable bool exception_thrown;
  
  OdeMap ( void ) {
  }
  
  bool & exception ( void ) const { return exception_thrown; }

  Rect intervalMethod ( const Rect & rectangle ) const {
    using namespace capd;
    
    // Put input into IVector structure "c"
    IVector c ( dim );
    for ( int d = 0; d < dim; ++ d ) {
      c [ d ] = interval (rectangle . lower_bounds [ d ],
                          rectangle . upper_bounds [ d ] );
    }
    // Use integrator
    capd::C0Rect2Set s(c);
    
    IVector mapped = (*timeMap)(integrationTime, s);
    
    // Return result
    Rect result ( dim );
    for ( int d = 0; d < dim; ++ d ) {
      result . lower_bounds [ d ] = mapped[d].leftBound();
      result . upper_bounds [ d ] = mapped[d].rightBound();
    }
    return result;
  }


  Rect operator () ( const Rect & rectangle ) const {
    static size_t exception_count = 0;
    Rect result ( dim );
    try {
      result = intervalMethod ( rectangle );
    } catch (std::exception& e) {
      exception_thrown = true;
      ++ exception_count;
      //std::cout << "Exception.\n";
      //abort ();
      for ( int d = 0; d < dim; ++ d ) {
        result . lower_bounds [ d ] = -1e300;
        result . upper_bounds [ d ] = 1e300;
      }
      
      if ( exception_count%100 == 0 ) {
        std::cout << e . what () << "\n";
       // throw e;
      }   
    }
    return result;
  }
 
  bool good ( void ) const { return true; }
};

#endif
