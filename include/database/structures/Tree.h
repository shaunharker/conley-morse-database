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
  virtual bool isLeft ( iterator it ) const = 0;
  virtual bool isRight ( iterator it ) const = 0;
  virtual bool isLeaf ( iterator it ) const = 0;
  size_type depth ( iterator it ) const;
 
  // Builder Methods
  //virtual void subdivide ( void ) = 0;
  virtual CompressedTree * subtree ( const std::deque < Tree::iterator > & leaves ) const;
  virtual void assign ( boost::shared_ptr<const CompressedTree> compressed ) = 0;


/** Tree::join
 *     InputIterator should be passed as begin() and end() of a container
 *     of pointers to Trees. (Either raw pointers or smart pointers will work.)
 *     The result is the CompressedTree object of the join of all trees.
 *     The join is defined to be the smallest binary tree for which each of the 
 *     input trees is a subtree. 
 *     There is some complexity here due to the valid_sequence complication.
 *     Since CompressedTree represents a binary tree via the "leaf_sequence" of
 *     a full binary tree and a valid_sequence over the leaves, what we need to do
 *     is create the full binary tree that is the join of the "joinands", and
 *     each leaf will be valid if it is valid for at least one of the joinands.
 */
  template < class InputIterator >
  static CompressedTree * join ( InputIterator start, InputIterator stop );
  
  // Test and Debug
  virtual uint64_t memory ( void ) const = 0;
  
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


inline CompressedTree * 
Tree::subtree ( const std::deque < iterator > & leaves ) const {

  const bool LEAF = false;
  const bool NOT_A_LEAF = true;
  const bool NOT_VALID = false;
  const bool VALID = true;

  CompressedTree * result = new CompressedTree;
  CompressedTree & new_tree = * result;
  std::vector < bool > & new_leaf_sequence
    = new_tree . leaf_sequence;
  std::vector < bool > & new_valid_sequence
    = new_tree . valid_sequence;

  if ( leaves . empty () ) {
    new_leaf_sequence . push_back ( LEAF );
    new_valid_sequence . push_back ( NOT_VALID );
    return result;
  }
  
  // Mark the subtree
  std::vector < bool > visited ( size (), false );
  visited [ 0 ] = true;
  BOOST_FOREACH ( iterator leaf, leaves ) {
    iterator it = leaf;
    while ( visited [ * it ] == false ) {
      visited [ * it ] = true;
      it = parent ( it );
    }
  }
 
  // Now walk through visited nodes and write leaf_sequence and valid_sequence 
  // Possible optimization of following not yet implemented:
  //  We could avoid calling "left" and "right" fewer times on the second visit
  //  by encoding whether the children exist in the "visit" variable on the work stack


  iterator end_it = end ();
  std::stack < std::pair<iterator, int> > work_stack;
  work_stack . push ( std::make_pair(begin (), 0 ) );
  while ( not work_stack . empty () ) {
    iterator it = work_stack . top () . first;
    int visit = work_stack . top () . second;
    work_stack . pop ();
    
    iterator left_it = left ( it );
    bool left_branch;
    if ( left_it == end_it ) left_branch = false;
    else left_branch = visited [ * left_it ];
    
    iterator right_it = right ( it );
    bool right_branch;
    if ( right_it == end_it ) right_branch = false;
    else right_branch = visited [ * right_it ];
    
    if ( visit == 0 ) {
      if ( left_branch || right_branch ) {
        new_leaf_sequence . push_back ( NOT_A_LEAF );
        work_stack . push ( std::make_pair ( it, 1 ) );
      } else {
        new_leaf_sequence . push_back ( LEAF );
        new_valid_sequence . push_back ( VALID );
      }
      if ( left_branch ) {
        work_stack . push ( std::make_pair ( left_it, 0 ) );
        continue;
      } else {
        if ( right_branch ) {
          new_leaf_sequence . push_back ( LEAF );
          new_valid_sequence . push_back ( NOT_VALID );
        }
      }
    }
    if ( visit == 1 ) {
      if ( right_branch ) {
          work_stack . push ( std::make_pair ( right_it, 0 ) );
      } else {
        if ( left_branch ) {
          new_leaf_sequence . push_back ( LEAF );
          new_valid_sequence . push_back ( NOT_VALID );
        }
      }
    }
  }
  return result;
}

