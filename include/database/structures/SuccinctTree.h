#ifndef CMDB_SUCCINCTTREE_H
#define CMDB_SUCCINCTTREE_H

#include <vector>
#include <deque>

#include "database/structures/Tree.h"
#include "database/structures/SDSLFullBinaryTree.h"
#include "database/structures/RankSelect.h"


#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/export.hpp"

/**
 * @file
 * @author Arnaud Goullet, Shaun Harker
 * @version 1.0
 * 
 * @description
 * Implementation of a tree class using a succinct representation.
 * Any node in the tree may have at the most two children.
 * Standard operations and queries are provided. 
 */

class SuccinctTree : public Tree {
public:

  // Construction and Initialization
  SuccinctTree ( void );
  virtual ~SuccinctTree ( void ) {}
  
  void initialize ( std::vector < bool > bp , std::vector < bool > valid );
 
  // Iteration methods
  virtual iterator parent ( iterator it ) const;
  virtual iterator left ( iterator it ) const;
  virtual iterator right ( iterator it ) const;
  
  // Query methods
  virtual bool isleft ( iterator it ) const;
  virtual bool isright ( iterator it ) const;
  virtual bool isleaf ( iterator it ) const;
  size_type depth ( iterator it ) const;
  
  // Builder Methods
  virtual void subdivide ( void );
  //virtual void adjoin ( const Tree & other );
  virtual SuccinctTree * subtree ( const std::deque < Tree::iterator > & leaves ) const;
  virtual void assign ( const CompressedTree & compressed );

  virtual void debug ( void ) const;
  
  // Test and Debug
  void export_graphviz ( const char * filename ) const;
  void info ( void ) const;
  void memory_usage ( void ) const;
  uint64_t memory ( void ) const;
  
private:
  // Private methods
  SDSLFullBinaryTree::iterator ActualToFull ( iterator it ) const;
  iterator FullToActual ( SDSLFullBinaryTree::iterator it ) const;

  // Data
  SDSLFullBinaryTree fulltree_;
  RankSelect valid_rs_;
  
  // Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize ( Archive & ar , const unsigned int version ) {
    ar & boost::serialization::base_object<Tree>(*this);
    ar & fulltree_;
    ar & valid_rs_;   
  }
    
};

BOOST_CLASS_EXPORT_KEY(SuccinctTree);

inline SuccinctTree::SuccinctTree ( void ) {
  std::vector < bool > bp;
  std::vector < bool > valid;
  bp . push_back ( 1 );
  bp . push_back ( 0 );
  valid . push_back ( 1 );
  initialize ( bp, valid );
}

/**
 * Initialize the tree from a binary tree given by its balanced parentheses
 * representation bp and the corresponding bit sequence of which nodes
 * are valid ('1') or not ('0') (ghost node).
 * @param bp
 * @param valid
 */
inline void SuccinctTree::initialize ( std::vector < bool > bp , std::vector < bool > valid ) {
  SDSLFullBinaryTree fbt ( bp );
  fulltree_ = fbt;
  RankSelect rs ( valid );
  valid_rs_ = rs;
  size_ = valid_rs_ . rank ( fulltree_ . size ( ) );
}

inline SuccinctTree::iterator SuccinctTree::parent ( iterator it ) const {
  SDSLFullBinaryTree::iterator full_it = ActualToFull ( it );
  full_it = fulltree_ . parent ( full_it );
  return FullToActual ( full_it );
}

inline SuccinctTree::iterator SuccinctTree::left ( iterator it ) const {
  SDSLFullBinaryTree::iterator full_it = ActualToFull ( it );
  full_it = fulltree_ . left ( full_it );
  return FullToActual ( full_it );
}

inline SuccinctTree::iterator SuccinctTree::right ( iterator it ) const {
  SDSLFullBinaryTree::iterator full_it = ActualToFull ( it );
  full_it = fulltree_ . right ( full_it );
  return FullToActual ( full_it );
}

