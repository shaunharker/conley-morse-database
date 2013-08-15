
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
#include <iterator>     // std::insert_iterator
#include <cassert>
#include <limits>

/***************************/
/* Preprocessor directives */
/***************************/

//#define SNF_DEBUG

#define CMG_VERBOSE
#define MEMORYBOOKKEEPING
//#define NO_REACHABILITY
//#define CMDB_STORE_GRAPH
//#define ODE_METHOD

#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "chomp/ConleyIndex.h"
#include "chomp/Rect.h"

#include "database/tools/SingleOutput.h"


#include "CImg.h"
//#undef None

/*************************/
/* Subdivision Settings  */
/*************************/

#undef GRIDCHOICE

#ifdef USE_SUCCINCT
#define GRIDCHOICE SuccinctGrid
#include "database/structures/SuccinctGrid.h"
#else
#define GRIDCHOICE PointerGrid
#include "database/structures/PointerGrid.h"
#endif

using namespace chomp;
int SINGLECMG_INITIAL_SUBDIVISIONS = 11;
int SINGLECMG_MIN_PHASE_SUBDIVISIONS = 12;
int SINGLECMG_MAX_PHASE_SUBDIVISIONS = 17;
int SINGLECMG_COMPLEXITY_LIMIT = 100;


/**************/
/*    MAP     */
/**************/
//#include "database/maps/Newton4D.h"
#include "/Users/sharker/Desktop/Newton4D-1/ModelMap.h"
Rect initialize_phase_space_box ( void ) {
  const Real pi = 3.14159265358979323846264338327950288;
  int phase_space_dimension = 1;
  Rect phase_space_bounds ( phase_space_dimension );
  phase_space_bounds . lower_bounds [ 0 ] = -pi;
  phase_space_bounds . upper_bounds [ 0 ] = pi ;
  std::cout << "Phase Space Bounds = " << phase_space_bounds << "\n";
  return phase_space_bounds;
}


Rect initialize_parameter_space_box ( double a, 
				      double b, 
				      double c,
				      double phi ) {
  int parameter_space_dimension = 4;
  Rect parameter_box ( parameter_space_dimension );
  parameter_box.lower_bounds[0]=c;
  parameter_box.upper_bounds[0]=c;
  parameter_box.lower_bounds[1]=phi;
  parameter_box.upper_bounds[1]=phi;
  parameter_box.lower_bounds[2]=a;
  parameter_box.upper_bounds[2]=a;
  parameter_box.lower_bounds[3]=b;
  parameter_box.upper_bounds[3]=b;
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";
  return parameter_box;
}

