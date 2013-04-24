
/********************/
/* Standard Headers */
/********************/
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <set>
#include <map>

/***************************/
/* Preprocessor directives */
/***************************/

#define CMG_VERBOSE
//#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/Rect.h"

#include "database/tools/SingleOutput.h"


/*************************/
/* Subdivision Settings  */
/*************************/

#ifndef GRIDCHOICE
#define GRIDCHOICE PointerGrid
#endif

#include "database/structures/PointerGrid.h"
//#include "database/structures/SuccinctGrid.h"

using namespace chomp;
int INITIALSUBDIVISIONS = 0;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 24 - INITIALSUBDIVISIONS;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 32 - INITIALSUBDIVISIONS;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;

/**************/
/*  EXAMPLES  */
/**************/

// choose example by uncommenting define
//#define LORENZ
#define TWODIMLESLIE
//#define TWODIMLESLIEPRISMCAPD
//#define PRISMLESLIE
//#define FIVEDIMPRISMLESLIE


#ifdef TWODIMLESLIEPRISMCAPD
#include "data/leslie12prism2/ModelMap.h"
Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 224.040;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}

#endif

#ifdef FIVEDIMPRISMLESLIE
#include "database/maps/leslie5d.h"

Rect initialize_phase_space_box ( void ) {
  int phase_space_dimension = 5;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;// -25.0
  phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 224.04;
  phase_space_bounds . lower_bounds [ 2 ] = 0.0;
  phase_space_bounds . upper_bounds [ 2 ] = 156.828;
  phase_space_bounds . lower_bounds [ 3 ] = 0.0;
  phase_space_bounds . upper_bounds [ 3 ] = 109.78;
  phase_space_bounds . lower_bounds [ 4 ] = 0.0;
  phase_space_bounds . upper_bounds [ 4 ] = 76.8457;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}

#endif

//#define VANDERPOL 
#ifdef VANDERPOL
#include "database/maps/VanderPol.h"
typedef VanderPol ModelMap;

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -5.0;// -25.0
  phase_space_bounds . upper_bounds [ 0 ] = 5.0;
  phase_space_bounds . lower_bounds [ 1 ] = -8.0;
  phase_space_bounds . upper_bounds [ 1 ] = 8.0;

  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  return Rect ( 2 );
}

#endif


#ifdef LORENZ
#include "database/maps/LorenzMap.h"
typedef LorenzMap ModelMap;

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 3;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -16.0;// -25.0
  phase_space_bounds . upper_bounds [ 0 ] = 16.0;
  phase_space_bounds . lower_bounds [ 1 ] = -20.0;
  phase_space_bounds . upper_bounds [ 1 ] = 20.0;
  phase_space_bounds . lower_bounds [ 2 ] = -8.0;
  phase_space_bounds . upper_bounds [ 2 ] = 32.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  return Rect ( 2 );
}
  
#endif

//#define FAT
#ifdef FAT
struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval p0, p1;
  
  ModelMap ( const Rect & rectangle ) {
    p0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    p1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () 
  ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    
    /* Perform map computation */
    //interval y0 = (p0 * x0 + p1 * x1 ) * exp ( -0.1 * (x0 + x1) );     
    //interval y1 = 0.7 * x0;
    interval y0 ( 0.0, 1.0 );
    interval y1 ( 0.0, 1.0 );
    /* Write output */
    Rect r ( 2 );
    r . lower_bounds [ 0 ] = y0 . lower ();
    r . upper_bounds [ 0 ] = y0 . upper ();
    r . lower_bounds [ 1 ] = y1 . lower ();
    r . upper_bounds [ 1 ] = y1 . upper ();
    
    return r;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 1.0;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 1.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}

#endif


#ifdef PRISMLESLIE
#include "data/leslie12prism/ModelMap.h"
Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 224.040;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}
#endif

#ifdef TWODIMLESLIE
#include "database/maps/simple_interval.h"

struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval p0, p1;
  
  ModelMap ( const Rect & rectangle ) {
    p0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    p1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () 
  ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);

    /* Perform map computation */
    interval y0 = (p0 * x0 + p1 * x1 ) * exp ( -0.1 * (x0 + x1) );     
    interval y1 = 0.7 * x0;

    /* Write output */
    Rect r ( 2 );
    r . lower_bounds [ 0 ] = y0 . lower ();
    r . upper_bounds [ 0 ] = y0 . upper ();
    r . lower_bounds [ 1 ] = y1 . lower ();
    r . upper_bounds [ 1 ] = y1 . upper ();

    return r;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 224.040;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}