inline bool SuccinctTree::isleft ( iterator it ) const {
  return fulltree_ . isleft ( ActualToFull ( it ) );
}

inline bool SuccinctTree::isright ( iterator it ) const {
  return fulltree_ . isright ( ActualToFull ( it ) );
}

inline bool SuccinctTree::isleaf ( iterator it ) const {
  //std::cout << "isleaf(" << *it << ") = fulltree.isleaf(" << * ActualToFull ( it ) << ") = ... \n";
  return fulltree_ . isleaf ( ActualToFull ( it ) );
}

inline void SuccinctTree::subdivide ( void ) {

  //std::cout << "Presubdivision:\n";
  //debug ();
  
  // Method 2 : we construct the new balance parentheses sequence
  //            and rebuild the rank/select structures once at the end
  // Issue : need a copy of the balance parentheses and valid sequences
  //         since currently, we don't have access to their element
  // 200 times faster as method above.

  std::vector < bool > oldbp , newbp;
  std::vector < bool > oldvalid , newvalid;

  oldbp = fulltree_ . bp_sequence ();
  oldvalid = valid_rs_ . bits_sequence ();

  size_type oldsize;
  oldsize = fulltree_ . size ();

  size_type indexbp , indexvalid;

  indexbp = 0;
  indexvalid = 0;
  while ( indexbp < 2 * oldsize - 1 ) {
    // if we encounter a leaf
    if ( oldbp [ indexbp ] == 1 && oldbp [ indexbp + 1 ] == 0 ) {
      // and the leaf is valid then subdivide it
      if ( oldvalid [ indexvalid ] ) {
        // subdivide a leaf, add the sequence 1 10 10 0
        newbp . push_back ( 1 );
        newbp . push_back ( 1 );
        newbp . push_back ( 0 );
        newbp . push_back ( 1 );
        newbp . push_back ( 0 );
        newbp . push_back ( 0 );
        newvalid . push_back ( 1 );
        newvalid . push_back ( 1 );
        newvalid . push_back ( 1 );
        indexbp += 2;
        ++ indexvalid;
      } else { // if the leaf is not valid, keep it non valid and don't subdivide
        newbp . push_back ( 1 );
        newbp . push_back ( 0 );
        newvalid . push_back ( oldvalid [ indexvalid ] );
        indexbp += 2;
        ++ indexvalid;
      }
    } else { // we do not encounter a leaf
      newbp . push_back ( oldbp [ indexbp ] );
      if ( oldbp [ indexbp ] ) {
        newvalid . push_back ( oldvalid [ indexvalid ] );
        ++ indexvalid;
      }
      ++ indexbp;
    }
  }
  newbp . push_back ( 0 ); // To close the root node

  fulltree_ = SDSLFullBinaryTree ( newbp );
  valid_rs_ = RankSelect ( newvalid );
  size_ = valid_rs_ . rank ( newvalid . size () - 1 );
  //std::cout << "Postsubdivision:\n";
  //debug ();
}
/*
inline void SuccinctTree::adjoin ( const Tree & other ) {
  std::vector < bool > bp;
  std::vector < bool > valid;
  
  // Brief description of following algorithm: (Very similar to clutching algorithm)
  //
  // We want to advance through both trees simultaneously
  // If we explore a subtree in one tree that does not exist in the other, we remain halted on the one
  // until the other finishes.
  
  // State machine:
  // State 0: Try to go left. If can't, set success to false and try to go right. Otherwise success is true and try to go left on next iteration.
  // State 1: Try to go right. If can't, set success to false and rise. Otherwise success is true and try to go left on next iteration.
  // State 2: Rise. If rising from the right, rise again on next iteration. Otherwise try to go right on the next iteration.
  
  
  iterator iter1 = begin ();
  iterator iter2 = other . begin ();
  iterator end1 = end ();
  iterator end2 = other . end ();

  size_t depth1 = 0;
  size_t depth2 = 0;
  size_t depth = 0;
  size_t newsize = 0;
  int state = 0;
  
  bp . push_back ( 1 );
  valid . push_back ( 1 );
  ++ newsize;
  int direction = 0;
  while ( 1 ) {
    if ( (depth == 0) && ( state == 2 ) ) break;
    //std::cout << "Position 0. depth = " << depth << " and state = " << state << "\n";
    bool success = false;
    
    //std::cout << "Position 1. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
    // If node is halted, continue
    if ( depth1 == depth ) {
      switch ( state ) {
        case 0:
        {
          iterator left_it = left ( iter1 );
          if ( left_it == end1 ) break;
          iter1 = left_it;
          ++ depth1;
          success = true;
          break;
        }
        case 1:
        {
          iterator right_it = right ( iter1 );
          if ( right_it == end1 ) break;
          iter1 = right_it;
          ++ depth1;
          success = true;
          break;
        }
        case 2:
        {
          if ( isright ( iter1 ) ) success = true;
          iter1 = parent ( iter1 );
          -- depth1;
          break;
        }
      }
    }
    
    //std::cout << "Position 2. i = " << i << ", depth = " << depth << " and state = " << state << "\n";
    // If node is halted, continue
    if ( depth2 == depth ) {
      switch ( state ) {
        case 0:
        {
          iterator left_it = other . left ( iter2 );
          if ( left_it == end2 ) break;
          iter2 = left_it;
          ++ depth2;
          success = true;
          break;
        }
        case 1:
        {
          iterator right_it = other . right ( iter2 );
          if ( right_it == end2 ) break;
          iter2 = right_it;
          ++ depth2;
          success = true;
          break;
        }
        case 2:
        {
          if ( other . isright ( iter2 ) ) success = true;
          iter2 = other . parent ( iter2 );
          -- depth2;
          break;
        }
      }
    }
  
    //std::cout << "Position 3. depth = " << depth << " and state = " << state << "\n";
    //std::cout << ( success ? "success" : "failure" );
    switch ( state ) {
      case 0: // Tried to go left
        if ( success ) {
          // Success. Try to go left again.
          // push: (valid
          bp . push_back ( 1 );
          valid . push_back ( 1 );
          ++ newsize;
          direction = 0; // went left
          state = 0;
          ++ depth;
        } else {
          // Failure. Try to go right instead.
          state = 1;
        }
        break;
      case 1: // Tried to go right
        if ( success ) {
          if ( direction != 2 ) { // there is no left branch, but there is a right branch
            // push: (invalid)(valid
            bp . push_back ( 1 );
            valid . push_back ( 0 );
            bp . push_back ( 0 );
            bp . push_back ( 1 );
            valid . push_back ( 1 );
            ++ newsize;
          } else { // there is a left branch and a right branch
            // push: (valid
            bp . push_back ( 1 );
            valid . push_back ( 1 );
            ++ newsize;
          }
          // Sucess. Try to go left now.
          direction = 1; // went right
          state = 0;
          ++ depth;
        } else {
          if ( direction == 2 ) { // there is a left branch but no right branch
            // push: (invalid)
            bp . push_back ( 1 );
            valid . push_back ( 0 );
            bp . push_back ( 0 );
          }
          // Failure. Rise.
          state = 2;
        }
        break;
      case 2: // Rose
        // push: )
        bp . push_back ( 0 );
        direction = 2; // rose
        -- depth;
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
  bp . push_back ( 0 );

  fulltree_ = SDSLFullBinaryTree ( bp );
  valid_rs_ = RankSelect ( valid );
  size_ = newsize;

}
*/
inline SuccinctTree * SuccinctTree::subtree ( const std::deque < iterator > & leaves ) const {

  std::vector < bool > bp;
  std::vector < bool > valid;

  if ( leaves . empty () ) {
    SuccinctTree * result = new SuccinctTree;
    bp . push_back ( 1 );
    valid . push_back ( 0 );
    bp . push_back ( 0 );
    result -> initialize ( bp , valid );
    return result;
  }

  
  std::vector < bool > visited ( size (), false );
  visited [ 0 ] = true;
  BOOST_FOREACH ( iterator leaf, leaves ) {
    iterator it = leaf;
    while ( visited [ * it ] == false ) {
      visited [ * it ] = true;
      it = parent ( it );
    }
  }
 
  // Now walk through visited nodes and write bp and valid

  
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
      bp . push_back ( 1 );
      valid . push_back ( 1 );
      work_stack . push ( std::make_pair ( it, 1 ) );

      if ( left_branch ) {
        work_stack . push ( std::make_pair ( left_it, 0 ) );
        continue;
      } else {
        if ( right_branch ) {
          bp . push_back ( 1 );
          valid . push_back ( 0 );
          bp . push_back ( 0 );
        }
      }
    }

    if ( visit == 1 ) {
      work_stack . push ( std::make_pair ( it, 2 ) );
      if ( right_branch ) {
          work_stack . push ( std::make_pair ( right_it, 0 ) );
      } else {
        if ( left_branch ) {
          bp . push_back ( 1 );
          valid . push_back ( 0 );
          bp . push_back ( 0 );
        }
      }
    }
    
    if ( visit == 2 ) {
      bp . push_back ( 0 );
    }
  }
  
  SuccinctTree * result = new SuccinctTree;
  result -> initialize ( bp , valid );
  return result;
  
  // The following is far too inefficient
