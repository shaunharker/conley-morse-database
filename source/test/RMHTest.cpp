/// Construct and display a conley morse graph for a given dynamical system

//#define OLD_CMAP_METHOD

// STANDARD HEADERS
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <set>
#include <map>

// To get SCC chatter
#define CMG_VERBOSE 
#define DO_CONLEY_INDEX
#define RMHMEASUREGRAPH
//#define NOREACHABILITY
#define VISUALIZE_DEBUG
#define ILLUSTRATE

// HEADERS FOR DATA STRUCTURES
#include "chomp/Toplex.h"
#include "chomp/ConleyIndex.h"
#include "database/structures/Conley_Morse_Graph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"



//////////////////////////////////////BEGIN USER EDIT//////////////////////////////////////////

// MAP FUNCTION OBJECT
#include <cmath>
#include "database/maps/simple_interval.h"   // for interval arithmetic
#include "chomp/Rect.h"
#include "chomp/Prism.h"

using namespace chomp;

// choose example

//#define TWODIMLESLIE
//#define FOURDIMLESLIE
//#define TWOSPHERE
#define CYCLICLOGISTIC
//#define HENONEXAMPLE

#ifdef TWODIMLESLIE 
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 26;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 30;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;
#endif

#ifdef FOURDIMLESLIE 
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 26;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 30;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;
#endif

#ifdef TWOSPHERE
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 26;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 30;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;
#endif

#ifdef CYCLICLOGISTIC 
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 26;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 30;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;
#endif

#ifdef HENONEXAMPLE 
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 26;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 30;
int SINGLECMG_COMPLEXITY_LIMIT = 10000;
#endif

#ifdef TWODIMLESLIE
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

#ifdef TWOSPHERE
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
    interval x2 = interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]);

    interval r = x0*x0 + x1*x1 + x2*x2;
    interval factor = 2.0 * r - r * r;
    /* Perform map computation */
    interval y0 = factor * x0;     
    interval y1 = factor * x1;
    interval y2 = factor * x2;

    /* Write output */
    Rect r ( 3 );
    r . lower_bounds [ 0 ] = y0 . lower ();
    r . upper_bounds [ 0 ] = y0 . upper ();
    r . lower_bounds [ 1 ] = y1 . lower ();
    r . upper_bounds [ 1 ] = y1 . upper ();
    r . lower_bounds [ 2 ] = y2 . lower ();
    r . upper_bounds [ 2 ] = y2 . upper ();

    return r;
  } 
  
};

Rect initialize_phase_space_box ( void ) {
  // Two dimensional phase space
  // [0, 320.056] x [0.0, 224.040]
  //  int phase_space_dimension = 2;
  int phase_space_dimension = 3;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -2.0;
  phase_space_bounds . upper_bounds [ 0 ] = 2.0;
  phase_space_bounds . lower_bounds [ 1 ] = -2.0;
  phase_space_bounds . upper_bounds [ 1 ] = 2.0;
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
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}

Rect initialize_parameter_space_box ( const Real bx, const Real by ) {
  // Two dimensional parameter space
  // A box chosen from [8, 37] x [3, 50]
  int parameter_space_dimension = 2;
  Rect parameter_space_limits ( parameter_space_dimension ); 
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

// TYPEDEFS
typedef std::vector < GridElement > CellContainer;
typedef ConleyMorseGraph < CellContainer , ConleyIndex_t > CMG;

// Declarations
void DrawMorseSets ( const Toplex & phase_space, const CMG & cmg );
void CreateDotFile ( const CMG & cmg );

template < class Toplex, class CellContainer >
void output_cubes ( const Toplex & my_toplex, 
                   const ConleyMorseGraph < CellContainer , chomp::ConleyIndex_t > & conley_morse_graph );


// MAIN PROGRAM
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
  Toplex phase_space;
  phase_space . initialize ( phase_space_bounds );
  //phase_space . subdivide ();
  //phase_space . subdivide ();
  //phase_space . subdivide ();
  //phase_space . subdivide ();  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box );
  
  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  CMG conley_morse_graph;

  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & conley_morse_graph, 
                        & phase_space, 
                        map, 
                        SINGLECMG_MIN_PHASE_SUBDIVISIONS, 
                        SINGLECMG_MAX_PHASE_SUBDIVISIONS, 
                        SINGLECMG_COMPLEXITY_LIMIT );

  
  stop = clock ();
  //std::cout << "Total Time for Finding Morse Sets and reachability relation: " << (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  CMG::VertexIteratorPair vip = conley_morse_graph . Vertices ();
  CMG::VertexIterator it;
  for ( CMG::VertexIterator it = vip . first; it != vip . second; ++ it ) {
    
  }
  /* DRAW MORSE SETS */
  DrawMorseSets ( phase_space, conley_morse_graph );

  output_cubes ( phase_space, conley_morse_graph );
  
  /* OUTPUT MORSE GRAPH */
  CreateDotFile ( conley_morse_graph );
  
  return 0;
} /* main */

