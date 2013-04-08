/// ToplexDetail.h
/// Shaun Harker
/// 9/21/11

#ifndef CMDB_POINTERTREE_H
#define CMDB_POINTERTREE_H

#include <stdint.h>
#include <memory>
#include <vector>
#include <stack>

#include <boost/foreach.hpp>
#include "database/structures/Tree.h"

#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/export.hpp"

/**************************
 * struct PointerTreeNode *
 **************************/
struct PointerTreeNode {
  PointerTreeNode * left_;
  PointerTreeNode * right_;
  PointerTreeNode * parent_;
  uint64_t contents_;
  PointerTreeNode ( void );
  ~PointerTreeNode ( void );
  friend class boost::serialization::access;
  
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & left_;
    ar & right_;
    ar & parent_;
    ar & contents_;
  }
};

inline PointerTreeNode::PointerTreeNode ( void ) : left_ ( NULL ), right_ ( NULL ), parent_ ( NULL ), contents_ ( 0 ) {
} /* PointerTreeNode::PointerTreeNode */

inline PointerTreeNode::~PointerTreeNode ( void ) {
} /* PointerTreeNode::~PointerTreeNode */

/*********************
 * class PointerTree *
 *********************/

class PointerTree : public Tree {
public:
  // Constructor/Deconstructor Methods
  PointerTree ( void );
  virtual ~PointerTree ( void );
  
  // Iteration methods
  virtual iterator parent ( iterator it ) const;
  virtual iterator left ( iterator it ) const;
  virtual iterator right ( iterator it ) const;
  
  // Query methods
  virtual bool isleft ( iterator it ) const;
  virtual bool isright ( iterator it ) const;
  virtual bool isleaf ( iterator it ) const;
  
  // Mutation Methods
  virtual void subdivide ( void );
  virtual void adjoin ( const Tree & other );
  virtual PointerTree * subtree ( const std::vector < Tree::iterator > & leaves ) const;
private:

  std::vector < PointerTreeNode * > nodes_;

  // Serialization Methods
  friend class boost::serialization::access;
  template<typename Archive>
  void serialize(Archive & ar, const unsigned int file_version) {
    ar & boost::serialization::base_object<Tree>(*this);
    ar & nodes_;
  }
  
  void debug ( void ) const;
};

BOOST_CLASS_EXPORT_KEY(PointerTree);

inline PointerTree::PointerTree ( void ) {
  PointerTreeNode * node = new PointerTreeNode;
  node -> contents_ = 0;
  nodes_ . push_back ( node );
}
inline PointerTree::~PointerTree ( void ) {
  //std::cout << "Deconstructing pointer tree " << this << ", which has " << nodes_.size() << " nodes.\n";
  for ( size_t i = 0; i < nodes_ . size (); ++ i ) {
    delete nodes_[i];
  }
}

inline Tree::iterator PointerTree::parent ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> parent_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
}

inline Tree::iterator PointerTree::left ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> left_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
  
}

inline Tree::iterator PointerTree::right ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> right_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
  
}

// Query methods
inline bool PointerTree::isleft ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> left_ == node );
}

inline bool PointerTree::isright ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> right_ == node );
}

inline bool PointerTree::isleaf ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  if ( ( node -> left_ == NULL ) && ( node -> right_ == NULL ) ) return true;
  return false;
}


inline void PointerTree::subdivide ( void ) {
  //std::cout << "PointerTree::subdivide\n";
  uint64_t InitialNumberOfPointerTreeNodes = nodes_ . size ();
  uint64_t label = InitialNumberOfPointerTreeNodes;
  for ( uint64_t i = 0; i < InitialNumberOfPointerTreeNodes; ++ i ) {
    PointerTreeNode * node = nodes_ [ i ];
    if ( ( node -> left_ == NULL ) && ( node -> right_ == NULL ) ) {
      //std::cout << "Giving node " << node -> contents_ << " children " << label << " and " << label + 1  << "\n";
      node -> left_ = new PointerTreeNode;
      node -> right_ = new PointerTreeNode;
      node -> left_ -> parent_ = node;
      node -> right_ -> parent_ = node;
      node -> left_ -> contents_ = label ++;
      node -> right_ -> contents_ = label ++;
      nodes_ . push_back ( node -> left_ );
      nodes_ . push_back ( node -> right_ );
    }
  }
  size_ = label;
  //debug ();
}