#if 0
  std::vector < iterator > visited;
  
  BOOST_FOREACH ( iterator it_node , leaves ) {
    visited . push_back ( *it_node );
    while ( it_node != begin ( ) ) {
      it_node = parent ( it_node );
      // no need to check for ghost, parent has to be valid
      visited . push_back ( *it_node );
    }
  }
  
  // find the closing and opening parentheses position for the visited nodes from
  // the fulltree_ representation
  std::vector < std::pair < size_type , bool > > oc;
  
  BOOST_FOREACH ( iterator it_node , visited ) {
    std::pair < size_type , bool > pair;
    //iterator it(node);
    SDSLFullBinaryTree::iterator it_full_o , it_full_c;
    it_full_o = ActualToFull ( it_node );
    pair . first = fulltree_ . opening_position ( it_full_o );
    pair . second = 1;
    oc . push_back ( pair );
    pair . first = fulltree_ . closing_position ( it_full_o );
    pair . second = 0;
    oc . push_back ( pair );
  }
  
  // we sort oc with respect to the first argument of the pair and remove any repetition
  std::sort ( oc.begin ( ) , oc.end ( ) ,
             boost::bind ( &std::pair<size_type , bool>::first , _1 ) <
             boost::bind ( &std::pair<size_type , bool>::first , _2 ) );
  boost::erase ( oc , boost::unique<boost::return_found_end > ( oc ) );
  
  // construction of the binary tree
  std::vector < bool > newbp;
  std::vector < bool > newvalid;
  // root always there and active
  newbp . push_back ( 1 );
  newvalid . push_back ( 1 );
  //
  for ( unsigned int i = 1; i < oc.size ( ); ++ i ) {
    if ( oc[i].first - oc[i - 1].first > 1 ) {
      newbp . push_back ( 1 );
      newbp . push_back ( 0 );
      newvalid . push_back ( 0 );
    }
    newbp . push_back ( oc[i].second );
    if ( oc[i].second == 1 ) newvalid . push_back ( 1 );
  }
  
  SuccinctTree * result = new SuccinctTree;
  result -> initialize ( newbp , newvalid );
  return result;
