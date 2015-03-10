/// externalUnionFind.h
/// Author: Shaun Harker
/// Date: January 28, 2015

#ifndef EXTERNAL_UNION_FIND_H
#define EXTERNAL_UNION_FIND_H

#define USE_STXXL
#define INTERNAL_MEMORY_FOR_SORTER 4000000000LL
#include "stxxl.h"

/// Declarations

/// externalUnionFind
/// input: a relation R with two fields "first" and "second"
/// output: a relation with two fields "vertex" and "component" (x, p(x) ) 

template < class T >
Relation 
externalUnionFind ( T & input );

/// sort_merge_join
///   Sort A by field1 and B by field2.
///   Find all pairs of rows in A and rows in B where field1 in A matches field2 in B
///   For such pairs, create new rows by performing the join. Use map1 and map2 to rename
///   fields. If map1 or map2 fails to contain some field, discard it in the result.
///   NOTE: The current implementation assumes the join-key is UNIQUE for either A or B.
template < class S, class T >
Relation
sort_merge_join ( S & A, std::string const& field1,
                  T & B, std::string const& field2,
                  std::unordered_map<std::string, std::string> const& map1,
                  std::unordered_map<std::string, std::string> const& map2 ) {
  // Create resulting relation with appropriate fields
  Relation result;
  for ( auto const& pair : map1 ) result . insert_field ( pair . second );
  for ( auto const& pair : map2 ) result . insert_field ( pair . second );
  
  // Sort relation A by field1 and relation B by field2
  database::sort ( A, field1 );
  database::sort ( B, field2 );

  size_t field1_id = A . field_id ( field1 );
  size_t field2_id = B . field_id ( field2 );

  Relation::iterator it1 = A . begin ();
  Relation::iterator it2 = B . begin ();

  while ( it1 != A . end () && it2 != B . end () ) {
    Relation::Cell cell1 = it1 -> cell ( field1_id );
    Relation::Cell cell2 = it2 -> cell ( field2_id );
    if ( cell1 < cell2 ) ++ it1;
    if ( cell2 < cell1 ) ++ it2;
    if ( cell1 == cell2 ) { 
      result . insert ( database::join ( *it1, *it2, map1, map2 ) );
    }
  }
  return result;
}

/// Definitions
template < class T >
Relation 
externalUnionFind ( T & input ) {
  size_t N = input . size ();
  boost::joined_range< Relation, Relation> result;
  if ( N < 67108864L ) { // 1GB of memory for 16 byte records.
    // Internal Memory Union-Find.
    // TODO
  } else {
    boost::iterator_range input1 = boost::make_iterator_range( input, 0, N/2);
    boost::iterator_range input2 = boost::make_iterator_range( input, N/2, N);
    Relation R1 = externalUnionFind ( input1 );
    sort_merge_join ( input2, R1, 0 );
    sort_merge_join ( input2, R1, 1 );
    Relation R2 = externalUnionFind ( input2 );
    sort_merge_join ( R1, R2, 1 );
    result = boost::join ( R1, R2 );
  }
  return result;
}





#endif