inline void PointerTree::adjoin ( const Tree & other ) {
  // Do a tour of "other", forcing the creation of nodes in "this" to follow along
  
  // Strategy: stack containing pair of "other tree iterator, this tree iterator"
  //           for corresponding points in tree
  std::stack < std::pair<iterator, iterator> > work_stack;
  work_stack . push ( std::make_pair ( begin (), other . begin () ) );
  
  // Store an iterator and its parent.
  while ( not work_stack . empty () ) {
    Tree::iterator other_it = work_stack . top () . first;
    Tree::iterator this_it = work_stack . top () . second;
    work_stack . pop ();
    // Proceed down right branch.
    Tree::iterator other_right_it = other . right ( other_it );
    if ( other_right_it != other . end () ) {
      if ( nodes_ [ *this_it ] -> right_ == NULL ) {
        // Adjoint new node if needed
        PointerTreeNode * node = new PointerTreeNode;
        nodes_ [ *this_it ] -> right_ = node;
        node -> parent_ = nodes_ [ *this_it ];
        node -> contents_ = size_ ++;
        nodes_ . push_back ( node );
      }
      work_stack . push ( std::make_pair ( other_right_it, right ( this_it ) ) );
    }
    // Proceed down left branch.
    Tree::iterator other_left_it = other . left ( other_it );
    if ( other_left_it != other . end () ) {
      if ( nodes_ [ *this_it ] -> left_ == NULL ) {
        // Adjoint new node if needed
        PointerTreeNode * node = new PointerTreeNode;
        nodes_ [ *this_it ] -> left_ = node;
        node -> parent_ = nodes_ [ *this_it ];
        node -> contents_ = size_ ++;
        nodes_ . push_back ( node );
      }
      work_stack . push ( std::make_pair ( other_left_it, left ( this_it ) ) );
    }
  }
  
}

inline PointerTree * PointerTree::subtree
( const std::vector < Tree::iterator > & leaves ) const {
  //std::cout << "PointerTree::raw_subtree with " << leaves.size () << " leaves.\n";
  if ( leaves . empty () ) {
    std::cout << "Error in subtree: expect nonempty set of leaves\n";
    abort ();
  }
  PointerTree * result = new PointerTree ();
  // Mark part of tree to be copied.
  std::vector < bool > part_of_tree ( size_, false );
  Tree::iterator root = begin ();
  BOOST_FOREACH ( Tree::iterator leaf, leaves ) {
    Tree::iterator it = leaf;
    while ( it != root ) {
      if ( part_of_tree [ * it ] == true ) break; // efficiency
      part_of_tree [ * it ] = true;
      it = parent ( it );
    }
  }
  part_of_tree [ * root ] = true;
  // Construct pointer tree for marked part of tree
  // Strategy: store pairs of "old iterator, new parent node"
  // store a stack of bools to represent left or right child. omit the entry for root.
  std::stack < std::pair<Tree::iterator, PointerTreeNode *> > dfs_stack;
  std::stack < bool > parity;
  dfs_stack . push ( std::make_pair ( root, (PointerTreeNode *)NULL ) );
  while ( not dfs_stack . empty () ) {
    std::pair < Tree::iterator, PointerTreeNode * > item = dfs_stack . top ();
    dfs_stack . pop ();
    Tree::iterator old_it = item . first;
    PointerTreeNode * parent = item . second;
    if ( not part_of_tree [ * old_it ] )  {
      parity . pop ();
      continue;
    }
    PointerTreeNode * node;
    // If not root, create new node and assign members of it and parent
    if ( parent != NULL ) {
      node = new PointerTreeNode;
      node -> parent_ = parent;
      node -> contents_ = result -> nodes_ . size ();
      result -> nodes_ . push_back ( node );
      ++ ( result -> size_ );
      bool side = parity . top ();
      parity . pop ();
      if ( side == false ) {
        // left side
        node -> parent_ -> left_ = node;
      } else {
        // right side
        node -> parent_ -> right_ = node;
      }
    } else {
      node = result -> nodes_ [ 0 ];
    }
    // Enqueue left and right children of "node" for insertion if they exist
    Tree::iterator left_it = left ( old_it );
    if ( left_it != end () ) {
      dfs_stack . push ( std::make_pair (left_it, node) );
      parity . push ( false );
    }
    Tree::iterator right_it = right ( old_it );
    if ( right_it != end () ) {
      dfs_stack . push ( std::make_pair (right_it, node) );
      parity . push ( true );
    }
  }
  //result -> debug ();
  return result;
}

inline void PointerTree::debug ( void ) const {
  // DEBUG:
  std::cout << "Pointertree debug output:\n";
  std::cout << "address = " << this << "\n";
  for ( uint64_t i = 0; i < nodes_.size(); ++ i ) {
    std::cout << i << " == " << nodes_[i] -> contents_ << "\n";
    if ( nodes_[i] -> parent_ != NULL ) {
      std::cout << i << ":" << nodes_[i]->parent_->contents_ << " -> " << nodes_[i]->contents_ << "\n";
    }
  }
  std::cout << "finish pointertree debug output\n";
}

#endif
