//Tree.h

#ifndef CMDB_TREE_H
#define CMDB_TREE_H

#include <memory>
#include <deque>
#include <boost/iterator/counting_iterator.hpp>
#include <stdint.h>
#include "boost/serialization/serialization.hpp"

class Tree {
public:
  
  // Typedefs
  typedef boost::counting_iterator < uint64_t > iterator;
  typedef iterator const_iterator;
  typedef uint64_t value_type;
  typedef uint64_t size_type;
  
  // Constructor/Deconstructor Methods
protected:
  Tree ( void );
public:
  virtual ~Tree ( void );
  
  // Container methods
  iterator begin ( void ) const;
  iterator end ( void ) const;
  size_type size ( void ) const ;

  // Iteration methods
  virtual iterator parent ( iterator it ) const = 0;
  virtual iterator left ( iterator it ) const = 0;
  virtual iterator right ( iterator it ) const = 0;
  
  // Query methods
  virtual bool isleft ( iterator it ) const = 0;
  virtual bool isright ( iterator it ) const = 0;
  virtual bool isleaf ( iterator it ) const = 0;
  size_type depth ( iterator it ) const;
 
  // Builder Methods
  virtual void subdivide ( void ) = 0;
  virtual void adjoin ( const Tree & other ) = 0;
  virtual Tree * subtree ( const std::deque < Tree::iterator > & leaves ) const = 0;
  
  // Test and Debug
  virtual uint64_t memory ( void ) const = 0;
  virtual void debug ( void ) const = 0;
  
protected:
  uint64_t size_;
  
private:
  // Serialization Methods
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & size_;
  }
};

inline Tree::Tree ( void ) {
  size_ = 1;
}

inline Tree::~Tree ( void ) {}

inline Tree::iterator Tree::begin ( void ) const {
  return iterator ( 0 );
}

inline Tree::iterator Tree::end ( void ) const {
  return iterator ( size () );
}

inline Tree::size_type Tree::size ( void ) const {
  return size_;
}

inline Tree::size_type Tree::depth ( iterator it ) const {
  size_type result = 0;
  while ( it != begin () ) {
    ++ result;
    it = parent ( it );
  }
  return result;
}

#endif
