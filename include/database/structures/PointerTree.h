/// PointerTree.h
/// Shaun Harker

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

/// PointerTreeNode
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

inline PointerTreeNode::PointerTreeNode ( void ) 
: left_ ( NULL ), right_ ( NULL ), 
  parent_ ( NULL ), contents_ ( 0 ) {
}

inline PointerTreeNode::~PointerTreeNode ( void ) {
}


/// PointerTree
class PointerTree : public Tree {
public:
  PointerTree ( void );
  virtual ~PointerTree ( void );
  virtual iterator parent ( iterator it ) const;
  virtual iterator left ( iterator it ) const;
  virtual iterator right ( iterator it ) const;
  virtual bool isLeft ( iterator it ) const;
  virtual bool isRight ( iterator it ) const;
  virtual bool isLeaf ( iterator it ) const;
  virtual void assign ( boost::shared_ptr<const CompressedTree> compressed );

private:
  std::vector < PointerTreeNode * > nodes_;
  friend class boost::serialization::access;
  template<typename Archive>
  void serialize(Archive & ar, const unsigned int file_version) {
    ar & boost::serialization::base_object<Tree>(*this);
    ar & nodes_;
  }
public:
  virtual uint64_t memory ( void ) const {
    return sizeof ( PointerTree ) +
           sizeof ( PointerTreeNode * ) * nodes_ . size () +
           sizeof ( PointerTreeNode ) * nodes_ . size ();
  }
};

BOOST_CLASS_EXPORT_KEY(PointerTree);

inline PointerTree::PointerTree ( void ) {
  PointerTreeNode * node = new PointerTreeNode;
  node -> contents_ = 0;
  nodes_ . push_back ( node );
}
inline PointerTree::~PointerTree ( void ) {
  for ( size_t i = 0; i < nodes_ . size (); ++ i ) {
    delete nodes_[i];
  }
}

inline Tree::iterator PointerTree::parent ( iterator it ) const {
  if ( it == end () ) return end ();
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> parent_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
}

inline Tree::iterator PointerTree::left ( iterator it ) const {
  if ( it == end () ) return end ();
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> left_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
  
}

inline Tree::iterator PointerTree::right ( iterator it ) const {
  if ( it == end () ) return end ();
  PointerTreeNode * node = nodes_ [ * it ];
  node = node -> right_;
  if ( node == NULL ) return end ();
  return iterator ( node -> contents_ );
  
}

inline bool PointerTree::isLeft ( iterator it ) const {
  if ( it == end () ) return false;
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> left_ == node );
}

inline bool PointerTree::isRight ( iterator it ) const {
  if ( it == end () ) return false;
  PointerTreeNode * node = nodes_ [ * it ];
  PointerTreeNode * parent = node -> parent_;
  if ( parent == NULL ) return false;
  return (parent -> right_ == node );
}

inline bool PointerTree::isLeaf ( iterator it ) const {
  if ( it == end () ) return false;
  PointerTreeNode * node = nodes_ [ * it ];
  if ( ( node -> left_ == NULL ) && ( node -> right_ == NULL ) ) return true;
  return false;
}

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

#endif
