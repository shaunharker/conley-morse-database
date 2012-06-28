// MapSubdivider.h

#ifndef CMDB_MAPSUBDIVIDER_H
#define CMDB_MAPSUBDIVIDER_H

#include "chomp/Rect.h"
#include <vector>

template < class Map, int K >
class MapSubdivider {
 private:
  Map f;
  template < class InsertIterator >
  void compute ( InsertIterator & ii, const chomp::Rect & r, int depth ) const {
    if ( depth == 0 ) {
      * ii ++ = f ( r );
    } else {
      int D = r . lower_bounds . size ();
      long N = 1 << D;
      for ( long i = 0; i < N; ++ i ) {
        chomp::Rect q ( D );
        for ( int d = 0; d < D; ++ d ) {
          if ( i & ( ((long)1) << d ) ) {
            q . lower_bounds [ d ] = r . lower_bounds [ d ];
            q . upper_bounds [ d ] = r . lower_bounds [ d ] + (r.upper_bounds[d]-r.lower_bounds[d])/2.0;
          } else {
            q . lower_bounds [ d ] = r . lower_bounds [ d ] + (r.upper_bounds[d]-r.lower_bounds[d])/2.0;
            q . upper_bounds [ d ] = r . upper_bounds [ d ];
          }
          
        }
        compute ( ii, q, depth - 1 );
      }
    }
  }
 public:
  MapSubdivider ( const chomp::Rect & r ) : f(r) {}

  std::vector<chomp::Rect> operator () ( const chomp::Rect & r ) const {
    std::vector < chomp::Rect > result;
    std::insert_iterator < std::vector < chomp::Rect > > ii ( result, result . end () );
    compute ( ii, r, K );
    return result; 
  }
};
#endif