/* ----------------------------  OUTPUT FUNCTIONS ---------------------------- */

// HEADERS FOR DEALING WITH PICTURES
#include "database/tools/picture.h"
#include "database/tools/lodepng/lodepng.h"

void DrawMorseSets ( const Toplex & phase_space, const CMG & conley_morse_graph ) {
  // Create a Picture
  int Width =  4096;
  int Height = 4096;
  Picture * picture = draw_morse_sets<Toplex,CellContainer>( Width, Height, phase_space, conley_morse_graph );
  LodePNG_encode32_file( "morse_sets.png", picture -> bitmap, picture -> Width, picture -> Height);
  Picture * picture2 = draw_toplex <Toplex,CellContainer>( Width, Height, phase_space );
  LodePNG_encode32_file( "toplex.png", picture2 -> bitmap, picture2 -> Width, picture2 -> Height);
  Picture * picture3 = draw_toplex_and_morse_sets <Toplex,CellContainer>( Width, Height, phase_space, conley_morse_graph );
  LodePNG_encode32_file( "toplex_and_morse.png", picture3 -> bitmap, picture3 -> Width, picture3 -> Height);
  delete picture;
  delete picture2;
  delete picture3;
}


void CreateDotFile ( const CMG & cmg ) {
  typedef CMG::Vertex V;
  typedef CMG::Edge E;
  typedef CMG::VertexIterator VI;
  typedef CMG::EdgeIterator EI;

  std::ofstream outfile ("morsegraph.gv");
  
  outfile << "digraph G { \n";
  //outfile << "node [ shape = point, color=black  ];\n";
  //outfile << "edge [ color=red  ];\n";

  // LOOP THROUGH VERTICES AND GIVE THEM NAMES
  std::map < V, int > vertex_to_index;
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
    vertex_to_index [ *start ] = i;
    outfile << i << " [label=\""<< cmg . CellSet (*start) .size () << "\"]\n";
    ++ i;
  }
  int N = cmg . NumVertices ();
  
  // LOOP THROUGH CMG EDGES
  EI estart, estop;
  typedef std::pair<int, int> int_pair;
  std::set < int_pair > edges;
  for (boost::tie ( estart, estop ) = cmg . Edges ();
       estart != estop;
       ++ estart ) {
    V source = estart -> first; 
    V target = estart -> second; 
    //std::cout << "I see (" << source << ", " << target << ")\n";
    int index_source = vertex_to_index [ source ];
    int index_target = vertex_to_index [ target ];
    //std::cout << "I see indexed (" << index_source << ", " << index_target << ")\n";

    if ( index_source != index_target ) // Cull the self-edges
      edges . insert ( std::make_pair ( index_source, index_target ) );
  }
  // TRANSITIVE REDUCTION (n^5, non-optimal)
  // We determine those edges (a, c) for which there are edges (a, b) and (b, c)
  // and store them in "transitive_edges"
  std::set < int_pair > transitive_edges;
  BOOST_FOREACH ( int_pair edge, edges ) {
    for ( int j = 0; j < N; ++ j ) {
      bool left = false;
      bool right = false;
      BOOST_FOREACH ( int_pair edge2, edges ) {
        if ( edge2 . first == edge . first && edge2 . second == j ) left = true;
        if ( edge2 . first == j && edge2 . second == edge . second ) right = true;
      }
      if ( left && right ) transitive_edges . insert ( edge );
    }
  }
  
  // PRINT OUT EDGES OF TRANSITIVE CLOSURE
  BOOST_FOREACH ( int_pair edge, edges ) {
    if ( transitive_edges . count ( edge ) == 0 )
      outfile << edge . first << " -> " << edge . second << ";\n";
  }
  
  edges . clear ();
  transitive_edges . clear ();

  outfile << "}\n";
  
  outfile . close ();
  
}

template < class Toplex, class CellContainer >
void output_cubes ( const Toplex & my_toplex, 
                    const ConleyMorseGraph < CellContainer , chomp::ConleyIndex_t > & conley_morse_graph ) {
  using namespace chomp;
  typedef ConleyMorseGraph < CellContainer , ConleyIndex_t > CMG;
    
  // Loop Through Morse Sets to determine bounds
  typedef typename CMG::VertexIterator VI;
  VI it, stop;
  std::vector < std::vector < uint32_t > > cubes;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices (); it != stop; ++ it ) {
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    int depth = my_toplex . getDepth ( my_subset );
    BOOST_FOREACH ( const typename Toplex::Top_Cell & ge, my_subset ) {
      my_toplex . GridElementToCubes ( & cubes, ge, depth  );
    }
  }
  std::ofstream outfile ( "morsecubes.txt" );
  for ( uint32_t i = 0; i < cubes . size (); ++ i ) {
    for ( uint32_t j = 0; j < cubes [ i ] . size (); ++ j ) {
      outfile << cubes [ i ] [ j ] << " ";
    }
    outfile << "\n";
  }
  outfile . close ();
} /* draw_morse_sets */