#endif
}

inline void SuccinctTree::assign ( const CompressedTree & compressed ) {
  initialize ( compressed . balanced_parentheses, compressed . valid_tree_nodes );
}

inline void SuccinctTree::debug ( void ) const {  
  std::vector < bool > bp = fulltree_ . bp_sequence ( );
  std::vector < bool > valid = valid_rs_ . bits_sequence ( );
  
  std::cout << "Balanced Parantheses Sequence:\n";
  for ( size_t i = 0; i < bp . size (); ++ i ) std::cout << (bp[i]? '(' : ')');
  std::cout << ".\n";
  
  std::cout << "Validity Sequence:\n";
  size_t count = 0;
  for ( size_t i = 0; i < valid . size (); ++ i ) {
    std::cout << (valid[i]? 1 : 0);
    if ( valid [ i ] ) ++ count;
  }
  std::cout << ".\n";
  
  std::cout << "Size of BP sequence = " << bp . size () << "\n";
  std::cout << "Size of validity sequence = " << valid . size () << "\n";
  std::cout << "Number of valid nodes = " << count << "\n";
}


// Private methods
inline SDSLFullBinaryTree::iterator SuccinctTree::ActualToFull ( iterator it ) const {
  return valid_rs_ . select ( *it + 1 );
}

inline Tree::iterator SuccinctTree::FullToActual ( SDSLFullBinaryTree::iterator it ) const {
  if ( valid_rs_ . bits ( *it ) ) return valid_rs_ . rank ( * it - 1 );
  else return end ( );
}