#endif

 
#ifdef FOURDIMLESLIE
struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  ModelMap ( const Rect & rectangle ) {
    parameter1 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    parameter2 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    return;
  }
  Rect operator () 
  ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    interval x2 = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
    interval x3 = interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]);
    
    /* Perform map computation */
    interval y0 = (parameter1 * ( x0 + x1 ) +  parameter2 * ( x2 + x3 ) ) * exp ( (double) -0.1 * (x0 + x1 + x2 + x3) );     
    interval y1 = (double) 0.7 * x0;
    interval y2 = (double) 0.7 * x1;
    interval y3 = (double) 0.7 * x2;
    
    /* Write output */
    Rect return_value ( 4 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    return_value . lower_bounds [ 3 ] = y3 . lower ();
    return_value . upper_bounds [ 3 ] = y3 . upper ();
    return return_value;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 4;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  phase_space_bounds . upper_bounds [ 0 ] = 300.0;
  phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  phase_space_bounds . upper_bounds [ 1 ] = 300.0;
  phase_space_bounds . lower_bounds [ 2 ] = 0.0;
  phase_space_bounds . upper_bounds [ 2 ] = 300.0;
  phase_space_bounds . lower_bounds [ 3 ] = 0.0;
  phase_space_bounds . upper_bounds [ 3 ] = 300.0;
  //phase_space_bounds . lower_bounds [ 0 ] = 0.0;
  //phase_space_bounds . upper_bounds [ 0 ] = 320.056;
  //phase_space_bounds . lower_bounds [ 1 ] = 0.0;
  //phase_space_bounds . upper_bounds [ 1 ] = 224.040;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  //  parameter_space_limits . lower_bounds [ 0 ] = 8.0; 
  //  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  //  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  //  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  parameter_space_limits . lower_bounds [ 0 ] = 13.0; 
  parameter_space_limits . upper_bounds [ 1 ] = 13.0;//17.0; 
  parameter_space_limits . lower_bounds [ 0 ] = 13.0; 
  parameter_space_limits . upper_bounds [ 1 ] = 13.0;//17.0; 
                                                     // Use command line arguments to choose a box from ? x ? choices.
  int PARAMETER_BOXES = 1000; //64
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}

#endif


//#define CYCLICLOGISTIC
#ifdef CYCLICLOGISTIC
// 3D EXAMPLE FROM IPPEI

/*
 phase space = [-1.1, 1.1]^3
 parameter space = [.78125, .785625] [-.005, -.00125] [.06, .06]
                       a               epsilon         delta
 define f(x,a) = 1 - a * x^2
 y0 = (1-epsilon*2)*f(x0, a) + (epsilon - delta)*f(x2, a) + (epsilon + delta)*f(x1,a)
 y1 = (1-epsilon*2)*f(x1, a) + (epsilon - delta)*f(x0, a) + (epsilon + delta)*f(x2,a)
 y2 = (1-epsilon*2)*f(x2, a) + (epsilon - delta)*f(x1, a) + (epsilon + delta)*f(x0,a)
 
 */
struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval a, epsilon, delta;
  interval coef0, coef1, coef2;
  ModelMap ( const Rect & rectangle ) {
    a = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    epsilon = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    delta = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);
    interval one ( 1.0, 1.0 );
    coef0 = (one + (-1.0)*epsilon);
    coef1 = .5*(epsilon + (-1.0) * delta);
    coef2 = .5*(epsilon + delta);
    std::cout << " a = " << a.lower() << " " << a.upper() << "\n";
    std::cout << " epsilon = " << epsilon.lower() << " " << epsilon.upper() << "\n";
    std::cout << " delta = " << delta.lower() << " " << delta.upper() << "\n";
    std::cout << " coef0 = " << coef0.lower() << " " << coef0.upper() << "\n";
    std::cout << " coef1 = " << coef1.lower() << " " << coef1.upper() << "\n";
    std::cout << " coef2 = " << coef2.lower() << " " << coef2.upper() << "\n";
   
    return;
  }
  Rect operator () 
  ( const Rect & rectangle ) const {    
    /* Read input */
    interval x0 = interval (rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ]);
    interval x1 = interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]);
    interval x2 = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);

    /* Perform map computation */
    interval one ( 1.0, 1.0 );
    // Logistic maps.
    interval f0 = one + (-1.0) * a * x0 * x0;
    interval f1 = one + (-1.0) * a * x1 * x1;
    interval f2 = one + (-1.0) * a * x2 * x2;

    /*
    std::cout << "f0 = " << f0.lower() << " " << f0.upper() << "\n";
    std::cout << "f1 = " << f1.lower() << " " << f1.upper() << "\n";
    std::cout << "f2 = " << f2.lower() << " " << f2.upper() << "\n";
    std::cout << " coef0 = " << coef0.lower() << " " << coef0.upper() << "\n";
    std::cout << " coef1 = " << coef1.lower() << " " << coef1.upper() << "\n";
    std::cout << " coef2 = " << coef2.lower() << " " << coef2.upper() << "\n";
    */
    interval y0 = coef0*f0 + coef1*f2 + coef2*f1;
    interval y1 = coef0*f1 + coef1*f0 + coef2*f2;
    interval y2 = coef0*f2 + coef1*f1 + coef2*f0;

    /*
    std::cout << "y0 = " << y0.lower() << " " << y0.upper() << "\n";
    std::cout << "y1 = " << y1.lower() << " " << y1.upper() << "\n";
    std::cout << "y2 = " << y2.lower() << " " << y2.upper() << "\n";
    */
    
    /* Write output */
    Rect return_value ( 3 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return_value . lower_bounds [ 2 ] = y2 . lower ();
    return_value . upper_bounds [ 2 ] = y2 . upper ();
    return return_value;
  } 
  
};

