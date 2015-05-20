/* Model Map */
// Author: Aniket Shah

#ifndef CMDP_MODELMAP_H
#define CMDP_MODELMAP_H

#include "database/maps/Map.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/boost_interval.h"
#include <memory>
#include <utility>
#include <vector>

#include "IAS_MapHelper.h"

class ModelMap : public Map {
public:

// User interface: method to be provided by user
  // Parameter variables
  interval p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;
  
  // Constructor: sets parameter variables
  void assign ( RectGeo const& rectangle ) {
    // Read parameter intervals from input rectangle
    p0 = getRectangleComponent ( rectangle, 0 ); //a
    p1 = getRectangleComponent ( rectangle, 1 ); //b
    p2 = getRectangleComponent ( rectangle, 2 ); //c
    p3 = getRectangleComponent ( rectangle, 3 ); //d
    p4 = getRectangleComponent ( rectangle, 4 ); //e
    p5 = getRectangleComponent ( rectangle, 5 ); //f
    p6 = getRectangleComponent ( rectangle, 6 ); //a'
    p7 = getRectangleComponent ( rectangle, 7 ); //b'
    p8 = getRectangleComponent ( rectangle, 8 ); //c'
    p9 = getRectangleComponent ( rectangle, 9 ); //d'
  }

  RectGeo badbox ( void ) const {
    return makeRectangle ( interval(10.0,10.0), interval(10.0,10.0) );
  }

  // Map
  UnionGeo operator () ( const RectGeo & rectangle ) const {    
    UnionGeo result;
    // Convert input to intervals
    interval x0 = getRectangleComponent ( rectangle, 0 );
    interval x1 = getRectangleComponent ( rectangle, 1 );

    // Check if "badbox"
    if ( x0 . upper () == 10.0 && x1 . upper () == 10.0 ) {
      result . insert ( std::shared_ptr<Geo> ( new RectGeo ( badbox () ) ) ) ;
      return result;      
    }
    
    // Check if not feasible
    if ( x0 . lower ()  + x1 . lower () > 10.0 ) {
      return result;
    }

    // Evaluate map
    interval x2 = 10.0 - x0 - x1;
    interval coeffA = x0*(1.0+p1/(p0-p2)+p3/(p0-p5)+(p4/(p0-p5))*(p1/(p0-p2)));
    interval coeffB = x1-x0*(p1/(p0-p2))*(1.0+p4/(p2-p5));
    interval coeffC = x2-x0*((p4/(p0-p5))*(p2/(p0-p2))+(p3/(p0-p5)))-(p4/(p2-p5))*(x1-x0*(p1/(p0-p2)));
    
    std::pair<interval,int> stopping_time = expMinRoot(p0,p2,p5,coeffA,coeffB,coeffC,-4.0,0.0);
    int const& return_code = stopping_time . second;
    // return_code: 0 means intersection surely, 
    //              1 means no intersectly surely
    //              2 means possible intersection
    if ( return_code == 1 ) {
      result . insert ( std::shared_ptr<Geo> ( new RectGeo ( badbox () ) ) );
      return result;
    } else if (return_code == 2) {
      result . insert ( std::shared_ptr<Geo> ( new RectGeo ( badbox () ) ) );
    }
    interval PSA4x0 = x0*exp(p0*stopping_time.first);
    interval PSA4x1 = x0*(p1/(p0-p2))*exp(p0*stopping_time.first)+(x1-x0*(p1/(p0-p2)))*exp(p2*stopping_time.first);
    interval PSA4x2 = 4.0 - PSA4x0 - PSA4x1;
    interval coeffAprime = PSA4x0-PSA4x1*(p7/(p8-p6));
    interval coeffBprime = PSA4x1*(1.0+p7/(p8-p6));
    interval coeffCprime = PSA4x2;
    stopping_time = expMinRoot(p6,p8,p9,coeffAprime,coeffBprime,coeffCprime,-10.0,0.0);
    if ( return_code == 1 ) {
      result . insert ( std::shared_ptr<Geo> ( new RectGeo ( badbox () ) ) );
      return result;
    } else if (return_code == 2) {
      result . insert ( std::shared_ptr<Geo> ( new RectGeo ( badbox () ) ) );
    }
    interval y1 = PSA4x1*exp(p8*stopping_time.first);
    interval y0 = 10.0 - y1 - PSA4x2*exp(p9*stopping_time.first);
    
    // Return result
    std::shared_ptr<Geo> rect_result ( new RectGeo ( makeRectangle ( y0, y1 ) ) );
    result . insert ( rect_result );
    return result;

  } 

// Program interface (methods used by program)

  ModelMap ( std::shared_ptr<Parameter> parameter ) {
    const RectGeo & rectangle = 
      * std::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    assign ( rectangle );
  }

  std::shared_ptr<Geo> 
  operator () ( std::shared_ptr<Geo> geo ) const {   
    return std::shared_ptr<Geo> ( new UnionGeo ( 
        operator () ( * std::dynamic_pointer_cast<RectGeo> ( geo ) ) ) );
  }

private:
  interval getRectangleComponent ( const RectGeo & rectangle, int d ) const {
    return interval (rectangle . lower_bounds [ d ], rectangle . upper_bounds [ d ]); 
  }
  RectGeo makeRectangle ( interval const& y0, interval const& y1 ) const {
    RectGeo return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  }
};

#endif