// Features

/**
 * Export the Full Binary Tree to a file "filename" in the GraphViz format .
 * @param filename
 */
inline void SuccinctTree::export_graphviz ( const char *filename ) const {
  std::ofstream myfile;
  myfile.open ( filename );
  myfile << "digraph binarytree { " << std::endl;
  if ( size_ == 1 ) {
    myfile << "\"" << 0 << "\"->\""
    << 0 << "\";" << "\n";
  } else {
    iterator it;
    for ( it = begin ( ); it != end ( ); ++ it ) {
      if ( *it != 0 ) {
        if ( valid_rs_ . bits(*ActualToFull(it)) ) {
          iterator it2;
          it2 = parent ( it );
          myfile << "\"" << * it2
          << "\"->\""
          << * it << "\";" << std::endl;
        }
      }
    }
  }
  myfile << "}" << std::endl;
  myfile.close ( );
}

/**
 * Display information and statistic regarding the balanced parentheses representation.
 */
inline void SuccinctTree::info ( void ) const {
  std::cout << "\n----------------------\n";
  std::cout << "Tree size : " << size_ << "\n";
  fulltree_ . display_bp ( );
  std::cout << "Valid ";
  valid_rs_ . info ( );
}

inline void SuccinctTree::memory_usage ( void ) const {
  //
  fulltree_ . memory_usage ( );
  valid_rs_ . memory_usage ( );
}

inline uint64_t SuccinctTree::memory ( void ) const {
  return fulltree_ . memory () + valid_rs_ . memory ();
}

#if 0


inline std::vector < SuccinctTree::iterator > SuccinctTree::subtree_list ( iterator it ) const {
  iterator it_begin ( it );
  iterator it_end ( next ( it ) );
  
  std::vector < iterator > list;
  for ( iterator itt = it_begin; itt != it_end; ++ itt ) list . push_back ( itt );
  return list;
}

inline SuccinctTree::iterator SuccinctTree::next ( iterator it ) const {
  return it + subtree_size ( it );
}

// Features

inline std::vector < unsigned char > SuccinctTree::prefix ( iterator it ) const {
  return fulltree_ . prefix ( ActualToFull ( it ) );
}

inline SuccinctTree::size_type SuccinctTree::depth ( iterator it ) const {
  return fulltree_ . depth ( ActualToFull ( it ) );
  
}

inline SuccinctTree::size_type SuccinctTree::subtree_size ( iterator it ) const {
  SDSLFullBinaryTree::iterator full_it = ActualToFull ( it );
  uint64_t full_subtree_size = fulltree_ . subtree_size ( full_it );
  return valid_rs_ . rank ( * full_it + full_subtree_size - 1 ) - * it;
  // note: *it == rank ( * full_it )
}





/**
 * Subdivide all the leaves in the tree.
 */
void subdivide ( void );

// Queries

/**
 * @param it
 * @return the iterator pointing to the parent node.
 */
iterator parent ( iterator it ) const;

/**
 * @param it
 * @return the iterator pointing to the left child.
 */
