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

#include <boost/foreach.hpp>

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
  virtual bool isleft ( iterator it ) const;
  virtual bool isright ( iterator it ) const;
  virtual bool isleaf ( iterator it ) const;
  
  // Mutation Methods
  virtual void subdivide ( void );
  virtual void adjoin ( const Tree & other );
  virtual PointerTree * subtree ( const std::deque < Tree::iterator > & leaves ) const;
  virtual void assign ( const CompressedTree & compressed );

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
    return sizeof ( std::vector < PointerTreeNode * > ) +
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
( const std::deque < Tree::iterator > & leaves ) const {
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

inline void PointerTree::assign ( const CompressedTree & compressed ) {
  
  const std::vector<bool> & balanced_parentheses = compressed . balanced_parentheses;
  const std::vector<bool> & valid_tree_nodes = compressed . valid_tree_nodes;
  
  for ( size_t i = 0; i < nodes_ . size (); ++ i ) delete nodes_[i];
  size_ = 0;
  
  // Method. We use a "sentinel" pointer as a placeholder for missing left children
  PointerTreeNode * sentinel = new PointerTreeNode; // assert( sentinel != NULL )
  PointerTreeNode * parent = NULL;
  size_t N = balanced_parentheses . size ();
  size_t bp_pos = 0;
  size_t valid_pos = 0;
  while ( bp_pos < N ) {
    if ( balanced_parentheses [ bp_pos ] ) {
      // An open parentheses ( 
      std::cout << "(";
      if ( valid_tree_nodes [ valid_pos ++ ] ) {
        // We have found a valid child.
        PointerTreeNode * child = new PointerTreeNode;
        child -> contents_ = size_;
        if ( parent ) {
          if ( parent -> left_ != NULL ) {
            std::cout << "right";
            parent -> right_ = child;
            child -> parent_ = parent;
            if ( parent -> left_ == sentinel ) parent -> left_ = NULL; // premature!
          } else {
            parent -> left_ = child;
            child -> parent_ = parent;
            std::cout << "left";
          }
        }
        nodes_ . push_back ( child );
        ++ size_;
        ++ bp_pos; // consume (
        parent = child;
      } else {
        // This is an invalid node, don't create it,
        //  just consume the parentheses and move on.
        if ( balanced_parentheses [ bp_pos + 1 ] != 0 ) {
          std::cout << "I will eat my shorts on live television A.\n";
          throw 1;
        }
        bp_pos += 2; // consume ()
        if ( balanced_parentheses [ bp_pos - 2 ] != 1 ||
             balanced_parentheses [ bp_pos - 1 ] != 0 ) {
          std::cout << "I will eat my shorts on live television A.\n";
          throw 1;
        }
        std::cout << "(invalid)"; abort ();
        if ( parent -> left_ != NULL ) {
          // It was the right child that was invalid.
          if ( parent -> left_ == sentinel ) parent -> left_ = NULL;
          parent -> right_ = NULL;
          // Rise up.
          parent = parent -> parent_;
          if ( balanced_parentheses [ bp_pos ] != 0 ) {
            std::cout << "I will eat my shorts on live television B.\n";
            throw 1;
          }
          ++ bp_pos; // consume )
          std::cout << ")";
        } else {
          // It was the left child that was invalid.
          // Mark it temporarily with "sentinel"
          parent -> left_ = sentinel;
        }
      }
    } else {
      std::cout << ")";
      // We have read a closed parenthesis )
      parent = parent -> parent_;
      ++ bp_pos; // consume )
    }
  }
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
    }
  }
  std::cout << "finish pointertree debug output\n";
}

#endif
