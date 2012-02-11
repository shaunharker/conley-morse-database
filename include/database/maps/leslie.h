/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

//#include <boost/numeric/interval.hpp>
#include "chomp/Toplex.h"
#include "database/maps/simple_interval.h"
#include <vector>

struct LeslieMap {
  
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  LeslieMap ( const Rect & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () 
    ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * exp ( (double) -0.1 * (x0 + x1) );     
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    Rect return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } 
  
};

struct LeslieFishMap {
  typedef simple_interval<double> interval;
  interval parameter1, parameter2;
  std::vector < double > coefficients;
  LeslieFishMap ( const Rect & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    coefficients . resize ( 10, (double) 0 );
    coefficients [ 0 ] = (double) 0;
    coefficients [ 1 ] = (double) 0;
    coefficients [ 2 ] = (double) .026;
    coefficients [ 3 ] = (double) .046;
    // justin, add more
    return;
  }
  Rect operator () 
    (const Rect & rectangle ) const {    
      /* Read input */
      int dim = rectangle . lower_bounds . size ();
      std::vector < interval > input ( dim );
      std::vector < interval > output ( dim );

      for ( int i = 0; i < dim; ++ i ) {
        input [ i ] = interval (rectangle . lower_bounds [ i ], rectangle . upper_bounds [ i ]);
      }
      
      /* Perform map computation */
      const double p = 0.60653;
      interval z ( (double) 0, (double) 0 );
      for ( int i = 0; i < dim; ++ i ) z = z + coefficients [ i ] * input [ i ];
      output [ 0 ] = parameter1 * z * exp ( (double) -0.1 * z); 
      for ( int i = 1; i < dim; ++ i ) output [ i ] = p * input [ i - 1 ];
  
      /* Write output */
      Rect return_value ( dim );
      for ( int i = 0; i < dim; ++ i ) {
        return_value . lower_bounds [ i ] = output [ i ] . lower ();
        return_value . upper_bounds [ i ] = output [ i ] . upper ();
      }
      
      //std::cout << "f(" << rectangle << ") = " << return_value << "\n";
      return return_value;
  } 
};

#endif