iterator left ( iterator it ) const;
/**
 * @param it
 * @return the iterator pointing to the right child.
 */
iterator right ( iterator it ) const;
/**
 * @param it
 * @return true if the iterator 'it' is a left child.
 */
bool isleft ( iterator it ) const;
/**
 * @param it
 * @return true if the iterator 'it' is a right child.
 */
bool isright ( iterator it ) const;
/**
 * @param it
 * @return true if the iterator 'it' is a leaf.
 */
bool isleaf ( iterator it ) const;
/**
 * @param it
 * @return the path from the root node to the iterator.
 * With convention '0' left branch and '1' right branch.
 */
std::vector < unsigned char > prefix ( iterator it ) const;
/**
 * @param it
 * @return the depth of a given node.
 */
size_type depth ( iterator it ) const;

/**
 * @param it
 * @return the size of the subtree of a given node (included in the subtree).
 */
size_type subtree_size ( iterator it ) const;
/**
 * @param it
 * @return a list of iterators covering the subtree of a given node.
 * The first element of the list is it.
 */
std::vector < iterator > subtree_list ( iterator it ) const;
/**
 *
 * @param leaves
 * @return the minimal Binary Tree constructed from a set of leaves.
 */
SuccinctTree subtree ( std::vector < iterator > & leaves );

/**
 *
 * @return the corresponding Full Binary Tree, where all nodes are valid.
 */
const SDSLFullBinaryTree & fulltree ( void ) const {
  return fulltree_;
}

/**
 *
 * @return the bits sequence of the nodes that are valid or not.
 */
const std::vector < bool > valid ( void ) const {
  std::vector<bool> result;
  result = valid_rs_ . bits_sequence ( );
  return result;
}

//  const bool valid(iterator it) const {
//    return valid_rs_ . bits(*ActualToFull(it));
//  }

/**
 *
 * @return the bits sequence of the nodes that are leaves ('1') or not ('0').
 */
const std::vector < bool > leaves_sequence ( void ) const {
  std::vector < bool > result;
  iterator it;
  for ( it = begin ( ); it != end ( ); ++ it ) {
    if ( isleaf ( it ) ) {
      result . push_back ( 1 );
    } else {
      result . push_back ( 0 );
    }
  }
  return result;
}

/**
 * Erase the subtree under a particular node in the tree.
 * The node pointed by the iterator is also removed
 * @param it
 */
void erase ( iterator it );
/**
 * Given a list of iterators, erase the subtrees under them.
 * The iterators are sorted and filtered out if they belong to the same subtree.
 * @param it_list
 */
void erase ( std::vector < iterator > it_list );


/**
 * @param it
 * @return the iterator pointing to the next node, that does not belong to the subtree of 'it'.
 */
iterator next ( iterator it ) const;


/**
 * Display the memory usage of the Binary Tree.
 */
void memory_usage ( void ) const;


inline void SuccinctTree::erase ( iterator it ) {
  SDSLFullBinaryTree::iterator it_full ( ActualToFull ( it ) );
  size_type subtreesize ( fulltree_ . subtree_size ( it_full ) );
  size_type oldfullsize ( fulltree_.size ( ) );
  
  fulltree_ . erase ( it_full );
  
  std::vector < bool > newvalid;
  for ( size_type i = 0; i <= * it_full; ++ i ) {
    newvalid . push_back ( valid_rs_ . bits ( i ) );
  }
  for ( size_type i = * it_full + subtreesize; i < oldfullsize; ++ i ) {
    newvalid . push_back ( valid_rs_ . bits ( i ) );
  }
  valid_rs_ = RankSelect ( newvalid );
  size_ = valid_rs_ . rank ( fulltree_ . size ( ) - 1 );
}


// THIS NOT THE BEST WAY TO DO IT, BECAUSE WE ARE DOING TWICE THE SAME THING

