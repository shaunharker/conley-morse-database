/// Relation.h
/// Author: Shaun Harker
/// Date: March 6, 2015

#ifndef RELATION_H
#define RELATION_H

#include <cstdint>
#include <vector>
#include <string>

#define USE_STXXL
#define INTERNAL_MEMORY_FOR_SORTER 4000000000LL
#include "stxxl.h"

namespace database {

class Cell {

};

/// Expose an interface that appears like std::vector<Cell>
/// but uses external memory.
class CellData {
// TODO
};

class Row {
  std::vector<Cell> data_;
  // TODO
};

/// class Scheme
///   This class gives a description of the fields associated
///   with a relation and the manner in which they are mapped to columns.
///   It essentially implements a bijection {0,1,2,...,k} to a collection of 
///   string names.
class Scheme {
  std::vector<std::string> fields_;
  std::unordered_map<std::string, int64_t> column_;
public:
  /// insert
  ///   Insert a field into the relation
  int64_t insert ( std::string const& field_name );

  /// column
  ///   Give the column number associated with a field
  int64_t column ( std::string const& field_name ) const;

  /// field
  ///   Give the field name associated with a column index
  std::string field ( int64_t column_index ) const;

  /// fields
  ///   Return container of the fields
  auto & fields ( void ) const; 
};

inline int64_t 
Scheme::insert ( std::string const& field_name ) {
  if ( column_ . count ( field_name ) ) {
    return column ( field_name );
  }
  column_ [ field_name ] = fields_ . size ();
  fields_ . push_back ( field_name );
  return column ( field_name );
}

inline int64_t 
Scheme::column ( std::string const& field_name ) const {
  return column_ [ field_name ];
}

inline std::string 
Scheme::field ( int64_t column_index ) const {
  return fields_ [ column_index ];
}

inline auto &
Scheme::fields {
  return fields_;
}

/// class Relation
class Relation {
  Scheme scheme_;
  CellData data_;
public:

  /// scheme
  ///   Give read/write access to scheme
  Scheme &
  scheme ( void );

  Scheme const&
  scheme ( void ) const;

  /// insert
  ///   Insert a field into the relation
  int64_t 
  insert ( Row const& row );
  
  /// sort
  ///   Sort the rows by some column
  void
  sort ( std::string const& field_name );
};


inline Scheme &
Relation::scheme ( void ) {
  return scheme_;
}

inline Scheme const&
Relation::scheme ( void ) const {
  return scheme_;
}

inline int64_t 
Relation::insert ( Row const& row ) {
  // TODO
}
  
inline int64_t 
Relation::column ( std::string const& field_name ) const {
  return column_ [ field_name ];
}

/// Algorithms

inline void 
sort ( Relation const& R, std::string const& field ) {

}

/// class JoinScheme
///   We specify two relations 
class JoinScheme {
private:
  std::vector<int64_t> const& columns1;
  std::vector<int64_t> const& columns2;
  JoinScheme ( Relation const& A, 
               Relation const& B, 
               std::unordered_map<std::string, std::string> const& map1,
               std::unordered_map<std::string, std::string> const& map2 ) {
    columns1 . resize ( A . fields () . size () );
    columns2 . resize ( B . fields () . size () );
    Relation C;
    for ( auto const& field : A . fields () ) C . insert ( field );
    for ( auto const& field : B . fields () ) C . insert ( field );
  
    for ( auto const& pair : map1 ) {
      columns1 [ A . column ( pair . first ) ] = 
        C . column ( pair . second );
    }
    for ( auto const& pair : map2 ) {
      columns2 [ B . column ( pair . first ) ] = 
        C . column ( pair . second );
    }
  }
  Row join ( Row const& r1, 
             Row const& r2 ) {
    Row result;
    for ( int64_t i : columns1 ) result . push_back ( r1 [ i ] );
    for ( int64_t i : columns2 ) result . push_back ( r2 [ i ] );
    return result;
  }
};

} // namespace database

#endif

