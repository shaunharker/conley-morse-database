/// BooleanSwitchingMaps.h
/// Author: Shaun Harker
/// Date: August 20, 2014

#ifndef BOOLEANSWITCHINGMAPS_H
#define BOOLEANSWITCHINGMAPS_H

#include <vector>
#include "boost/foreach.hpp"
#include <utility>

std::vector < std::pair < int64_t, int64_t > >
BooleanSwitchingMaps ( std::vector<int64_t> const& closestface );

inline std::vector < std::pair < int64_t, int64_t > >
BooleanSwitchingMaps ( std::vector<int64_t> const& closestface ) {
  std::vector < std::pair < int64_t, int64_t > > result;
  std::vector < int64_t > cold; 
  std::vector < int64_t > hot;
  for ( int64_t d = 0; d < closestface . size (); ++ d ) {
    int64_t axis = d + 1;
    switch ( closestface [ d ] ) {
    case 0:
      hot . push_back ( - axis );
      cold . push_back ( axis );
      break;
    case 1:
      cold . push_back ( -axis );
      cold . push_back ( axis );
      break;
    case 2:
      cold . push_back ( -axis );
      hot . push_back ( axis );
      break; 
    }
  }
  if ( hot . empty () ) {
    cold . push_back ( 0 );
    hot . push_back ( 0 );
  }
  BOOST_FOREACH ( int64_t c, cold ) {
    BOOST_FOREACH ( int64_t h, hot ) {
      result . push_back ( std::make_pair ( c, h ) );
    }
  }
  return result;
}

#endif