Rect initialize_parameter_space_box ( void ) {
  //[-0.84375, -0.828125]x[0.245437, 0.269981]
  //abort ();
  int bx, by, bz;
  bx = 2;
  by = 26;
  bz = 17;
  int parameter_space_dimension = 4;
  Rect parameter_space_limits ( parameter_space_dimension ); 

  /*
  // an example that was giving me trouble
  // [-0.129492, -0.129004]x[1.37716, 1.37793]
    parameter_space_limits . lower_bounds [ 0 ] = .0003906250000000056;
  parameter_space_limits . upper_bounds [ 0 ] = .0008789062499999917;
  parameter_space_limits . lower_bounds [ 1 ] = .785;
  parameter_space_limits . upper_bounds [ 1 ] = .7857690429687499;
*/

  /*
// period 15 orbit
 parameter_space_limits . lower_bounds [ 0 ]  = -0.09482421875000001;
 parameter_space_limits . upper_bounds [ 0 ] =  -0.09433593749999999;
 parameter_space_limits . lower_bounds [ 1 ]  = 2.04699951171875;
 parameter_space_limits . upper_bounds [ 1 ] = 2.0477685546875;
*/
 /*
  // Region with an unstable minimal morse set (bug)
  //[-0.1413978576660156, -0.1413034439086914] X [1.668061140625, 1.66821059375]  
 parameter_space_limits . lower_bounds [ 0 ]  = -0.1413978576660156;
 parameter_space_limits . upper_bounds [ 0 ] =  -0.1413034439086914;
 parameter_space_limits . lower_bounds [ 1 ]  = 1.668061140625;
 parameter_space_limits . upper_bounds [ 1 ] = 1.66821059375;
*/

  //parameter_space_limits . lower_bounds [ 0 ] = .41015625;
  //parameter_space_limits . upper_bounds [ 0 ] =.412109375;
  //parameter_space_limits . lower_bounds [ 1 ] = 0.4939418136991764;
  //parameter_space_limits . upper_bounds [ 1 ] = 0.4970097752749477;

  // Bug inside double winding region (phase space -- init6, min7, max8)
 // a x b x c =
  //[-0.34375, -0.328125] X [0.265625, 0.28125] X [-0.078125, -0.0625]
/*
  parameter_space_limits . lower_bounds [ 0 ] = -0.078125;
  parameter_space_limits . upper_bounds [ 0 ] = -0.0625;
  parameter_space_limits . lower_bounds [ 1 ] = 1.0472;
  parameter_space_limits . upper_bounds [ 1 ] = 1.0472;  
 parameter_space_limits . lower_bounds [ 2 ]  = -0.34375;
 parameter_space_limits . upper_bounds [ 2 ] =  -0.328125;
 parameter_space_limits . lower_bounds [ 3 ]  = 0.265625;
 parameter_space_limits . upper_bounds [ 3 ] = 0.28125;
*/
 // bug in Newton3D-1
   parameter_space_limits . lower_bounds [ 0 ] = -0.140625;
  parameter_space_limits . upper_bounds [ 0 ] = -0.125;
  parameter_space_limits . lower_bounds [ 1 ] = 0.0;
  parameter_space_limits . upper_bounds [ 1 ] = 0.0;  
 parameter_space_limits . lower_bounds [ 2 ]  = 0.375;
 parameter_space_limits . upper_bounds [ 2 ] =  0.390625;
 parameter_space_limits . lower_bounds [ 3 ]  = 0.453125;
 parameter_space_limits . upper_bounds [ 3 ] = 0.46875;

 // x^2 + x + 1

  parameter_space_limits . lower_bounds [ 0 ] = -0.104;
  parameter_space_limits . upper_bounds [ 0 ] = -0.104;
  parameter_space_limits . lower_bounds [ 1 ] = 1.697;
  parameter_space_limits . upper_bounds [ 1 ] = 1.697;  
  parameter_space_limits . lower_bounds [ 2 ]  = -1.0;
  parameter_space_limits . upper_bounds [ 2 ] =  -1.0;
  parameter_space_limits . lower_bounds [ 3 ]  = 1.0;
  parameter_space_limits . upper_bounds [ 3 ] = 1.0;

  // x + 21 (discovered polynomial string writing bug with this one)
  // [-1, -0.96875]x[0.343612, 0.392699]x[-0.84375, -0.8125]x[0.25, 0.28125]
  parameter_space_limits . lower_bounds [ 0 ] = -1.0;
  parameter_space_limits . upper_bounds [ 0 ] = -0.96875;
  parameter_space_limits . lower_bounds [ 1 ] = 0.3436116964863836;
  parameter_space_limits . upper_bounds [ 1 ] = 0.39269908169872414;  
  parameter_space_limits . lower_bounds [ 2 ]  = -0.84375;
  parameter_space_limits . upper_bounds [ 2 ] =  -0.8125;
  parameter_space_limits . lower_bounds [ 3 ]  = 0.25;
  parameter_space_limits . upper_bounds [ 3 ] = 0.28125;

  // Empty Hasse Diagram?
  // [-0.8125, -0.78125]x[2.30711, 2.35619]x[0.59375, 0.625]x[0, 0.03125]
  parameter_space_limits . lower_bounds [ 0 ] = -0.8125;
  parameter_space_limits . upper_bounds [ 0 ] = -0.78125;
  parameter_space_limits . lower_bounds [ 1 ] = 2.3071071049800045;
  parameter_space_limits . upper_bounds [ 1 ] = 2.356194490192345;  
  parameter_space_limits . lower_bounds [ 2 ]  = 0.59375;
  parameter_space_limits . upper_bounds [ 2 ] =  0.625;
  parameter_space_limits . lower_bounds [ 3 ]  = 0.0;
  parameter_space_limits . upper_bounds [ 3 ] = 0.03125;
/*
 // 2D a=-1 b=1 slice
  parameter_space_limits . lower_bounds [ 2 ] = -1.0;
  parameter_space_limits . upper_bounds [ 2 ] = -1.0;
  parameter_space_limits . lower_bounds [ 3 ] = 1.0;
  parameter_space_limits . upper_bounds [ 3 ] = 1.0; 

  // Crash at 4-6-12
  //[-5.000000000000000e-01, -4.687500000000000e-01]x[1.374446785945534e+00, 1.423534171157875e+00]x[-5.937500000000000e-01, -5.625000000000000e-01]x[-1.562500000000000e-01, -1.250000000000000e-01]

  parameter_space_limits . lower_bounds [ 0 ]  = -0.5;
 parameter_space_limits . upper_bounds [ 0 ] =  -0.46875;
 parameter_space_limits . lower_bounds [ 1 ]  = 1.374446785945534;
 parameter_space_limits . upper_bounds [ 1 ] = 1.423534171157875;
  parameter_space_limits . lower_bounds [ 2 ] = -0.59375;
  parameter_space_limits . upper_bounds [ 2 ] = -0.5625;
  parameter_space_limits . lower_bounds [ 3 ] = -0.15625;
  parameter_space_limits . upper_bounds [ 3 ] = -0.125;  
  */
  /*
int PARAMETER_BOXES = 32;
  Rect parameter_box ( parameter_space_dimension );
  
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / (float) PARAMETER_BOXES;
  
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / (float) PARAMETER_BOXES;
  
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] +  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / (float) PARAMETER_BOXES;
  
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] +  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / (float) PARAMETER_BOXES;
  
  parameter_box . lower_bounds [ 2 ] = parameter_space_limits . lower_bounds [ 2 ] +  ( parameter_space_limits . upper_bounds [ 2 ] - parameter_space_limits . lower_bounds [ 2 ] ) * bz / (float) PARAMETER_BOXES;
  
  parameter_box . upper_bounds [ 2 ] = parameter_space_limits . lower_bounds [ 2 ] +  ( parameter_space_limits . upper_bounds [ 2 ] - parameter_space_limits . lower_bounds [ 2 ] ) * ( bz + 1.0 ) / (float) PARAMETER_BOXES;
  
  std::cout << "Parameter Box Choice = " << parameter_box << "\n";

  //  Rect parameter_space_limits ( parameter_space_dimension ); 
  //parameter_space_limits . lower_bounds [ 0 ] = -0.84375;
  //parameter_space_limits . upper_bounds [ 0 ] = -0.828125;
  //parameter_space_limits . lower_bounds [ 1 ] = 0.245437;
  //parameter_space_limits . upper_bounds [ 1 ] = 0.269981;
  */

  std::cout << "Parameter Space Bounds = " << parameter_space_limits << "\n";

  return parameter_space_limits;
}

