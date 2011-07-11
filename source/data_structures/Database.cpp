/* Database */

#include <iostream>

#include <fstream>

#include "data_structures/Database.h"
#include "boost/foreach.hpp"


GridElementRecord::GridElementRecord ( void ) {}
GridElementRecord::GridElementRecord ( int dimension ) {
  lower_bounds_ . resize ( dimension );
  upper_bounds_ . resize ( dimension );
}

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
  std::ofstream ofs(filename, std::ios::out | std::ios::binary);
  assert(ofs.good());
  ofs << (int) box_records_ . size ();
  BOOST_FOREACH ( const ParameterBoxRecord & record, box_records_ ) {
    ofs << record . id_;
    ofs << record . num_morse_sets_;
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
  typedef std::pair < int, int >  Edge;
  // open the archive
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  if( not ifs.good() ) {
    std::cout << "Failed to open " << filename << "\n";
    exit ( 1 );
  }

  int number_of_box_records;
  ifs >> number_of_box_records;
  std::cout << "Number of Parameter Box Records = " << number_of_box_records << "\n";
  for ( int i = 0; i < number_of_box_records; ++ i ) {
    ParameterBoxRecord record;
    int dimension;
    ifs >> record . id_;
    ifs >> record . num_morse_sets_;
    ifs >> dimension;
    record . ge_ = GridElementRecord ( dimension );
    for ( int d = 0; d < dimension; ++ d ) ifs >> record . ge_ . lower_bounds_ [ d ];
    for ( int d = 0; d < dimension; ++ d ) ifs >> record . ge_ . upper_bounds_ [ d ];
    int number_of_edges;
    ifs >> number_of_edges;
    for ( int j = 0; j < number_of_edges; ++ j ) {
      Edge edge;
      ifs >> edge . first;
      ifs >> edge . second;
      record . partial_order_ . push_back ( edge );
    }
    insert ( record );
  }
  
 
  int number_of_clutch_records;
  ifs >> number_of_clutch_records;
  std::cout << "Number of Clutching Records = " << number_of_clutch_records << "\n";

  for ( int i = 0; i < number_of_clutch_records; ++ i ) {
    ClutchingRecord record;
    ifs >> record . id1_;
    ifs >> record . id2_;
    int number_of_edges;
    ifs >> number_of_edges;
    for ( int j = 0; j < number_of_edges; ++ j ) {
      Edge edge;
      ifs >> edge . first;
      ifs >> edge . second;
      record . clutch_ . push_back ( edge );
    }
    insert ( record );
  }
  
}
