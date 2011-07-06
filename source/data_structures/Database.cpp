/* Database */

#include <iostream>

#include <fstream>

#include "data_structures/Database.h"
#include "boost/foreach.hpp"


GridElementRecord::GridElementRecord ( void ) {}

ParameterBoxRecord::ParameterBoxRecord ( void ) {}

bool ParameterBoxRecord::operator < ( const ParameterBoxRecord & rhs ) const {
  return id_ < rhs . id_;
}

bool ClutchingRecord::operator < ( const ClutchingRecord & rhs ) const {
  if ( id1_ == rhs . id1_ ) return id2_ < rhs . id2_;
  return id1_ < rhs . id1_;
}

void Database::merge ( const Database & other ) {
  BOOST_FOREACH ( const ParameterBoxRecord & record, other . box_records_ ) {
    insert ( record );
  }
  BOOST_FOREACH ( const ClutchingRecord & record, other . clutch_records_ ) {
    insert ( record );
  }
}

void Database::insert ( const ParameterBoxRecord & record ) {
  box_records_ . insert ( record );
}

void Database::insert ( const ClutchingRecord & record ) {
  clutch_records_ . insert ( record );
}


void Database::save ( const char * filename ) {
  // make an archive
  std::ofstream ofs(filename);
  assert(ofs.good());
  ofs << (int) box_records_ . size ();
  BOOST_FOREACH ( const ParameterBoxRecord & record, box_records_ ) {
    ofs << record . id_;
    ofs << record . ge_ . lower_bounds_ . size ();
    BOOST_FOREACH ( double val, record . ge_ . lower_bounds_ ) {
      ofs << val;
    }
    BOOST_FOREACH ( double val, record . ge_ . upper_bounds_ ) {
      ofs << val;
    }    
    ofs << (int) record . partial_order_ . size ();
    typedef std::pair < int, int >  Edge;
    BOOST_FOREACH ( const Edge & edge, record . partial_order_ ) {
      ofs << edge . first;
      ofs << edge . second;
    }
  }
  ofs << (int) clutch_records_ . size ();
  BOOST_FOREACH ( const ClutchingRecord & record, clutch_records_ ) {
    ofs << record . id1_;
    ofs << record . id2_;
    ofs << (int) record . clutch_ . size ();
    typedef std::pair < int, int >  Edge;
    BOOST_FOREACH ( const Edge & edge, record . clutch_ ) {
      ofs << edge . first;
      ofs << edge . second;
    } 
  }
}

void Database::load ( const char * filename ) {
  // open the archive
  std::ifstream ifs(filename);
  assert(ifs.good());
  
}
