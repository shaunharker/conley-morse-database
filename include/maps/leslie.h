/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

#include <boost/numeric/interval.hpp>
#include "toplexes/Adaptive_Cubical_Toplex.h"


struct LeslieMap {

  //using namespace boost;
  //using namespace numeric;
  typedef boost::numeric::interval<double, boost::numeric::interval_lib::
  policies<
    boost::numeric::interval_lib::save_state<
    boost::numeric::interval_lib::rounded_transc_opp<double> >,
    boost::numeric::interval_lib::checking_base<double> > > interval;
  
  interval parameter1, parameter2;

  LeslieMap ( const Adaptive_Cubical::Geometric_Description & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    return;
  }
  Adaptive_Cubical::Geometric_Description operator () (
    const Adaptive_Cubical::Geometric_Description & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], 
			    rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], 
                            rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * x0 + parameter2 * x1 ) * 
      exp ( (double) -0.1 * (x0 + x1) );
    interval y1 = (double) 0.7 * x0;
    
    /* Write output */
    Adaptive_Cubical::Geometric_Description return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } /* MapPopModel::compute */

};

#endif
