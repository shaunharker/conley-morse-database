#ifndef MODELMAP_H
#define MODELMAP_H

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"
#include "database/structures/RectGeo.h"
#include "database/maps/AtlasMap.h"

#include "database/numerics/boost_interval.h"

#include "BooleanClasses.h"

// ODE : gamma < 0
// 
// \dot X_i = gamma_i * X_i + sigma_ij 
//
// THE SOLUTION IS 
//
// X ( t ) = -sigma/gamma + ( X(0) + sigma/gamma ) * exp ( gamma * t )
//
// Will take and return an AtlasGeo of Codimension 1
// 
//
// Map between two faces defined as : face1 -> face2
//
// Face : dir = i ( means degenerate in the i-th direction )
//        rect = I_0 x I_1 x ... x I_(i-1) x {C_i} x I_(i+1) x ... I_(n-1) x I_n 
//
//
//
//

// Define the analytical solution 
inline double timemap ( double gamma, double sigma, double x0, double T ) {
  return - sigma / gamma + ( x0 + sigma / gamma ) * exp ( gamma * T );
}

// Define the analytical solution 
inline interval timemap ( double gamma, double sigma, interval x0, interval T ) {
  return - sigma / gamma + ( x0 + sigma / gamma ) * exp ( gamma * T );
}

// Define the analytical solution 
inline std::vector < interval > timemap ( std::vector < double > gamma, std::vector < double > sigma, 
                                   std::vector < interval > x0, interval T ) {
  std::vector < interval > result;
  int dim = gamma.size();
  for ( unsigned int i=0; i<dim; ++i ) {
    result . push_back ( - sigma[i] / gamma[i] + ( x0[i] + sigma[i] / gamma[i] ) * exp ( gamma[i] * T ) );
  }
    // return - sigma / gamma + ( x0 + sigma / gamma ) * exp ( gamma * T );
  return result;
}



// compute the image of rect after a certain time T 
// the function above should be used instead
inline chomp::Rect timemap ( std::vector < double > gamma, std::vector < double > sigma, chomp::Rect rect, double T ) {
  int dim = rect.dimension();
  std::vector < double > lb, ub;
  for ( unsigned int i=0; i<dim; ++i ) {
    double x1 = rect . lower_bounds [ i ];
    double x2 = rect . upper_bounds [ i ];
    //
    double y1 = timemap ( gamma[i], sigma[i], x1, T );
    double y2 = timemap ( gamma[i], sigma[i], x2, T );
    //
    lb . push_back ( std::min ( y1, y2 ) );
    ub . push_back ( std::max ( y1, y2 ) );
  }
  return chomp::Rect ( dim, lb, ub );
}


// Check if we have a time T, that goes from xs to xf
inline bool findTbool ( double gamma, double sigma, double xs, double xf ) { 
  double tmp = ( xf + sigma/gamma ) / ( xs + sigma/gamma );
  if ( tmp>=0.0 && tmp<=1.0 ) {
    return true;
  } else {
    return false;
  }
}

// Return the time T to go from xs to xf
inline double findT ( double gamma, double sigma, double xs, double xf ) {
  double tmp = ( xf + sigma/gamma ) / ( xs + sigma/gamma );
  return log ( tmp ) / gamma;
}



class BooleanChartMap {

public:
	typedef uint64_t size_type;
  double lambda;
  BooleanChartMap ( void ) { }
  BooleanChartMap ( const chomp::Rect & rectangle, const char * string );
  BooleanChartMap ( int status, double gamma, double sigma, interval T ) : status_(status),gamma_(gamma),sigma_(sigma),T_(T) {}
	
  // New Map interface : Now the computation for T are done within Map
  BooleanChartMap ( int status, int imageid, std::vector<double> gamma, std::vector<double> sigma, Face face1, Face face2 ) : status_(status),imageid_(imageid),gamma_(gamma),sigma_(sigma),face1_(face1),face2_(face2) {}


  // AtlasGeo operator ( ) ( boost::shared_ptr < Geo > geo_ptr ) const {
	//  const AtlasGeo & geo = * boost::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
	//  return operator ( ) ( geo );
	// }

