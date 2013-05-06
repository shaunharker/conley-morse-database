/* Integrator for Database using CAPD */

#ifndef CMDP_INTEGRATOR_H
#define CMDP_INTEGRATOR_H

//#include "capd/krak/krak.h"

#include "chomp/Rect.h"

#include <cmath>
#include <stdexcept>
#include <fstream>

#include "boost/shared_ptr.hpp"

// To avoid a conflict with X11
#undef None
#include "capd/capdlib.h"


// Generic name for the class MapRect
class MapRect {
public:
  
  typedef chomp::Rect Rect;
  typedef chomp::Rect image_type;
  int D;
  mutable int order;
  mutable capd::interval step;
  mutable boost::shared_ptr<capd::IMap> f;
  mutable boost::shared_ptr < capd::ITaylor > solver;
  mutable bool exception_thrown;
  
  MapRect ( void ) {
    std::cout << "default constructor.\n";
  }
  
  /*
  MapRect ( const chomp::Rect & rectangle ) {
    using namespace capd;
    D = 2;
    f . reset ( new IMap );
    *f = "par:c;var:x,y;fun:y,-x+c*(1-x*x)*y;";
    f -> setParameter ( "c", interval(rectangle.lower_bounds[0],
                                      rectangle.upper_bounds[0]));
    
    step= interval(1.0/(256.0),1.0/(256.0));
    order = 5;
    solver . reset ( new ITaylor (*f,order,step) );
    return;
  }
  */

  MapRect ( const chomp::Rect & rectangle, double timestep ) {
    using namespace capd;

    // Dimension of the phase space
    D = 2;

    f . reset ( new IMap );  

    *f = "par:nu,eta,alpha,gamma;var:x,y;fun:nu-x*y^gamma/(x*y^gamma+y^gamma+1.0),alpha*x*y^gamma/(x*y^gamma+y^gamma+1.0)-eta*y;";
 
    // since gamma=2, we write y^gamma as y*y
    //*f = "par:nu,eta,alpha,gamma;var:x,y;fun:nu-x*y*y/(x*y*y+y*y+1.0),alpha*x*y*y/(x*y*y+y*y+1.0)-eta*y;";

    // Normalized system
    //*f = "par:nu,eta,alpha,gamma;var:x,y;fun:(nu-x*y*y/(x*y*y+y*y+1.0))/(sqrt(0.01+((x*y*y*alpha)/(1+y*y+x*y*y)-y*eta)*((x*y*y*alpha)/(1+y*y+x*y*y)-y*eta)+(-((x*y*y)/(1.0+y*y+x*y*y))+nu)*(-((x*y*y)/(1.0+y*y+x*y*y))+nu))),(alpha*x*y*y/(x*y*y+y*y+1.0)-eta*y)/(sqrt(0.01+((x*y*y*alpha)/(1+y*y+x*y*y)-y*eta)*((x*y*y*alpha)/(1+y*y+x*y*y)-y*eta)+(-((x*y*y)/(1.0+y*y+x*y*y))+nu)*(-((x*y*y)/(1.0+y*y+x*y*y))+nu)));";

    f -> setParameter ( "nu", interval(rectangle.lower_bounds[0],
                                      rectangle.upper_bounds[0]));
    f -> setParameter ( "eta", interval(rectangle.lower_bounds[1],
                                      rectangle.upper_bounds[1]));
    f -> setParameter ( "alpha", interval(rectangle.lower_bounds[2],
                                      rectangle.upper_bounds[2]));
    f -> setParameter ( "gamma", interval(rectangle.lower_bounds[3],
                                      rectangle.upper_bounds[3]));

    step= interval(timestep, timestep);
    order = 5;
    solver . reset ( new ITaylor (*f,order,step) );
    std::cout << "Construct. timestep = " << timestep << "  solver = " << solver . get () << "\n";
    
    return;
  }
  
  bool & exception ( void ) const { return exception_thrown; }
  
  chomp::Rect intervalMethod ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    //std::cout << rectangle << "\n";
    
    // Count executions
    static size_t execution_count = 0;
    ++ execution_count;
    //std::cout << rectangle << "\n";
#if 0
    if ( execution_count % 10000 == 0 ) {
      std::cout << "Execution count = " << execution_count << "\n";
      std::cout << "Rect = " << rectangle << "\n";
    }
#endif
    
    // Put input into IVector structure "c"
    IVector c ( D );
    for ( int d = 0; d < D; ++ d ) {
      c [ d ] = interval (rectangle . lower_bounds [ d ],
                          rectangle . upper_bounds [ d ] );
    }
    // Use integrator
    capd::dynset::C0Rect2Set<IMatrix> s(c);
    s . move ( *solver ); // move the set under the flow
    IVector mapped ( s );
    
    // Return result
    chomp::Rect result ( D );
    for ( int d = 0; d < D; ++ d ) {
      result . lower_bounds [ d ] = mapped[d].leftBound();
      result . upper_bounds [ d ] = mapped[d].rightBound();
    }
    return result;
  }
  
  
  chomp::Rect  operator () ( const chomp::Rect & rectangle ) const {
    static size_t exception_count = 0;
    chomp::Rect result ( 2 );
    try {
      result = intervalMethod ( rectangle );
    } catch (std::exception& e) {
      //std::cout << e . what () << "\n";

      exception_thrown = true;
      // Count exceptions
      ++ exception_count;
      //std::cout << rectangle << "\n";
      
      //result . lower_bounds [ 0 ] = -100.0;
      //result . upper_bounds [ 0 ] = 100.0;
      //result . lower_bounds [ 1 ] = -100.0;
      //result . upper_bounds [ 1 ] = 100.0;
      result = rectangle;
      if ( exception_count > 100 ) {
        std::cout << e . what () << "\n";
        exception_count = 0;
        throw e;
      }
      
    }
    return result;
  }
  
  
  
  bool good ( void ) const { return true; }
};

#endif

#if 0
if ( exception_count % 10000 == 0 ) {
  std::cout << "Exception count = " << exception_count << "\n";
  }
#endif
