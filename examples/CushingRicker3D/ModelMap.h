//  CushingRicker3D Map


#ifndef MODELMAP_H
#define MODELMAP_H

#include "database/maps/Map.h"
#include "database/structures/RectGeo.h"
#include "boost/shared_ptr.hpp"

#include <vector>

#ifdef USE_BOOST_INTERVAL
#include "database/numerics/boost_interval.h"
#endif

#ifdef USE_CAPD
#undef None
#include "capd/capdlib.h"
using namespace capd;
#endif

#ifndef USE_BOOST_INTERVAL
#ifndef USE_CAPD
#include "database/numerics/simple_interval.h"
#endif
#endif


struct ModelMap : public Map {
#ifdef USE_CAPD
typedef capd::intervals::Interval<double> interval;
#endif  
#ifndef USE_BOOST_INTERVAL
#ifndef USE_CAPD
  typedef simple_interval<double> interval;
#endif
#endif  
  std::vector < interval > parameter;

  // constructor
  ModelMap ( boost::shared_ptr<Parameter> parameter ) {
    const RectGeo & rectangle = 
      * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    parameter . resize ( rectangle . dimension () );
    for ( unsigned int i=0; i<rectangle.dimension(); ++i ) 
      parameter [ i ] = interval (rectangle . lower_bounds [ i ], 
                                  rectangle . upper_bounds [ i ]);
    return;
  }

  boost::shared_ptr<Geo> 
  operator () ( boost::shared_ptr<Geo> geo ) const {   
    return boost::shared_ptr<Geo> ( new RectGeo ( 
        operator () ( * boost::dynamic_pointer_cast<RectGeo> ( geo ) ) ) );
  }

  RectGeo operator () 
    ( const RectGeo & rectangle ) const {    

    std::vector < interval > x; 
    std::vector < interval > y;
    
    x . resize ( rectangle . dimension ( ) );
    y . resize ( rectangle . dimension ( ) );

    RectGeo return_value ( rectangle . dimension ( ) );
    
    for ( unsigned int i=0; i<rectangle.dimension(); ++i ) 
      x [ i ] = interval (rectangle . lower_bounds [ i ], rectangle . upper_bounds [ i ]);

    /********************************************************************* 
      Define the map in terms of the phase space variables and parameters. 
    *********************************************************************/
    
    // parameters : b1, b2, c12, c13, c31, c33, rho
    // y0 = b1 y2 exp( - (c12*y2+c13*y3) )
    // y1 = rho y1
    // y2 = b2 y3 exp( - (c31*y1+c33*y3) )

    y [ 0 ] = parameter[0] * x[1] * exp( -1.0* ( parameter[2]*x[1]+parameter[3]*x[2] ) );

    y [ 1 ] = parameter[6] * x[0];

    y [ 2 ] = parameter[1] * x[2] * exp( -1.0* ( parameter[4]*x[0]+parameter[5]*x[2] ) );
    
    /*********************************************************************  
    *********************************************************************/

#ifdef USE_BOOST_INTERVAL
    for ( unsigned int i=0; i<rectangle.dimension(); ++i ) {
      return_value . lower_bounds [ i ] = y [ i ] . lower ( );
      return_value . upper_bounds [ i ] = y [ i ] . upper ( );
    }
#endif

#ifdef USE_CAPD
    for ( unsigned int i=0; i<rectangle.dimension(); ++i ) {
      return_value . lower_bounds [ i ] = y [ i ] . leftBound ( );
      return_value . upper_bounds [ i ] = y [ i ] . rightBound ( );
    }
#endif
#ifndef USE_BOOST_INTERVAL
#ifndef USE_CAPD
  for ( unsigned int i=0; i<rectangle.dimension(); ++i ) {
    return_value . lower_bounds [ i ] = y [ i ] . lower ( );
    return_value . upper_bounds [ i ] = y [ i ] . upper ( );
  }
#endif
#endif  
    return return_value;
  } 
};

#endif
