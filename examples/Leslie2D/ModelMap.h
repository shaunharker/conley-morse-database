/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/simple_interval.h"
#include <boost/shared_ptr.hpp>
#include <vector>

struct ModelMap {
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  ModelMap ( boost::shared_ptr<Parameter> parameter ) {
    RectGeo rectangle = * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  RectGeo operator () ( const boost::shared_ptr<Geo> & geo ) const {   
    return operator () ( * boost::dynamic_pointer_cast<RectGeo> ( geo ) );
  }

  RectGeo operator () 
    ( const RectGeo & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * exp ( (double) -0.1 * (x0 + x1) );     
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
  bool good ( void ) const { return true; }
};

#endif
