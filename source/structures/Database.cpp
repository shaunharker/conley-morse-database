/* Database */

#include <iostream>

#include <fstream>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>


#include <cstdio> // remove
#include <boost/config.hpp>
#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{
  using ::remove;
}
#endif

#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>


#include "database/structures/Database.h"
#include "boost/foreach.hpp"


// record constructors
GridElementRecord::GridElementRecord ( void ) {}
GridElementRecord::GridElementRecord ( int dimension ) {
  lower_bounds_ . resize ( dimension );
  upper_bounds_ . resize ( dimension );
}

ParameterBoxRecord::ParameterBoxRecord ( void ) {}

// record comparison
bool ParameterBoxRecord::operator < ( const ParameterBoxRecord & rhs ) const {
  return id_ < rhs . id_;
}

bool ClutchingRecord::operator < ( const ClutchingRecord & rhs ) const {
  if ( id1_ == rhs . id1_ ) return id2_ < rhs . id2_;
  return id1_ < rhs . id1_;
}

bool ConleyRecord::operator < ( const ConleyRecord & rhs ) const {
  if ( id_ . first == rhs . id_ . first ) return id_ . second < rhs . id_ . second;
  return id_ . first < rhs . id_ . first;
}

// database merging
void Database::merge ( const Database & other ) {
  BOOST_FOREACH ( const ParameterBoxRecord & record, other . box_records_ ) {
    insert ( record );
  }
  BOOST_FOREACH ( const ClutchingRecord & record, other . clutch_records_ ) {
    insert ( record );
  }
  BOOST_FOREACH ( const ConleyRecord & record, other . conley_records_ ) {
    insert ( record );
  }
}

// record insertion
void Database::insert ( const ParameterBoxRecord & record ) {
  box_records_ . insert ( record );
}

void Database::insert ( const ClutchingRecord & record ) {
  clutch_records_ . insert ( record );
}

void Database::insert ( const ConleyRecord & record ) {
  conley_records_ . insert ( record );
}

// record access
std::set < ParameterBoxRecord > & Database::box_records ( void ) {
  return box_records_;
}
std::set < ClutchingRecord > & Database::clutch_records ( void ) {
  return clutch_records_;
}
std::set < ConleyRecord > & Database::conley_records ( void ) {
  return conley_records_;
}
const std::set < ParameterBoxRecord > & Database::box_records ( void ) const {
  return box_records_;
}
const std::set < ClutchingRecord > & Database::clutch_records ( void ) const {
  return clutch_records_;
}
const std::set < ConleyRecord > & Database::conley_records ( void ) const {
  return conley_records_;
}

// file operations
void Database::save ( const char * filename ) {
    std::cout << "Database SAVE\n";
  std::ofstream ofs(filename);
  assert(ofs.good()); 
  boost::archive::xml_oarchive oa(ofs);
  oa << boost::serialization::make_nvp("database", * this);
  //ofs . close ();
}

void Database::load ( const char * filename ) {
  //std::cout << "Database LOAD\n";
  std::ifstream ifs(filename);
  if ( not ifs . good () ) {
    std::cout << "Could not load " << filename << "\n";
    exit ( 1 );
  }
  boost::archive::xml_iarchive ia(ifs);
  // read class state from archive
  ia >> boost::serialization::make_nvp("database",*this);
  //ifs . close ();
}