void test_map ( const ModelMap & map ) {
  std::ofstream lowerfile ( "newtonmaplower.txt" );
  std::ofstream upperfile ( "newtonmapupper.txt" );
  double h = .1;
  for ( double x = -3.1415926535; x < 3.1415926535; x += h ) {
    chomp::Rect r(1);
    r . lower_bounds[0] = x;
    r . upper_bounds[0] = x + h;
    chomp::Rect result = map ( r );
    lowerfile << x + h/2.0 << " " << result . lower_bounds [ 0 ] << "\n";
    upperfile << x + h/2.0 << " " << result . upper_bounds [ 0 ] << "\n";
  }
  lowerfile . close ();
  upperfile . close ();
  
}

void test_grid ( const ModelMap & map, const Grid & grid ) {
  using namespace cimg_library;

  std::cout << "Size of grid = " << grid . size () << "\n";
  double pi = 3.1415926535;
  int PICSIZE = 512;
  CImg<unsigned char> img ( PICSIZE, PICSIZE, 1, 3, 0 );
  /*
  BOOST_FOREACH ( Grid::GridElement ge, grid ) {
    chomp::Rect x = grid . geometry ( ge );
    {
    int i = PICSIZE*(x . lower_bounds [ 0 ] + pi)/ (2.0*pi);
    for ( int j = 0; j < PICSIZE; ++ j ) {
      img (i,j,0,0)=255;
      img (i,j,0,1)=255;
      img (i,j,0,2)=255;
    }
    }
    {
    int j = PICSIZE*(x . lower_bounds [ 0 ] + pi)/ (2.0*pi);
    for ( int i = 0; i < PICSIZE; ++ i ) {
      img (i,j,0,0)=255;
      img (i,j,0,1)=255;
      img (i,j,0,2)=255;
    }
    }
  }
  */
  BOOST_FOREACH ( Grid::GridElement ge, grid ) {
    chomp::Rect x = grid . geometry ( ge );
    chomp::Rect y = map ( x );
    //std::cout << ge << ": " << x << " -> " << y << "\n";
    std::vector < Grid::GridElement > image;
    std::insert_iterator < std::vector < Grid::GridElement > >
    ii ( image, image.begin () );
    grid . cover ( ii, y );
    assert ( image . size () != 0 );
    BOOST_FOREACH ( Grid::GridElement co_ge, image ) {
      chomp::Rect z = grid . geometry ( co_ge );
      //std::cout << "DRAW" << ge << ": " << x << " -> " << z << "\n";
      int left = PICSIZE*(x . lower_bounds [ 0 ] + pi)/ (2.0*pi);
      int right = PICSIZE*(x . upper_bounds [ 0 ] + pi)/ (2.0*pi);
      int bottom = PICSIZE*(z . lower_bounds [ 0 ] + pi)/ (2.0*pi);
      int top = PICSIZE*(z . upper_bounds [ 0 ] + pi)/ (2.0*pi);
      left = std::max ( 0, left ); 
      if ( left == right ) right ++;
      right = std::min ( 1023, right );
      bottom = std::max ( 0, bottom ); 
      if (bottom == top ) ++ top;
      top = std::min ( 1023, top );

      //std::cout << "LR = " << left << ", " << right << "\n";
      //std::cout << "BT = " << bottom << ", " << top << "\n";

      for ( int i = left; i < right; ++ i ) {
        bool iflag = false;
        if ( i == left ) iflag = true;
        for ( int j = bottom; j < top; ++ j ) {
          bool jflag = false;
          if ( j == bottom  ) jflag = true;
          if ( iflag || jflag ) img ( i, PICSIZE - j, 0, 0 ) = 255;
          img ( i, PICSIZE - j, 0, 1 ) = 255;
        }
      }
    }
  }
  CImgDisplay main_disp(img,"behold, a combinatorial map");
  while (!main_disp.is_closed() ) {
    main_disp.wait();
  }

}