template < class InputIterator >
CompressedTree * Tree::join ( InputIterator start, InputIterator stop ) {
  typedef Tree * TreePtr;
  typedef boost::shared_ptr<CompressedTree> CompressedTreePtr;

  CompressedTree * result = new CompressedTree;
  std::vector<bool> & leaf_sequence = result -> leaf_sequence;
  std::vector<bool> & valid_sequence = result -> valid_sequence;
  
  bool trivial_case = true; // All trees are empty.
  for ( InputIterator it = start; it != stop; ++ it ) {
    if ( it -> second -> leafCount () != 0 ) trivial_case = false;
  }
  if ( trivial_case ) {
    leaf_sequence . push_back ( false );
    valid_sequence . push_back ( false );
    return result;
  }
  //std::cout << "(1";

  leaf_sequence . push_back ( true );
  valid_sequence . push_back ( true );
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
  std::vector < TreePtr > trees;
  // As we traverse through the join of trees, we maintain a data structure
  // which tells us the maximum depth a tree has a node on the current path to root
  std::vector< boost::unordered_set < uint64_t > > trees_by_depth ( 1 );
  std::vector< CompressedTreePtr > compressed_trees;
  std::vector< int64_t > leaf_seq_positions;
  std::vector< int64_t > valid_seq_positions;
  std::stack<uint64_t> path_to_root;

  uint64_t current = 0;

  for ( InputIterator it = start; it != stop; ++ it ) {
    std::pair<TreePtr,CompressedTreePtr> tree_compressed_pair = *it;

    if ( tree_compressed_pair . second -> leafCount () == 0 ) continue;
    trees_by_depth [ 0 ] . insert ( trees . size () );
    trees . push_back ( tree_compressed_pair . first );
    iterators . push_back ( tree_compressed_pair . first -> begin () );
    compressed_trees . push_back ( tree_compressed_pair . second );
    leaf_seq_positions . push_back ( 0 );
    valid_seq_positions . push_back ( -1 );
  }

  const bool LEAF = false;
  const bool NOT_A_LEAF = true;
  const bool VALID = true;
  const bool NOT_VALID = false;
  int64_t depth = 0;
  int state = 0;  
  while ( 1 ) {
    if ( (depth == 0) && ( state == 2 ) ) break;
    //std::cout << "Position 0. depth = " << depth << " and state = " << state << "\n";
    bool success = false;
    boost::unordered_set < uint64_t > current_trees = trees_by_depth [ depth ];
    BOOST_FOREACH ( uint64_t i, current_trees ) {
      const Tree & tree = * trees [ i ];
      //std::cout << "Position 1. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
      iterator end = tree . end ();
      int64_t newdepth = depth;
      switch ( state ) {
        case 0: // Try to go left
        {
          iterator left = tree . left ( iterators[i] );
          if ( left == end ) break;
          ++ leaf_seq_positions [ i ];
          iterators[i] = left;
          newdepth = depth + 1;
          success = true;
          break;
        }
        case 1: // Try to go right
        {
          iterator right = tree . right ( iterators[i] );
          if ( right == end ) { 
            if ( compressed_trees [ i ] -> 
                leaf_sequence [ leaf_seq_positions [ i ] ] == LEAF ) {
              ++ valid_seq_positions [ i ];
            }
            break;
          }
          ++ leaf_seq_positions [ i ];
          iterators[i] = right;
          newdepth = depth + 1;
          success = true;
          break;
        }
        case 2: // Rise
        {
          if ( tree . isRight ( iterators[i] ) ) success = true;
          iterators[i] = tree . parent ( iterators[i] );
          newdepth = depth - 1;
          break;
        }
      }
      if ( newdepth != depth ) {
        trees_by_depth [ depth ] . erase ( i );
        if ( newdepth == (int64_t) trees_by_depth . size () ) { 
          trees_by_depth . push_back ( boost::unordered_set < uint64_t > () );
        }
        trees_by_depth [ newdepth ] . insert ( i );
      }
    }
    
    switch ( state ) {
      case 0: // Tried to go left
        if ( success ) {
          path_to_root . push ( current );
          current = leaf_sequence . size ();
          leaf_sequence . push_back ( NOT_A_LEAF ); // Start as not leaf (can change later)
          //valid_sequence . push_back ( true );
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
          path_to_root . push ( current );
          // Check if left branch exists
          if ( current == leaf_sequence . size () - 1 ) {
            // It doesn't. Make a fake leaf as placeholder.
            leaf_sequence . push_back ( LEAF );
            valid_sequence . push_back ( NOT_VALID );
          } 
          current = leaf_sequence . size ();
          leaf_sequence . push_back ( NOT_A_LEAF );
          //valid_sequence . push_back ( true );
          // Success. Try to go left now.
          state = 0;
          ++ depth;
        } else {
          // Check if left branch exists
          if ( current != leaf_sequence . size () - 1 ) {
            // It does. So make a fake leaf as placeholder for missing right.
            leaf_sequence . push_back ( LEAF );
            valid_sequence . push_back ( NOT_VALID );
          } else {
            // This is a leaf.
            // We must determine if it is valid.
            // Recharacterize it as a leaf
            leaf_sequence . back () = LEAF;
            // Advance one through valid_sequence of all current trees
            // and OR the results. (the leaf is valid if it is valid in
            // at least one of the joinands )
            bool is_valid = NOT_VALID;
            BOOST_FOREACH ( uint64_t i, current_trees ) {
              if ( compressed_trees [ i ] -> 
                   valid_sequence [ valid_seq_positions [ i ] ] ) {
                is_valid = VALID;
                break;
              }
            }
            valid_sequence . push_back ( is_valid );
          }
          // Failure. Rise.
          state = 2;
        }
        break;
      case 2: // Rose
        -- depth;
        current = path_to_root . top ();
        path_to_root . pop ();
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
  return result;
}

#endif
