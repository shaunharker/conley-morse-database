/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "database/maps/Map.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/simple_interval.h"
#include <memory>
#include <vector>

struct ModelMap : public Map {
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  ModelMap ( std::shared_ptr<Parameter> parameter ) {
    const RectGeo & rectangle = 
      * std::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    parameter1 = interval (rectangle . lower_bounds [ 0 ], 
                           rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], 
                           rectangle . upper_bounds [ 1 ]);
    return;
  }

  std::shared_ptr<Geo> 
  operator () ( std::shared_ptr<Geo> geo ) const {   
    return std::shared_ptr<Geo> ( new RectGeo ( 
        operator () ( * std::dynamic_pointer_cast<RectGeo> ( geo ) ) ) );
  }

  RectGeo operator () 
    ( const RectGeo & rectangle ) const {    
    /* Read input */
    //std::cout << "ModelMap::operator(). rectangle = " << rectangle << "\n";
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
                            rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * 
                   exp ( (double) -0.1 * (x0 + x1) );     
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    double tol = 1e-8;
    RectGeo return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower () - tol;
    return_value . upper_bounds [ 0 ] = y0 . upper () + tol;
    return_value . lower_bounds [ 1 ] = y1 . lower () - tol;
    return_value . upper_bounds [ 1 ] = y1 . upper () + tol;
    return return_value;
  } 
};

#endif
