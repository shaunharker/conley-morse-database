#ifndef CMDB_MAPEVALS_H
#define CMDB_MAPEVALS_H

// include headers that implement a archive in simple text format
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/vector.hpp>
#include "chomp/Rect.h"
#include "chomp/Prism.h"

class MapEvals {
 private:
  typedef chomp::Rect ValType;
  friend class boost::serialization::access;
  chomp::Rect parameter_;
  std::vector < chomp::Rect > arguments_;
  std::vector < ValType > values_;
 public:
  
  void insert ( const chomp::Rect & rect ) {
    arguments_ . push_back ( rect );
    values_ . push_back ( ValType () );
  }
  chomp::Rect & parameter ( void ) {
    return parameter_;
  }
  
  chomp::Rect arg ( size_t i ) const {
    return arguments_ [ i ];
  }
  ValType & val ( size_t i ) {
    return values_ [ i ];
  }
  size_t size ( void ) const {
    return arguments_ . size ();
  }
  
template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & parameter_;
    ar & arguments_;
    ar & values_;
  }

 void load ( const char * filename ) {
   std::ifstream ifs(filename);
   boost::archive::text_iarchive ia(ifs);
   ia >> *this;
   // 
 }
 void save ( const char * filename ) {
   std::ofstream ofs(filename);
   boost::archive::text_oarchive oa(ofs);
   oa << *this;
   // oa closes ofs
 }

};
#endif