// you mean depth 9
Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  //phase space = [-1.1, 1.1]^3
  //parameter space = [.78125, .785625] [-.005, -.00125] [.06, .06]
  
  // [0, 320.056] x [0.0, 224.040]
  int phase_space_dimension = 3;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -1.1;
  phase_space_bounds . upper_bounds [ 0 ] = 1.1;
  phase_space_bounds . lower_bounds [ 1 ] = -1.1;
  phase_space_bounds . upper_bounds [ 1 ] = 1.1;
  phase_space_bounds . lower_bounds [ 2 ] = -1.1;
  phase_space_bounds . upper_bounds [ 2 ] = 1.1;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Three dimensional parameter space
  //parameter space = [.78125, .785625] [-.005, -.00125] [.06, .06]
  // One example: 
  // Another example: .787, -.018, .06
  //  use center point: 
  int parameter_space_dimension = 3;
  Rect parameter_box ( parameter_space_dimension ); 
  parameter_box . lower_bounds [ 0 ] = .787; 
  parameter_box . upper_bounds [ 0 ] = .787;
  parameter_box . lower_bounds [ 1 ] = -0.018;
  parameter_box . upper_bounds [ 1 ] = -0.018;
  parameter_box . lower_bounds [ 2 ] = .06;
  parameter_box . upper_bounds [ 2 ] = .06;
  std::cout << "parameter box = " << parameter_box << "\n";
  return parameter_box;
}
#endif

#ifdef HENONEXAMPLE
// HENON MAP EXAMPLE
struct ModelMap {
  
  typedef simple_interval<double> interval;
  
  interval parameter1, parameter2;
  
  ModelMap ( const Rect & rectangle ) {
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
    interval y0 = x1 + interval ( 1.0, 1.0 ) + interval(-1.0,-1.0) * parameter1 * x0 * x0;
    interval y1 = parameter2 * x0;
    
    /* Write output */
    Rect return_value ( 2 );
    return_value . lower_bounds [ 0 ] = y0 . lower ();
    return_value . upper_bounds [ 0 ] = y0 . upper ();
    return_value . lower_bounds [ 1 ] = y1 . lower ();
    return_value . upper_bounds [ 1 ] = y1 . upper ();
    return return_value;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  int phase_space_dimension = 2;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -2.0;
  phase_space_bounds . upper_bounds [ 0 ] = 2.0;
  phase_space_bounds . lower_bounds [ 1 ] = -2.0;
  phase_space_bounds . upper_bounds [ 1 ] = 2.0;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
  parameter_space_limits . lower_bounds [ 0 ] = 1.4; 
  parameter_space_limits . upper_bounds [ 0 ] = 1.4;
  parameter_space_limits . lower_bounds [ 1 ] = .3;
  parameter_space_limits . upper_bounds [ 1 ] = .3;
  // Use command line arguments to choose a box from ? x ? choices.
  int PARAMETER_BOXES = 64;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  
  return parameter_box;
}
#endif

////////////////////////////////////////END USER EDIT//////////////////////////////////////////

/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] )
{
  
  clock_t start, stop;
  start = clock ();
  
  
  /* READ TWO INPUTS (which will give a parameter space box) */
  if ( argc != 3 ) {
    std::cout << "Usage: Supply 2 (not " << argc << ") arguments:\n";
    std::cout << "Input two integers in [0, " << 64 << "\n";
    return 0;
  }
  Real bx = ( Real ) atoi ( argv [ 1 ] );
  Real by = ( Real ) atoi ( argv [ 2 ] );
  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();
  
  /* SET PARAMETER SPACE REGION */
  Rect parameter_box = initialize_parameter_space_box (bx, by);
  
  /* INITIALIZE PHASE SPACE */
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  phase_space -> initialize ( phase_space_bounds );
  
  for ( int i = 0; i < INITIALSUBDIVISIONS; ++ i )
    phase_space -> subdivide ();
  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box );
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  MorseGraph conley_morse_graph;
  
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph,
                       phase_space,
                       map,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  
  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " <<
  (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  std::cout << "Creating image files...\n";
  DrawMorseSets ( *phase_space, conley_morse_graph );
  std::cout << "Creating DOT file...\n";
  CreateDotFile ( conley_morse_graph );
  
  return 0;
} /* main */
