/// BooleanSwitchingMaps.h
/// Author: Shaun Harker
/// Date: August 20, 2014

#ifndef BOOLEANSWITCHINGMAPS_H
#define BOOLEANSWITCHINGMAPS_H

#include <vector>
#include "boost/foreach.hpp"
#include <utility>

std::vector < std::pair < int, int > >
BooleanSwitchingMaps ( std::vector<int> const& closestface );

inline std::vector < std::pair < int, int > >
BooleanSwitchingMaps ( std::vector<int> const& closestface ) {
  std::vector < std::pair < int, int > > result;
  std::vector < int > cold; 
  std::vector < int > hot;
  for ( int d = 0; d < closestface . size (); ++ d ) {
    int axis = d + 1;
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
  BOOST_FOREACH ( int c, cold ) {
    BOOST_FOREACH ( int h, hot ) {
      result . push_back ( std::make_pair ( c, h ) );
    }
  }
  return result;
}

#endif
