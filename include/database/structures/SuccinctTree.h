#ifndef CMDB_SUCCINCTTREE_H
#define CMDB_SUCCINCTTREE_H
//#define SDSL_DEBUG_BP
/// @file SuccinctTree.h
/// @author Arnaud Goullet, Shaun Harker
/// @description This file defines class SuccinctTree which 
/// provides an implementation of a full binary tree using SDSL.
#include <exception>
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/iterator/counting_iterator.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/version.hpp"
#include "boost/serialization/split_member.hpp"
#include "sdsl/bp_support.hpp"
#include "sdsl/rank_support_v5.hpp"
#include "sdsl/select_support_mcl.hpp"
#include "sdsl/util.hpp"
#include "database/structures/Tree.h"

/// SuccinctTree
///   Implements a full binary tree using SDSL
class SuccinctTree : public Tree {
public:
  typedef boost::counting_iterator < uint64_t > iterator;
  typedef iterator const_iterator;
  typedef uint64_t value_type;
  typedef uint64_t size_type;

  /// SuccinctTree
  ///   Default constructor. Creates root node with no children.
  SuccinctTree ( void );

  /// assign
  ///   Reset structure as if it had been constructed with leaf_sequence
  virtual void assign ( boost::shared_ptr<const CompressedTree> compressed );

  /// assignFromLeafSequence 
  void assignFromLeafSequence ( const std::vector<bool> & leaf_sequence );

  /// leafEnd
  ///   Give the one-past-the-end leaf (i.e. return number of leaves)
  uint64_t leafEnd ( void ) const;

  /// TreeToLeaf
  ///    Given a tree iterator, return a leaf iterator
  uint64_t TreeToLeaf ( iterator it ) const;

  /// LeafToTree
  ///    Given a leaf iterator, return a tree iterator
  iterator LeafToTree ( uint64_t leaf ) const;

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
 
  /// memory 
  ///   Return the memory usage of data structure in bytes
  uint64_t memory ( void ) const;

  /// leafSequence 
  ///   Return a reference to the underlying bit_vector
  const sdsl::bit_vector & leafSequence ( void ) const;

private:
  sdsl::bit_vector leaf_sequence_;
  sdsl::bp_support_sada < > tree_;
  sdsl::rank_support_v5 <0> rank_;
  sdsl::select_support_mcl <0> select_;
  uint64_t leaf_count_;
  friend class boost::serialization::access;
  template<class Archive>
  void save ( Archive & ar , const unsigned int version ) const;
  template<class Archive>
  void load ( Archive & ar , const unsigned int version );
  BOOST_SERIALIZATION_SPLIT_MEMBER ( );
};

#if 0
BOOST_CLASS_EXPORT_KEY(SuccinctTree);
#endif

inline 
SuccinctTree::SuccinctTree ( void ) {
  // TODO: default constructor
}

inline void 
SuccinctTree::assign ( boost::shared_ptr<const CompressedTree> compressed ) {
  const std::vector < bool > & leaf_sequence = compressed -> leaf_sequence;
  assignFromLeafSequence ( leaf_sequence );
}