/*****************/
/* Main Program  */
/*****************/
int main ( int argc, char * argv [] ) {
  
   
  clock_t start, stop;
  start = clock ();
  std::cout << "A\n";  
  /* SET PHASE SPACE REGION */
  Rect phase_space_bounds = initialize_phase_space_box ();
  std::cout << "A2\n";  
  /* SET PARAMETER SPACE REGION */
  Rect parameter_box;
  if ( argc == 2 ) {
    std::string s;


  } else {
    parameter_box = initialize_parameter_space_box ();
    std::cout << "B\n";
  }

  
  /* INITIALIZE PHASE SPACE */
  boost::shared_ptr<GRIDCHOICE> phase_space (new GRIDCHOICE);
  std::vector<bool> periodicity ( 1, true );
  phase_space -> initialize ( phase_space_bounds, periodicity );
  
  /* INITIALIZE MAP */
  ModelMap map ( parameter_box );
  test_map ( map );

  /* INITIALIZE CONLEY MORSE GRAPH (create an empty one) */
  MorseGraph mg;
  std::cout << "D\n";
  /* COMPUTE CONLEY MORSE GRAPH */
  Compute_Morse_Graph ( & mg,
                       phase_space,
                       map,
                       SINGLECMG_INITIAL_SUBDIVISIONS,
                       SINGLECMG_MIN_PHASE_SUBDIVISIONS,
                       SINGLECMG_MAX_PHASE_SUBDIVISIONS,
                       SINGLECMG_COMPLEXITY_LIMIT );
  std::cout << "E\n";

  std::cout << "Number of Morse Sets = " << mg . NumVertices () << "\n";
  #if 1
  typedef std::vector < Grid::GridElement > Subset;
  for ( size_t v = 0; v < mg . NumVertices (); ++ v) {
  //size_t v = 2;
    Subset subset = phase_space -> subset ( * mg . grid ( v ) );

    // check if subset is uniform depth.
    //BOOST_FOREACH ( Grid::GridElement ge, * mg . grid ( v ) ) {
    //  std::cout << "Depth of ge = " << mg . grid ( v ) -> geometry ( ge ) << " is " <<
    //  mg . grid ( v ) -> depth ( ge ) << "\n";
    //}
    std::cout << "Calling Conley_Index on Morse Set " << v << "\n";
    boost::shared_ptr<ConleyIndex_t> conley ( new ConleyIndex_t );
    mg . conleyIndex ( v ) = conley;
    ConleyIndex ( conley . get (),
                 *phase_space,
                 subset,
                 map );
    std::vector<std::string> ci_vec = conleyIndexString ( *conley );
    for ( int d = 0; d < ci_vec . size (); ++ d ) std::cout << ci_vec[d] << "\n";
    
  }
#endif
  
  stop = clock ();
  std::cout << "Total Time for Finding Morse Sets and reachability relation: " <<
  (float) (stop - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  test_grid ( map, *phase_space );
  //std::cout << "Creating image files...\n";
  //DrawMorseSets ( *phase_space, mg );
  //std::cout << "Creating DOT file...\n";
  CreateDotFile ( mg );
  
  return 0;
} /* main */