inline void SuccinctTree::erase ( std::vector < iterator > it_list ) {
  
  // we start by updating valid_rs_
  // What follow is the same piece of code found in SDSLFullBinaryTree
  // sort the list of iterator in case they are not ordered
  
  
  // First we sort the list of iterators, just in case they are not ordered
  boost::sort ( it_list );
  
  // Second : in case several iterators are part of the same subtree, we should not count them
  // To know that, we check against the size of each subtree starting from the iterators in the list
  std::vector < size_type > subtree_size_list;
  // list the size of the subtree for each node we want to delete
  BOOST_FOREACH ( iterator it , it_list ) subtree_size_list . push_back ( subtree_size ( it ) );
  
  
  std::vector < iterator > it_list2;
  size_type pos;
  
  /*
   * given a sequence of iterators :
   * it0, it1, it2, ...
   * and the size of their respective substree
   * s0, s1, s2, ...
   * if si = 1 then it is a leaf, erase does not do anything, just skip
   * if si != 1 then we check if some iterators are part of the same subtree
   */
  
  
  int flag = 0;
  pos = 0;
  // add the first non-leaf to the new list
  while ( subtree_size_list[pos] == 1 ) {
    ++ pos;
  }
  it_list2 . push_back ( it_list[pos] );
  
  while ( flag == 0 ) {
    // if it is not a leaf
    if ( subtree_size_list[pos] > 1 ) {
      // loop over the consecutive iterator (after the reference iterator)
      for ( size_type i = pos + 1; i < it_list.size ( ); ++ i ) {
        // if the ith iterator does not belong to the subtree of the reference iterator
        if ( *it_list[i] >= * it_list[pos] + subtree_size_list[pos] ) {
          it_list2 . push_back ( it_list[i] );
          pos = i;
          break;
          // if the ith iterator belong to the subtree of the reference iterator
          // do nothing except if we reach the end of it_list, then exit
        } else {
          if ( i == it_list.size ( ) - 1 ) {
            flag = 1;
            break;
          }
        }
      }
      // if we have a leaf, skip and keep going
    } else {
      if ( pos == it_list.size ( ) - 1 ) {
        flag = 1;
        break;
      }
      ++ pos;
    }
    
    if ( pos >= it_list.size ( ) - 1 ) {
      flag = 1;
    }
  }
  
  std::vector < bool > newvalid;
  
  pos = 0;
  
  BOOST_FOREACH ( iterator it , it_list2 ) {
    // position of node to erase in the valid sequence
    size_type position ( *ActualToFull ( it ) );
    // position of the last node in the subtree starting from it
    size_type position_end ( position + fulltree_.subtree_size ( ActualToFull ( it ) ) );
    //
    for ( size_type j = pos; j <= position; ++ j ) {
      newvalid . push_back ( valid_rs_ .bits ( j ) );
    }
    pos = position_end;
  }
  // copy the last piece
  for ( size_type j = pos; j < size_; ++ j ) {
    newvalid . push_back ( valid_rs_ . bits ( j ) );
  }
  
  
  std::vector < SDSLFullBinaryTree::iterator > list;
  
  BOOST_FOREACH ( iterator it , it_list ) {
    list . push_back ( ActualToFull ( it ) );
  }
  
  fulltree_ . erase ( list );
  
  valid_rs_ = RankSelect ( newvalid );
  
  size_ = valid_rs_ . rank ( newvalid.size ( ) - 1 );
  
}

inline void SuccinctTree::subdivide ( iterator it ) {
  
  size_type oldsize ( fulltree_ . size ( ) );
  fulltree_ . subdivide ( ActualToFull ( it ) );
  
  // update the valid sequence
  std::vector < bool > newvalid;
  size_type position ( *ActualToFull ( it ) );
  for ( size_type i = 0; i <= position; ++ i ) newvalid . push_back ( valid_rs_ . bits ( i ) );
  newvalid . push_back ( 1 );
  newvalid . push_back ( 1 );
  for ( size_type i = position + 1; i < oldsize; ++ i ) newvalid . push_back ( valid_rs_. bits ( i ) );
  
  valid_rs_ = RankSelect ( newvalid );
  size_ += 2;
  
}

#endif
#endif