  // Here AtlasGeo geo is codimension-1
  AtlasGeo operator ( ) ( const AtlasGeo & geo ) const {
    std::vector < AtlasGeo > result;
  	RectGeo rectgeo = geo . rect ();

    int codim = rectgeo . lower_bounds . size();

    RectGeo rect0 ( codim );
    
    std::vector < interval > x, y;

    // first we reconstruct the full n-dimensional object from geo.
    std::vector < double > lbrg = rectgeo . lower_bounds;
    std::vector < double > ubrg = rectgeo . upper_bounds;
    std::vector < double >::iterator it;
    it = lbrg . begin();
    if ( face1_.direction == -1 ) {
      // std::cout << "we are dealing with a fixed point \n";
      it = lbrg.insert ( it, face1_.rect.lower_bounds[face1_.direction] );
      it = ubrg . begin();
      it = ubrg.insert ( it, face1_.rect.upper_bounds[face1_.direction] );
    } else { 
      it += face1_.direction;
      it = lbrg.insert ( it, face1_.rect.lower_bounds[face1_.direction] );
      it = ubrg . begin();
      it += face1_.direction;
      it = ubrg.insert ( it, face1_.rect.upper_bounds[face1_.direction] );
    }

    // Rect is n-dimensional, geo was n-1 dimensional
    chomp::Rect rect ( codim+1, lbrg, ubrg );

    // Normal case : 
    if ( status_ == 0 ) {
      int dir1 = face1_ . direction;
      int dir2 = face2_ . direction;

      double x1, x2; // starting points 
      double y1, y2; // images

      // // if faces are parallel, we just have to check from the images of the edges of rectangle of both faces
      // // they are both along the same dimension, and with the same ordering
      // // Both faces are of the from : x_i = cst and the rects are of the form : 
      // // I_0 x I_1 x I_2 x ... x I_(i-1) x I_(i+1) X ... I_n
      // //
      // // for each I above, check that f ( I ) |^| I non-empty and !={c} ( as to be true for all of them )
      // //
      if ( dir1 == dir2 ) {
        // std::cout << "dir1==dir2\n";
        x1 = face1_ . rect . lower_bounds [ dir1 ];
        y1 = face2_ . rect . lower_bounds [ dir1 ];
        // no need to check, we know T exists by construction of the map
        double T = findT( gamma_[dir2], sigma_[dir2], x1, y1 );
        // std::cout << "T : " << T <<"\n";
        // compute the image of the face 
        // std::cout << "rect : " << rect << "\n";
        std::vector < interval > ix, iy;
        for ( unsigned int i=0; i<codim+1; ++i ) {
          ix . push_back ( interval ( rect.lower_bounds[i], rect.upper_bounds[i] ) ); 
        }

        iy = timemap ( gamma_, sigma_, ix, interval(T,T) );

        std::vector < double > lb, ub;
        for ( unsigned int i=0; i<codim+1; ++i ) {
          // do not keep the image along the direction dir2
          if ( i != dir2 ) {
            lb . push_back ( iy[i].lower() );
            ub . push_back ( iy[i].upper() );
          } 
        }

        for ( unsigned int i=0; i<lb.size(); ++i ) {
          y . push_back ( interval ( lb [ i ], ub [ i ] ) );
        }

  // if the faces are not parallel  
      } else { 
        x1 = rect . lower_bounds [ dir2 ];
        x2 = rect . upper_bounds [ dir2 ];
        y1 = face2_.rect.lower_bounds[dir2];
        y2 = face2_.rect.lower_bounds[dir2];

        double T1 = findT ( gamma_[dir2], sigma_[dir2], x1, y1 );
        double T2 = findT ( gamma_[dir2], sigma_[dir2], x2, y2 );

          // face1 is for X_i = cst = a_i
          // face2 is for X_j = cst = b_j
          // the edge in the i-th direction on face1 gets map to an edge on face2 in the j-th direction.
          // the others edges direction do not change
          // to know which Ts to use, loop over the corner points
        std::vector < double > lb, ub;
        // std::vector < double > lb2, ub2;
        interval iT ( std::min(T1,T2), std::max(T1,T2) );
        std::vector < interval > is, iv;
        for ( unsigned int i=0; i<codim+1; ++i ) {
          is . push_back ( interval ( rect.lower_bounds [ i ], rect.upper_bounds [ i ] ) );
        }
        // image using interval arithmetic
        iv = timemap ( gamma_, sigma_, is, iT );
        //
        std::vector < interval > myimage;
        for ( unsigned int i=0; i<codim+1; ++i ) {
          if ( i != face2_ . direction ) {
            myimage . push_back ( iv [ i ] );
          }
        }
        // now construct the bounds for myimage
        // size of myimage should be equal to dim
        for ( unsigned int i=0; i<myimage.size(); ++i ) {
          lb . push_back ( myimage[i].lower() );
          ub . push_back ( myimage[i].upper() );
        }

        for ( unsigned int i=0; i<lb.size(); ++i ) {
          y . push_back ( interval ( lb [ i ], ub [ i ] ) );
        }

      }
    }

    if ( status_ == 1) {    //
      // Identity Maps
      // std::cout << "perform an identity map\n";
      for ( unsigned int i=0; i<codim; ++i ) {
        y . push_back ( interval ( rectgeo.lower_bounds[i], rectgeo.upper_bounds[i] ) );
      }
     } 
    if ( status_ == 2 ) {
      // Map to a fixed point given by sigma_, gamma_ values 
      // std::cout << "mapping to a fixed point\n";
      for ( unsigned int i=1; i<codim+1; ++i ) {
        y . push_back ( interval ( -sigma_[i]/gamma_[i], -sigma_[i]/gamma_[i] ) );
      }
    }
    if ( status_ == 3 ) {
      // Map to a GridElement that cannot be covered
      // used when a GridElement is mapped outside the phase space
      // std::cout << "mapping outside the phase space\n";
      for ( unsigned int i=0; i<codim; ++i ) {
        y . push_back ( interval ( 1E+37, 1E+37 ) );
      }
    }
    //
    //
    
      for ( unsigned int i=0; i<codim; ++i ) { 
        rect0 . lower_bounds [ i ] = y [ i ] . lower ( );
        rect0 . upper_bounds [ i ] = y [ i ] . upper ( );
      }
    
    //
    return AtlasGeo ( imageid_, rect0 );
}


bool good ( void ) const { return true; }

private:
  //
  // status = 0 : normal case 
  // status = 1 : Identity Map (parameters ignored)
  // status = 2 : Map to a fixed point given by -sigma_/gamma_
  // status = 3 : Map to a GridElement that cannot be covered
  int status_;
  int imageid_;
  std::vector < double > gamma_;
  std::vector < double > sigma_;
  Face face1_, face2_;

  interval T_;
};


typedef AtlasMap<BooleanChartMap> ModelMap;

#endif