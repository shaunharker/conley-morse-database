#ifndef BOOLEANSWITCHING_WALL_H
#define BOOLEANSWITCHING_WALL_H

#include <vector>
#include <cmath>

#include <boost/serialization/serialization.hpp>
#include <boost/functional/hash.hpp>
#include "database/structures/RectGeo.h"

// Wall class. 
// Define a chart for the boolean switching network

// Here a chart is a codimension 1 face 
// but it is represented by its n set of coordinates.
// to get the corresponding Codimension 1 Wall
// simply called the reducedWall method
// it will throw away the degenerate directions

class Wall { 
public:
  Wall ( void ) {}
  Wall ( int face, 
         std::vector<int> const& domain );

  // store the geometrical information of the chart
  // convention : 
  // [0,1] means the chart is spanning [ -infinity, 1st threshold ]
  // [3,5] means the chart is spanning [ 3rd threshold, 5th threshold ]
  // if we have 5 thresholds
  // [5,6] means the chart is spanning [ 5th threshold, +infinity ] 

  /// rect
  ///   return underlying RectGeo object
  const RectGeo & rect ( void ) const;
  RectGeo & rect ( void );

  //
  // return true if the chart is actually a fixed point
  // convention for a fixed point : 
  // rect represent the subdomain. (no degenerate direction)
  bool isFixedPoint ( void ) const;

  // return the corresponding chart without the degenerate direction
  // useful when adding a chart to the atlas
  RectGeo reducedRect ( void ) const;

//
  friend std::size_t hash_value ( Wall const & chart ) {
  std::size_t seed = 0;
  
  std::vector < double > const& lbounds = chart.rect().lower_bounds;
  std::vector < double > const& ubounds = chart.rect().upper_bounds;

  for ( unsigned int i = 0; i < chart.rect().dimension(); ++i ) {
    boost::hash_combine ( seed, lbounds[i] );
    boost::hash_combine ( seed, ubounds[i] );
  }
  return seed;
  }

// to compare charts, we do not check the id number
  bool operator==(const Wall &other) const {
    std::vector < double > const& lbounds1 = rect_.lower_bounds;
    std::vector < double > const& ubounds1 = rect_.upper_bounds;
    std::vector < double > const& lbounds2 = other.rect_.lower_bounds;
    std::vector < double > const& ubounds2 = other.rect_.upper_bounds;

    // just check against lbounds1&2 (should be enough)
    if ( lbounds1.size() != lbounds2.size() ) { return false; }

    int dim = lbounds1.size();
    for ( unsigned int i = 0; i<dim; ++i ) { 
      if ( std::abs(lbounds1[i]-lbounds2[i]) > 1e-12 ) { return false; }
      if ( std::abs(ubounds1[i]-ubounds2[i]) > 1e-12 ) { return false; }
    }
    return true;
  }

private:
  RectGeo rect_;
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
  ar & rect_;
  }

};

inline Wall::Wall ( int face, 
                    std::vector<int> const& domain ) {
  // dimension of the phase space
  int dimension = domain. size();

  std::vector < double > lbounds;
  std::vector < double > ubounds;

  // if face is a fixed point
  if ( face == 0 ) {
    for ( unsigned int i = 0; i<dimension; ++i ) {
      lbounds . push_back ( domain[i] );
      ubounds . push_back ( domain[i]+1 );
    }
  } else {
    for ( unsigned int i = 0; i<dimension; ++i ) {
      if ( i == abs(face) - 1 ) {
        if ( face > 0 ) {
          lbounds . push_back ( domain[i]+1 );
          ubounds . push_back ( domain[i]+1 );
        } else {
          lbounds . push_back ( domain[i] );
          ubounds . push_back ( domain[i] );
        }
      } else { 
        lbounds . push_back ( domain[i] );
        ubounds . push_back ( domain[i]+1 );
      }
    }
  }
  rect_ = RectGeo ( dimension, lbounds, ubounds );
}

inline RectGeo const& 
Wall::rect ( void ) const {
  return rect_;
}

inline RectGeo & 
Wall::rect ( void ) {
  return rect_;
}

inline bool 
Wall::isFixedPoint ( void ) const {
  bool output = true;
  std::vector < double > const& lbounds = rect_ . lower_bounds;
  std::vector < double > const& ubounds = rect_ . upper_bounds;
  int dim = lbounds . size();
  for ( int i = 0; i < dim; ++ i )  {
    // if at least one direction is not degenerate -> not a fixed point
    if ( abs(lbounds[i]-ubounds[i]) > 1e-12 ) {
      output = false;
      break;
    }
  }
  return output;
}

inline RectGeo 
Wall::reducedRect ( void ) const {
  std::vector < double > const& lbounds = rect_.lower_bounds;
  std::vector < double > const& ubounds = rect_.upper_bounds;
  int dim = lbounds . size ( );
  std::vector < double > newlbounds, newubounds;
  for ( int i = 0; i < dim; ++i ) {
    if ( abs (lbounds[i]-ubounds[i]) > 1e-12 ) {
      newlbounds . push_back ( lbounds[i] );
      newubounds . push_back ( ubounds[i] );
    }
  }
  if ( dim == newlbounds . size () ) return RectGeo ( 0 ); // Interior point.
  dim = newlbounds . size ();
  return RectGeo ( dim, newlbounds, newubounds );
}

#endif
