//Tree.h

#ifndef CMDB_TREE_H
#define CMDB_TREE_H

#include <memory>
#include <deque>
#include <boost/iterator/counting_iterator.hpp>
#include <stdint.h>
#include "boost/serialization/serialization.hpp"

#include "database/structures/CompressedTree.h" // later i may refactor

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
  virtual void assign ( const CompressedTree & compressed ) = 0;

  template < class InputIterator >
  static CompressedTree * join ( InputIterator start, InputIterator stop );
  
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

/** Tree::join
 *     InputIterator should be passed as begin() and end() of a container
 *     of pointers to Trees. (Either raw pointers or smart pointers will work.)
 *     The result is the balanced parantheses list of the join of all trees.
 *     The join is defined to be the smallest binary tree for which each of the 
 *     input trees is a subtree.
 */
template < class InputIterator >
CompressedTree * Tree::join ( InputIterator start, InputIterator stop ) {
  CompressedTree * result = new CompressedTree;
  
  std::vector<bool> & balanced_parentheses = result -> balanced_parentheses;
  std::vector<bool> & valid_tree_nodes = result -> valid_tree_nodes;
  
  //std::cout << "(1";

  balanced_parentheses.push_back ( true );
  valid_tree_nodes.push_back ( true );
  typedef Tree::iterator iterator;
  
  // How this works:
  // We want to advance through the trees simultaneously, but they aren't all the same tree
  // If we explore a subtree in some trees that does not exist in others, we remain halted on the others
  // until the subtree finishes.
  //
  // initialize iterators
  
  // State machine:
  // State 0: Try to go left. If can't, set success to false and try to go right. Otherwise success is true and try to go left on next iteration.
  // State 1: Try to go right. If can't, set success to false and rise. Otherwise success is true and try to go left on next iteration.
  // State 2: Rise. If rising from the right, rise again on next iteration. Otherwise try to go right on the next iteration.
  std::vector < iterator > iterators;
  for ( InputIterator it = start; it != stop; ++ it ) {
    iterators . push_back ( (*start) -> begin () );
  }
  size_t N = iterators . size ();
  std::vector < size_t > iterator_depth ( N );
  size_t depth = 0;
  int state = 0;  
  while ( 1 ) {
    if ( (depth == 0) && ( state == 2 ) ) break;
    //std::cout << "Position 0. depth = " << depth << " and state = " << state << "\n";
    bool success = false;
    size_t i = 0;
    for ( InputIterator it = start; it != stop; ++ it ) {
      //std::cout << "Position 1. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
      // If node is halted, continue
      if ( iterator_depth[i] == depth ) {
        iterator end = ( *it ) -> end ();
        switch ( state ) {
          case 0:
          {
            iterator left = ( *it ) -> left ( iterators[i] );
            if ( left == end ) break;
            iterators[i] = left;
            ++ iterator_depth[i];
            success = true;
            break;
          }
          case 1:
          {
            iterator right = ( *it ) -> right ( iterators[i] );
            if ( right == end ) break;
            iterators[i] = right;
            ++ iterator_depth[i];
            success = true;
            break;
          }
          case 2:
          {
            if ( ( *it ) -> isright ( iterators[i] ) ) success = true;
            iterators[i] = ( *it ) -> parent ( iterators[i] );
            -- iterator_depth[i];
            break;
          }
        }
      }
    ++ i;
    }
    
    //std::cout << "Position 3. depth = " << depth << " and state = " << state << "\n";
    //std::cout << ( success ? "success" : "failure" );
    switch ( state ) {
      case 0: // Tried to go left
        if ( success ) {
          balanced_parentheses . push_back ( true );
          valid_tree_nodes . push_back ( true );
          //std::cout << "(1";
          // Success. Try to go left again.
          state = 0;
          ++ depth;
        } else {
          // Failure. Try to go right instead.
          state = 1;
        }
        break;
      case 1: // Tried to go right
        if ( success ) {
          // Check if left branch exists
          if ( balanced_parentheses . back () ) {
            // It doesn't. Make a fake leaf as placeholder.
            valid_tree_nodes . push_back ( false );
            balanced_parentheses . push_back ( true );
            balanced_parentheses . push_back ( false );
            //std::cout << "(0)";
          }
          balanced_parentheses . push_back ( true );
          valid_tree_nodes . push_back ( true );
          //std::cout << "(1";
          // Sucess. Try to go left now.
          state = 0;
          ++ depth;
        } else {
          // Check if left branch exists
          if ( not balanced_parentheses . back () ) {
            // It does. So make a fake leaf as placeholder for missing right.
            balanced_parentheses . push_back ( true );
            balanced_parentheses . push_back ( false );
            valid_tree_nodes . push_back ( false ); // right node not valid
            //std::cout << "(0)";
          }
          // Failure. Rise.
          state = 2;
        }
        break;
      case 2: // Rose
        -- depth;
        balanced_parentheses . push_back ( false );
        //std::cout << ")";
        if ( success ) {
          // Rose from right, continue to rise
          state = 2;
        } else {
          // Rose from left, try to go right
          state = 1;
        }
        break;
    }
    
  }
  balanced_parentheses.push_back ( false );
  //std::cout << ")\n\n";
  //std::cout << balanced_parentheses . size () << "\n";
  //std::cout << valid_tree_nodes . size () << "\n";

  return result;
}

#endif
