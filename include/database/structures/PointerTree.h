/// ToplexDetail.h
/// Shaun Harker
/// 9/21/11

#ifndef CMDB_POINTERTREE_H
#define CMDB_POINTERTREE_H

#include <stdint.h>
#include <memory>
#include <vector>
#include <deque>
#include <stack>
#include <utility>

#include <boost/foreach.hpp>
#include "boost/shared_ptr.hpp"
#include "database/structures/Tree.h"
#include "database/structures/CompressedTree.h"

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
  virtual bool isLeft ( iterator it ) const;
  virtual bool isRight ( iterator it ) const;
  virtual bool isLeaf ( iterator it ) const;
  
  // Mutation Methods
  //virtual void subdivide ( void );
  //virtual PointerTree * subtree ( const std::deque < Tree::iterator > & leaves ) const;
  virtual void assign ( boost::shared_ptr<const CompressedTree> compressed );

private:

  std::vector < PointerTreeNode * > nodes_;


  // Serialization Methods
  friend class boost::serialization::access;
  template<typename Archive>
  void serialize(Archive & ar, const unsigned int file_version) {
    ar & boost::serialization::base_object<Tree>(*this);
    ar & nodes_;
  }
public:
  // Test and Debug
  virtual uint64_t memory ( void ) const {
    return sizeof ( PointerTree ) +
           sizeof ( PointerTreeNode * ) * nodes_ . size () +
           sizeof ( PointerTreeNode ) * nodes_ . size ();
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
inline bool PointerTree::isLeft ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> left_ == node );
}

inline bool PointerTree::isRight ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> right_ == node );
}

inline bool PointerTree::isLeaf ( iterator it ) const {
  PointerTreeNode * node = nodes_ [ * it ];
  if ( ( node -> left_ == NULL ) && ( node -> right_ == NULL ) ) return true;
  return false;
}

#if 0

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

inline PointerTree * PointerTree::subtree
( const std::deque < Tree::iterator > & leaves ) const {
  //std::cout << "PointerTree::subtree with " << leaves.size () << " leaves.\n";
  //std::cout << "   size of tree = " << size () << "\n";
  if ( leaves . empty () ) {
    PointerTree * result = new PointerTree;
    // new PointerTree is initialized with a root; we must erase it
    delete result -> nodes_ [ 0 ];
    result -> nodes_ . clear ();
    result -> size_ = 0;
    return result;
  }
  PointerTree * result = new PointerTree ();
  // Mark part of tree to be copied.
  std::vector < bool > part_of_tree ( size_, false );
  Tree::iterator root = begin ();
  BOOST_FOREACH ( Tree::iterator leaf, leaves ) {
    Tree::iterator it = leaf;
    //std::cout << "Inspecting leaf " << *leaf << "\n";
    while ( it != root ) {
      //std::cout << "  ->Inspecting leaf " << *it << "\n";
      if ( part_of_tree [ * it ] == true ) break; // efficiency
      part_of_tree [ * it ] = true;
      it = parent ( it );
    }
  }
  part_of_tree [ * root ] = true;
  // Construct pointer tree for marked part of tree
  // Strategy: store pairs of "old iterator, new parent node"
  // store a stack of bools "parity" to represent left or right child. 
  // omit the entry for root in "parity".
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
    //std::cout << "Enqueue (left) and right\n";
    Tree::iterator left_it = left ( old_it );
    if ( left_it != end () ) {
      //std::cout << "Found a left child.\n";
      dfs_stack . push ( std::make_pair (left_it, node) );
      parity . push ( false );
    }
    //std::cout << "Enqueue left and (right)\n";
    Tree::iterator right_it = right ( old_it );
    if ( right_it != end () ) {
      //std::cout << "Found a right child.\n";
      dfs_stack . push ( std::make_pair (right_it, node) );
      parity . push ( true );
    }
  }
  //result -> debug ();
  return result;
}
#endif
inline void PointerTree::assign ( boost::shared_ptr<const CompressedTree> compressed ) {
  const bool LEAF = false;
  const bool NOT_A_LEAF = true;

  const std::vector<bool> & leaf_sequence = compressed -> leaf_sequence;
  const std::vector<bool> & valid_sequence = compressed -> valid_sequence;
  size_t N = leaf_sequence . size ();
  std::stack<std::pair<PointerTreeNode *, int> > path_to_root;
  for ( size_t i = 0; i < nodes_ . size (); ++ i ) delete nodes_[i];
  nodes_ . clear ();
  size_ = 0;
  PointerTreeNode * sentinel = new PointerTreeNode;  
  path_to_root . push ( std::make_pair (sentinel, 0) );
  int64_t last_encountered_leaf = -1;
  for ( size_t i = 0; i < N; ++ i ) {
    while ( path_to_root . top () . second == 2 ) path_to_root . pop ();
    PointerTreeNode * parent = path_to_root . top () . first;
    int child_num = path_to_root . top () . second;
    path_to_root . pop ();
    path_to_root . push ( std::make_pair ( parent, child_num + 1 ) );
    if ( leaf_sequence [ i ] == LEAF ) {
      ++ last_encountered_leaf;
      if ( not valid_sequence [ last_encountered_leaf ] ) continue;
    }
    PointerTreeNode * node = new PointerTreeNode;
    nodes_ . push_back ( node );
    node -> parent_ = parent;
    node -> contents_ = size_ ++;
    if ( child_num == 0 ) {
      parent -> left_ = node;
    } else {
      parent -> right_ = node;
    }
    if ( leaf_sequence [ i ] == NOT_A_LEAF ) {
      path_to_root . push ( std::make_pair ( node, 0 ) );
    }
    
  }
  sentinel -> left_ -> parent_ = NULL;
  delete sentinel;
}

inline void PointerTree::debug ( void ) const {
  // DEBUG:
  std::cout << "Pointertree debug output:\n";
  std::cout << "address = " << this << "\n";
  for ( uint64_t i = 0; i < nodes_.size(); ++ i ) {
    std::cout << i << " == " << nodes_[i] -> contents_ << "\n";
    if ( nodes_[i] -> parent_ != NULL ) {
      std::cout << i << ":" << nodes_[i]->parent_->contents_ << " -> " << nodes_[i]->contents_ << "\n";
      if ( nodes_[i] -> left_ != NULL ) {
        if ( nodes_[i] -> left_ -> contents_ >= size () ) {
          std::cout << "contents wrong! " << nodes_[i] -> left_ -> contents_ << "\n";
          abort ();
        }
      }
      if ( nodes_[i] -> right_ != NULL ) {
        if ( nodes_[i] -> right_ -> contents_ >= size () ) {
          std::cout << "contents wrong!" << nodes_[i] -> right_ -> contents_ << "\n";
          abort ();
        }
      }
    }
  }
  // method 2. Recurse.
  std::stack < Tree::iterator > stack_of_nodes;
  stack_of_nodes . push ( begin () );
  while ( not stack_of_nodes . empty () ) {
    Tree::iterator it = stack_of_nodes . top ();
    stack_of_nodes . pop ();
    if ( it == end () ) continue;
    if ( nodes_ [ *it ] -> contents_ != *it ) {
      std::cout << "Content damaged.\n";
      abort ();
    }
    stack_of_nodes . push ( left ( it ) );
    stack_of_nodes . push ( right ( it ) );

  }
  std::cout << "finish pointertree debug output\n";
}

#endif