inline void 
SuccinctTree::assignFromLeafSequence ( const std::vector<bool> & leaf_sequence ) {
  leaf_count_ = 0; 
  size_ = leaf_sequence . size ();
  leaf_sequence_ . resize ( leaf_sequence . size () + 1 );
  leaf_sequence_ [ 0 ] = 1;
  for ( size_t i = 0; i < leaf_sequence . size (); ++ i ) {
    leaf_sequence_ [ i + 1 ] = (leaf_sequence [ i ] ? 1 : 0 );
    if ( not leaf_sequence [ i ] ) ++ leaf_count_;
  }
  tree_ = sdsl::bp_support_sada <> ( & leaf_sequence_ );
  rank_ . set_vector ( &leaf_sequence_ );
  select_ . set_vector ( &leaf_sequence_ );

#if 0
  // BEGIN DEBUG

  for ( uint64_t i = 0; i < leaf_sequence.size (); ++ i ) {
    std::cout << (leaf_sequence [ i ] ? "1":"0");
  }
  std::cout << "\n";

  std::cout << "SuccinctTree::assignFromLeafSequence. last_ = " << last_ << "\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. size_ = " << size_ << "\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. leaf_count_ = " << leaf_count_ << "\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. leaf_sequence_ = " << leaf_sequence_ . size () << "\n";

  std::cout << "SuccinctTree::assignFromLeafSequence. Beginning basic sanity test\n";
  boost::unordered_set<uint64_t> left_children;
  boost::unordered_set<uint64_t> right_children;
  boost::unordered_set<uint64_t> parents;

  if ( leaf_sequence [ 0 ] == 0 ) {
    std::cout << "The root is a leaf.\n";
  }
  int64_t excess = 0;
  for ( uint64_t i = 0; i < leaf_sequence_.size (); ++ i ) {
    if ( leaf_sequence_ [ i ] == 1 ) ++ excess;
    else -- excess;
    //std::cout << excess << "\n";
  }

  for ( uint64_t i = 0; i < leaf_sequence_.size (); ++ i ) {
    if ( leaf_sequence_ [ i ] == 0 ) continue;
    std::cout << "tree_.find_close(" << i << ") = " <<
                    tree_.find_close(i) << "   ";
    // brute force calculation.
    int64_t brute = 0;
    uint64_t j = i;
    for ( ; j < leaf_sequence_.size (); ++ j) {
      if ( leaf_sequence_ [ j ] == 1 ) ++ brute;
      else -- brute;
      if ( brute == 0 ) break;
    }
    std::cout << "brute force: find_close(" << i << ") = " << j << "\n";
  }

  for ( uint64_t i = 0; i < leaf_sequence_.size (); ++ i ) {
    if ( leaf_sequence_ [ i ] == 1 ) continue;
    std::cout << "tree_.find_open(" << i << ") = " <<
                    tree_.find_open(i) << "\n";
  }
  for ( uint64_t i = 0; i < size (); ++ i ) {
    iterator it ( i );
    if ( left ( it ) != end () ) left_children . insert ( * left(it) );

    if ( right ( it ) != end () ) {
      if ( right_children . count ( * right(it) ) ) {
        std::cout << *it << " -?-R-?-> " <<  * right ( it ) << "\n";
        std::cout << "tree_.find_close(" << *it << ") = " <<
                    tree_.find_close(*it) << "\n";
        std::cout << "leaf_sequence_[" << *it << "] = " <<
                    leaf_sequence_[*it] << "\n";
        //throw std::logic_error("Found the same right child twice!\n");
      }
      right_children . insert ( * right(it) );
    }
    if ( parent ( it ) != end () ) parents . insert ( * parent(it) );
    
    if ( left(it) == it ) std::cout << "Left is wrong.\n";
    if ( right(it) == it ) std::cout << "Right is wrong.\n";
    if ( parent(it) == it ) std::cout << "Parent is wrong.\n";


    if ( left ( it) != end() && parent(left(it)) != it ) {
      std::logic_error ( " parent(left(it))!=it in succinct tree\n" );
    }
    if ( right ( it) != end() && parent(right(it)) != it ) {
      std::logic_error ( " parent(right(it))!=it in succinct tree\n" );
    }
    if ( parent (it) != end() && right(parent(it))!=it && left(parent(it))!=it ) {
      std::logic_error ( " neither of parent's children is self\n" );
    }
    if ( parent (it) != end() && right(parent(it))==left(parent(it)) ) {
      std::logic_error ( " parent has identical children\n" );
    }
  }
  std::cout << "SuccinctTree::assignFromLeafSequence. Passed basic sanity test\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. left_children = " << left_children.size() << "\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. right_children = " << right_children.size() << "\n";
  std::cout << "SuccinctTree::assignFromLeafSequence. parents = " << parents.size() << "\n";

  // END DEBUG
#endif
}

inline uint64_t 
SuccinctTree::leafEnd ( void ) const {
  return leaf_count_;
}

inline uint64_t 
SuccinctTree::TreeToLeaf ( iterator it ) const {
  uint64_t x = *it + 1;
  if ( leaf_sequence_ [ x ] == 1 ) return leafEnd ();
  return rank_ . rank ( x );
}

