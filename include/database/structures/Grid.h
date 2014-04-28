// Grid.h
#ifndef CMDB_GRID_H
#define CMDB_GRID_H

#include <stdint.h>
#include <memory>
#include <vector>
#include <stack>
#include <deque>

#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/unordered_set.hpp>
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/shared_ptr.hpp"
#include "database/structures/Geo.h"

// Declaration
class Grid {
public:
  // Typedefs
  typedef uint64_t GridElement;
  typedef boost::counting_iterator < GridElement > iterator;
  typedef iterator const_iterator;
  typedef uint64_t size_type;
  
  // Virtual Methods
  virtual Grid * clone ( void ) const = 0;
  virtual void subdivide ( void ) = 0;
  virtual Grid * subgrid ( const std::deque < GridElement > & grid_elements ) const = 0;
  virtual std::vector<Grid::GridElement> subset ( const Grid & other ) const = 0;
  virtual boost::shared_ptr<Geo> geometry ( GridElement ge ) const = 0;  
  virtual std::vector<Grid::GridElement> cover ( const Geo & geo ) const = 0;

  // Container methods
  iterator find ( GridElement ge ) const;
  iterator begin ( void ) const;
  iterator end ( void ) const;
  size_type size ( void ) const;
  
  // Additional Features

  boost::shared_ptr<Geo> geometry ( const const_iterator & it ) const { return geometry ( *it ); }
  // Cover a union (given as a vector)
  template < class T >
  inline std::vector<Grid::GridElement> cover ( const std::vector < T > & V ) const;
  
  // Cover an intersection (given as a pair)
  template < class S, class T >
  std::vector<Grid::GridElement> cover ( const std::pair < S, T > & V ) const;
  
  // Return memory usage of this data structure
  virtual uint64_t memory ( void ) const = 0;

  // Constructor, Deconstructor
  protected: 
  Grid ( void );
  public: 
  virtual ~Grid ( void );  

// Data
protected:
  size_type size_;
  //chomp::Rect bounds_;
  //int dimension_;
  //std::vector < bool > periodic_;

// Serialization
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & size_;
    //ar & bounds_;
    //ar & dimension_;
    //ar & periodic_;
  }
  
};

// DEFINITIONS



// Container Methods

inline Grid::iterator Grid::find ( GridElement find_me ) const {
  return iterator ( find_me );
} /* Grid::find */

inline Grid::iterator Grid::begin ( void ) const {
  return iterator ( 0 );
} /* Grid::begin */

inline Grid::iterator Grid::end ( void ) const {
  return iterator ( size () );
} /* Grid::end */

inline Grid::size_type Grid::size ( void ) const {
  return size_;
} /* Grid::size */


// UNION version of cover
template < class T >
inline std::vector<Grid::GridElement> Grid::cover ( const std::vector < T > & V ) const {
  std::vector<Grid::GridElement> result;
  boost::unordered_set<Grid::GridElement> result_set;
  BOOST_FOREACH ( const T & geo, V ) {
    std::vector<Grid::GridElement> cover_vec = cover ( geo );
    result_set . insert ( cover_vec . begin (), cover_vec . end () );
  }
  result . insert ( result . end (), result_set . begin (), result_set . end () );
  return result;
}

// INTERSECTION (pair) for cover
template < class S, class T >
inline std::vector<Grid::GridElement> Grid::cover ( const std::pair < S, T > & V ) const {
  // Cover V . first, store in "firstcover"
  std::vector<Grid::GridElement> firstcover_vec = cover ( V . first );
  boost::unordered_set < GridElement > firstcover (firstcover_vec . begin (),
                                                   firstcover_vec . end () );
  // Cover V . second, store in "secondcover"
  std::vector<Grid::GridElement> secondcover_vec = cover ( V . second );
  boost::unordered_set < GridElement > secondcover (secondcover_vec . begin (),
                                                    secondcover_vec . end () );

  // Without loss, let firstcover be no smaller than secondcover.
  if ( firstcover . size () < secondcover. size () ) std::swap ( firstcover, secondcover );
  // Compute intersection by checking if each element in smaller cover is in larger cover
  std::vector<Grid::GridElement> result;
  BOOST_FOREACH ( const GridElement & ge, secondcover ) {
    if ( firstcover . count ( ge ) ) {
      result . push_back ( ge );
    }
  }
  return result;
}

inline Grid::Grid ( void ) {
  size_ = 1;
}

inline Grid::~Grid ( void ) {
}

#endif
