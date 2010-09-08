/* Leslie Map */

#ifndef CMDP_LESLIEMAP_H
#define CMDP_LESLIEMAP_H

struct LeslieMap {

  //using namespace boost;
  //using namespace numeric;
  typedef boost::numeric::interval<double, boost::numeric::interval_lib::
  policies<
    boost::numeric::interval_lib::save_state<
    boost::numeric::interval_lib::rounded_transc_opp<double> >,
    boost::numeric::interval_lib::checking_base<double> > > interval;
  
  interval parameter1, parameter2;
  LeslieMap ( int b1, int b2 ) {
    parameter1 = interval (8.0 + ( 37.0 - 8.0 ) / 50.0 * (double) b1,
                           8.0 + ( 37.0 - 8.0 ) / 50.0 * (double) ( b1 + 1 ) );
    parameter2 = interval (3.0 + ( 50.0 - 3.0 ) / 50.0 * (double) b2,
                           3.0 + ( 50.0 - 3.0 ) / 50.0 * (double) ( b2 + 1 ) ); 
    std::cout << "P1 = " << parameter1 . lower () << ", " << parameter1 . upper () << 
      "\n and P2 = " << parameter2 . lower () << ", " << parameter2 . upper () << 
      "\n";
  }
  
  Geometric_Description operator () ( const Geometric_Description & rectangle ) const {    
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
    Geometric_Description return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } /* MapPopModel::compute */

};

#endif