inline SuccinctTree::iterator 
SuccinctTree::LeafToTree ( uint64_t leaf ) const {
  //std::cout << "SuccinctTree::LeafToTree  " << leaf << ", " << size_ << " " << leaf_count_ << "\n";
  //for ( size_t i = 0; i < leaf_sequence_ . size (); ++ i ) {
  //  std::cout << (leaf_sequence_ [ i ] ? "1" : "0");
  //}
  //std::cout << "\n";
  return iterator ( select_ . select ( leaf + 1 ) - 1 );
}

inline SuccinctTree::iterator 
SuccinctTree::parent ( iterator it ) const {
  uint64_t x = *it;
  if ( x == 0 ) return end ();
  if ( leaf_sequence_ [ x ] == 1 ) return iterator ( x - 1 );
  return iterator ( tree_ . find_open ( x ) - 1 );
}

inline SuccinctTree::iterator 
SuccinctTree::left ( iterator it ) const {
  uint64_t x = *it + 1;
  if ( leaf_sequence_ [ x ] == 0 ) return end ();
  return iterator ( x );
}

inline SuccinctTree::iterator 
SuccinctTree::right ( iterator it ) const {
  uint64_t x = *it + 1;
  if ( leaf_sequence_ [ x ] == 0 ) return end ();
  return iterator ( tree_ . find_close ( x ) );
}

inline bool 
SuccinctTree::isLeft ( iterator it ) const {
  uint64_t x = *it;
  if ( x == 0 ) return false;
  if ( leaf_sequence_ [ x ] == 1 ) return true;
  return false;
}

inline bool 
SuccinctTree::isRight ( iterator it ) const {
  uint64_t x = *it;
  if ( x == 0 ) return false;
  if ( leaf_sequence_ [ x ] == 1 ) return false;
  return true;
}

inline bool 
SuccinctTree::isLeaf ( iterator it ) const {
  uint64_t x = *it + 1;
  if ( leaf_sequence_ [ x ] == 0 ) return true;
  return false;
}

inline uint64_t 
SuccinctTree::memory ( void ) const {
  //std::cout << "SuccinctTree::memory \n";
  //std::cout << "sdsl::util::get_size_in_bytes ( leaf_sequence_ ) = " << sdsl::util::get_size_in_bytes ( leaf_sequence_ ) << "\n";
  //std::cout << "sdsl::util::get_size_in_bytes ( tree_ ) = " << sdsl::util::get_size_in_bytes ( tree_ ) << "\n";
  //std::cout << "sdsl::util::get_size_in_bytes ( rank_ ) = " << sdsl::util::get_size_in_bytes ( rank_ ) << "\n";
  //std::cout << "sdsl::util::get_size_in_bytes ( select_ ) = " << sdsl::util::get_size_in_bytes ( select_ ) << "\n";

  return sizeof ( SuccinctTree ) + 
         sdsl::util::get_size_in_bytes ( leaf_sequence_ ) +
         sdsl::util::get_size_in_bytes ( tree_ ) +
         sdsl::util::get_size_in_bytes ( rank_ ) +
         sdsl::util::get_size_in_bytes ( select_);
}

inline const sdsl::bit_vector & 
SuccinctTree::leafSequence ( void ) const {
  return leaf_sequence_;
}

template<class Archive>
void SuccinctTree::save ( Archive & ar , const unsigned int version ) const {
  ar & boost::serialization::base_object<Tree>(*this);
  std::vector < bool > leaf_sequence ( size_ );
  for ( size_t i = 1; i < leaf_sequence_ . size (); ++ i ) {
    leaf_sequence [ i - 1 ] = leaf_sequence_ [ i ];
  }
  ar & leaf_sequence;
}

template<class Archive>
void SuccinctTree::load ( Archive & ar , const unsigned int version ) {
  ar & boost::serialization::base_object<Tree>(*this);
  std::vector < bool > leaf_sequence;
  ar & leaf_sequence;
  assignFromLeafSequence ( leaf_sequence );
}

#endif
