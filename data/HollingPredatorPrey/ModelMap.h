#ifndef CMDP_TRAVELWAVE_H
#define CMDP_TRAVELWAVE_H

#include "capd/capdlib.h"
//#include "capd/dynsys/DynSysMap.h"
//#include "capd/dynset/C0PpedSet.hpp"
//#include "capd/dynset/C0RectSet.hpp"
#include "capd/dynset/lib.h"
//#include <boost/numeric/interval.hpp>
#include "chomp/Rect.h"
#include "chomp/Prism.h"

//#include "database/maps/simple_interval.h"
#include "capd/intervals/lib.h"


#include <vector>

struct ModelMap {
  typedef chomp::Rect Rect;
  typedef chomp::Prism image_type;
  capd::IMap f;
  
  // ODES
  capd::interval step;
  int order;
  int num_steps;
  capd::ITaylor * solver;
  capd::ITimeMap * timeMap;
  capd::interval integrateTime;
  int D;
  chomp::Rect parameter_;
  
  chomp::Rect parameter ( void ) const { return parameter_; }
  ModelMap ( const Rect & rectangle ) {
    parameter_ = rectangle;
    using namespace capd;
    D = 4;
    f = "var:p,r,q,s,pa,pb,pc,pd,pe,pf;fun:"
    "r*(1+p),"
    "(((-pe)*r*(1+p))-(p*(((1-(pa*p))*((pc*p)-pb)*(1+p))-q)))/pf,"
    "s*(1+p),"
    "((-pe)*s*(1+p))-(q*(p-pd));";
    f.setParameter("pa", interval(rectangle . lower_bounds [ 0 ], rectangle . upper_bounds [ 0 ] ) );
    f.setParameter("pb", interval (rectangle . lower_bounds [ 1 ], rectangle . upper_bounds [ 1 ]) );
    f.setParameter("pc", interval (rectangle . lower_bounds [ 2 ], rectangle . upper_bounds [ 2 ]) );
    f.setParameter("pd", interval (rectangle . lower_bounds [ 3 ], rectangle . upper_bounds [ 3 ]) );
    f.setParameter("pe", interval (rectangle . lower_bounds [ 4 ], rectangle . upper_bounds [ 4 ]) );
    f.setParameter("pf", interval (rectangle . lower_bounds [ 5 ], rectangle . upper_bounds [ 5 ]) );
 
    
    
    step= interval(1.0/32.0,1.0/32.0);
    order = 3;
    num_steps = 6; // default, change to be in constructor
    //integrateTime = interval(1.0/8.0, 1.0/8.0);

    solver = new ITaylor (f,order,step);
    timeMap = new ITimeMap ( *solver );
    
    return;
  }
  
  ~ModelMap ( void ) {
    delete solver;
    delete timeMap;
  }
  
  chomp::Rect intervalMethod ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    // Count executions
    static size_t execution_count = 0;
    ++ execution_count;
    //std::cout << rectangle << "\n";
    if ( execution_count % 10000 == 0 ) {
      std::cout << execution_count << "\n";
    }
    
    // Put input into IVector structure "x0"
    IVector c ( D );
    for ( int d = 0; d < D; ++ d ) {
      c [ d ] = interval (rectangle . lower_bounds [ d ],
                          rectangle . upper_bounds [ d ] );
    }
    // Use integrator
    capd::dynset::C0Rect2Set<IMatrix> s(c);
    
    //IVector mapped = (*timeMap)(integrateTime,s);  //(really doesn't work at all!)
    for (int step = 0; step < num_steps; ++ step) {
      s . move ( *solver ); // move the set under the flow
    }
    IVector mapped ( s );
    
    // Return result
    chomp::Rect result ( D );
    for ( int d = 0; d < D; ++ d ) {
      result . lower_bounds [ d ] = mapped[d].leftBound();
      result . upper_bounds [ d ] = mapped[d].rightBound();
    }
    return result;
  }
  
  chomp::Prism ppedMethod ( const chomp::Rect & rectangle ) const {
    using namespace capd;
    
    static size_t execution_count = 0;
    ++ execution_count;
    //std::cout << rectangle << "\n";
    if ( execution_count % 10000 == 0 ) {
      std::cout << execution_count << "\n";
    }
    // Put input into IVector structure "x0"
    IVector box ( D );
    for ( int d = 0; d < D; ++ d ) {
      box [ d ] = interval ( rectangle . lower_bounds [ d ],
                            rectangle . upper_bounds [ d ] );
    }
    //capd::dynset::C0PpedSet<IMatrix> rect ( box );
    capd::dynset::C0RectSet<IMatrix> rect ( box );

    /* Perform map computation */
    chomp::Prism P ( D );
    try {
      
      for (int i =0; i< num_steps; ++i ) {
        rect.move(*solver);
      }
      
      IMatrix B = rect . get_B ();
      IVector r = rect . get_r ();
      IVector x = rect . get_x ();
      // x + Br
      IMatrix Diag ( D, D );
      for ( int d = 0; d < D; ++ d ) {
        Diag (d+1, d+1) = std::max ( abs ( r [ d ] . rightBound () ),
                                    abs ( r [ d ] . leftBound () ) );
      }
      B = B * Diag;
      
      for ( int d = 0; d < D; ++ d ) P . c ( d ) = ( x [ d ] . leftBound () + x [ d ] . rightBound ()) / 2.0;
      for ( int i = 0; i < D; ++ i ){
        for ( int j = 0; j < D; ++ j ) {
          P . A (i,j) = (B ( i + 1, j + 1 ) . leftBound () + B ( i+1, j+1) . rightBound () ) / 2.0;
        }
      }
      
    } catch (std::exception& e) {
      
      std::cout << "Threw exception" << e.what() << "\n" ;
      abort ();
    }
    return P;
  }
  
    
  // OPERATOR () FUNCTION
  
  //std::pair < chomp::Rect, chomp::Prism > operator ()
  //( const chomp::Rect & rectangle ) const {
  //  return std::make_pair ( intervalMethod ( rectangle ), ppedMethod ( rectangle ) );
  //}

  
  /*
  chomp::Rect operator () ( const chomp::Rect & rectangle ) const {
    return intervalMethod ( rectangle );
  }
   */
  
  
  chomp::Prism  operator () ( const chomp::Rect & rectangle ) const {
    return ppedMethod ( rectangle );
  }
   
  
  
  bool good ( void ) const { return true; }
  
};


#endif
