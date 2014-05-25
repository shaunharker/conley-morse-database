#ifndef SDSLFULLBINARYTREE_H
#define SDSLFULLBINARYTREE_H

/// @file SDSLFullBinaryTree.h
/// @author Arnaud Goullet, Shaun Harker
/// @description This file defines class SDSLFullBinaryTree which 
/// provides an implementation of a full binary tree using SDSL.
 
#include "boost/foreach.hpp"
#include "boost/iterator/counting_iterator.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/version.hpp"
#include "boost/serialization/split_member.hpp"
#include "sdsl/bp_support.hpp"
#include "sdsl/util.hpp"

/// SDSLFullBinaryTree
///   Implements a full binary tree using SDSL
class SDSLFullBinaryTree {
public:
  typedef boost::counting_iterator < uint64_t > iterator;
  typedef iterator const_iterator;
  typedef uint64_t value_type;
  typedef uint64_t size_type;

  /// SDSLFullBinaryTree
  ///   Default constructor. Creates root node with no children.
  SDSLFullBinaryTree ( void );

  /// SDSLFullBinaryTree
  ///   Constructor from a balanced parentheses sequence using the DFUDS convention 
  ///   @param leaf_sequence : A sequence representing which nodes are leaves.
  ///                          1 represents an interior node, 0 represents a leaf.
  SDSLFullBinaryTree ( std::vector < bool > & leaf_sequence );

  /// assign
  ///   Reset structure as if it had been constructed with leaf_sequence
  void assign ( std::vector < bool > & leaf_sequence );

  /// begin
  ///   STL-container style begin
  iterator begin ( void ) const;

  /// end
  ///   STL-container style end
  iterator end ( void ) const;
 
  /// size
  ///   @return the size of the Full Binary Tree. 
  size_type size ( void ) const;

  /// parent
  ///   @param it
  ///   @return the iterator pointing to the parent node.
  iterator parent ( iterator it ) const;
  
  /// left
  ///   @param it
  ///   @return the iterator pointing to the left child.
  iterator left ( iterator it ) const;
  
  /// right
  ///   @param it
  ///   @return the iterator pointing to the right child.
  iterator right ( iterator it ) const;
  
  /// isLeft
  ///   @param it
  ///   @return true if the iterator it is a left child.
  bool isLeft ( iterator it ) const;

  /// isRight
  ///   @param it
  ///   @return true if the iterator it is a right child.
  bool isRight ( iterator it ) const;

  /// isLeaf
  ///    @param it
  ///    @return true if the iterator it is a leaf.
  bool isLeaf ( iterator it ) const;
 
  /// depth
  ///   @return the depth of the node
  size_type depth ( iterator it ) const;

  /// memory 
  ///   Return the memory usage of data structure in bytes
  uint64_t memory ( void ) const;

  /// leafSequence 
  ///   Return a reference to the underlying bit_vector
  const sdsl::bit_vector & leafSequence ( void ) const;

private:
  sdsl::bit_vector leaf_sequence_;
  sdsl::bp_support_sada < > tree_;
  iterator begin_;
  iterator end_;
  size_type size_;
  friend class boost::serialization::access;
  template<class Archive>
  void save ( Archive & ar , const unsigned int version ) const;
  template<class Archive>
  void load ( Archive & ar , const unsigned int version );
  BOOST_SERIALIZATION_SPLIT_MEMBER ( );
};

inline 
SDSLFullBinaryTree::SDSLFullBinaryTree ( ) {
  std::vector<bool> leaf_sequence;
  assign ( leaf_sequence );
}

inline 
SDSLFullBinaryTree::SDSLFullBinaryTree ( const std::vector < bool > & leaf_sequence ) {
  assign ( leaf_sequence );
}

inline void 
SDSLFullBinaryTree::assign ( const std::vector < bool > & leaf_sequence ) {
}

inline SDSLFullBinaryTree::iterator 
SDSLFullBinaryTree::begin ( void ) const {
  return 0;
}

inline SDSLFullBinaryTree::iterator 
SDSLFullBinaryTree::end ( void ) const {
  return size_;
}

inline SDSLFullBinaryTree::size_type 
SDSLFullBinaryTree::size ( void ) const {
  return size_;
}

inline SDSLFullBinaryTree::iterator 
SDSLFullBinaryTree::parent ( iterator it ) const {
}

inline SDSLFullBinaryTree::iterator 
SDSLFullBinaryTree::left ( iterator it ) const {
}

inline SDSLFullBinaryTree::iterator 
SDSLFullBinaryTree::right ( iterator it ) const {
}

inline bool 
SDSLFullBinaryTree::isLeft ( iterator it ) const {
}

inline bool 
SDSLFullBinaryTree::isRight ( iterator it ) const {
}

inline bool 
SDSLFullBinaryTree::isLeaf ( iterator it ) const {
}

inline SDSLFullBinaryTree::size_type 
SDSLFullBinaryTree::depth ( iterator it ) const {
}

inline uint64_t 
SDSLFullBinaryTree::memory ( void ) const {
  return sdsl::util::get_size_in_bytes ( bp_ ) +
  sdsl::util::get_size_in_bytes ( tree_ );
}

inline const sdsl::bit_vector & 
SDSLFullBinaryTree::leafSequence ( void ) const {
  return leaf_sequence_;
}

template<class Archive>
void SDSLFullBinaryTree::save ( Archive & ar , const unsigned int version ) const {
}

template<class Archive>
void SDSLFullBinaryTree::load ( Archive & ar , const unsigned int version ) {
}

#endif
