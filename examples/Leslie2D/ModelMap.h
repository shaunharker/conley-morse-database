/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

#include "database/maps/Map.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/simple_interval.h"
#include <boost/shared_ptr.hpp>
#include <vector>

class ModelMap : public Map {
public:
  typedef simple_interval<double> interval;

// User interface: method to be provided by user
  // Parameter variables
  interval p0, p1;
  
  // Constructor: sets parameter variables
  void assign ( RectGeo const& rectangle ) {
    // Read parameter intervals from input rectangle
    p0 = getRectangleComponent ( rectangle, 0 );
    p1 = getRectangleComponent ( rectangle, 1 );  
  }

  // Map
  RectGeo operator () ( const RectGeo & rectangle ) const {    
    // Convert input to intervals
    interval x0 = getRectangleComponent ( rectangle, 0 );
    interval x1 = getRectangleComponent ( rectangle, 1 );

    // Evaluate map
    interval y0 = (p0 * x0 + p1 * x1 ) * exp ( -0.1 * (x0 + x1) );     
    interval y1 = 0.7 * x0;
    
    // Return result
    return makeRectangle ( y0, y1 );
  } 

// Program interface (methods used by program)

  ModelMap ( boost::shared_ptr<Parameter> parameter ) {
    const RectGeo & rectangle = 
      * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    assign ( rectangle );
  }

  boost::shared_ptr<Geo> 
  operator () ( boost::shared_ptr<Geo> geo ) const {   
    return boost::shared_ptr<Geo> ( new RectGeo ( 
        operator () ( * boost::dynamic_pointer_cast<RectGeo> ( geo ) ) ) );
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
