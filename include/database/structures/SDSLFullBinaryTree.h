#ifndef SDSLFULLBINARYTREE_H
#define SDSLFULLBINARYTREE_H

#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/bind.hpp>

// includes for serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>

// includes for sdsl library : succinct binary tree represensation
#include "sdsl/bp_support.hpp"
#include "sdsl/util.hpp"

/**
 * @file
 * @author Arnaud Goullet
 * @version 1.0
 * 
 * @description
 * Implementation of binary tree class using a succinct representation.
 * Standard operations and queries are provided. 
 */

class SDSLFullBinaryTree {
public:
  // Typedefs for the iterator
  typedef boost::counting_iterator < uint64_t > iterator;
  typedef iterator const_iterator;
  typedef uint64_t value_type;
  typedef uint64_t size_type;
  //

  iterator begin ( void ) const {
    return 0;
  }

  iterator end ( void ) const {
    return size_;
  }

  // Constructors

  /**
   * Constructor for minimal Full Binary Tree with just one node 
   */
  SDSLFullBinaryTree ( );
  /** 
   * Constructor from a balanced parentheses sequence using the DFUDS convention 
   * @param v : Boolean vector representing the balanced parentheses sequence. 
   * Convention : DFUDS ordering with '0' for an opening parentheses, '1' for a closing parentheses.
   */
  SDSLFullBinaryTree ( std::vector < bool > & v );

  // 

  SDSLFullBinaryTree& operator= ( const SDSLFullBinaryTree& other ) {
    bp_ = other . bp_;
    init_structures_ ( );
    size_ = bp_ . size ( ) / 2;
    end_ = other . end_;
    return *this;
  }

  // Features

  /** 
   * Subdivide a particular node in the Full Binary Tree.
   * The iterator should point to a leaf, otherwise do nothing.
   * @param it
   */
  void subdivide ( iterator it );
  /**
   * Subdivide a list of leaves 
   * @param it_list
   */
  void subdivide ( std::vector < iterator > it_list );
  /** 
   * Subdivide all the leaves in the Full Binary Tree.
   */
  void subdivide ( void );
  /** 
   * Erase the subtree under a particular node in the tree.
   * To keep the full binary tree structure, the node pointed by the iterator is being kept
   * @param it
   */
  void erase ( iterator it );
  /**
   * Given a list of iterators, erase the subtrees under them.
   * The iterators are sorted and filtered out if they belong to the same subtree. 
   * @param it_list
   */
  void erase ( std::vector<iterator> it_list );
  //

  // Queries

  /**
   * @param it
   * @return the iterator pointing to the parent node.
   */
  iterator parent ( iterator it ) const;
  /**
   * @param it
   * @return the iterator pointing to the next node, that does not belong to the subtree of 'it'.
   */
  iterator next ( iterator it ) const;
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
   * @return true if the iterator it is a left child.
   */
  bool isleft ( iterator it ) const;
  /**
   * @param it
   * @return true if the iterator it is a right child.
   */
  bool isright ( iterator it ) const;
  /**
   * @param it
   * @return true if the iterator it is a leaf.
   */
  bool isleaf ( iterator it ) const;
  /**
   * @param it
   * @return the path from the root node to the iterator.
   * With convention '0' left branch and '1' right branch. 
   */
  // could use vector < bool > 
  std::vector < unsigned char > prefix ( iterator it ) const;
  /**
   * @param it
   * @return the depth of a given node.
   */
  size_type depth ( iterator it ) const;

  /**
   * 
   * @param leaves
   * @return the minimal Full Binary Tree constructed from a set of leaves. 
   */
  SDSLFullBinaryTree subtree ( std::vector < iterator > & leaves );


  /**
   * @param it
   * @return the size of the subtree of a given node which is included in the subtree.
   */
  size_type subtree_size ( iterator it ) const;
  /**
   * @param it
   * @return a list of iterators covering the subtree of a given node. 
   * The first element of the list is it.  
   */
  std::vector < iterator > subtree_list ( iterator it ) const;
  //


  /**
   * 
   * @return the size of the Full Binary Tree. 
   */
  size_type size ( void ) const;
  /**
   * Display the memory usage of the Full Binary Tree.
   */
  void memory_usage ( void ) const;
  /**
   * Display the balanced parentheses sequence.
   */
  void display_bp ( void ) const;

  /**
   * Display information and statistic regarding the balanced parentheses representation.
   */
  void info ( void ) const {
    tree_ . get_info ( );
  }
  /**
   * Export the Full Binary Tree to a file "filename" in the GraphViz format .
   * @param filename
   */
  void export_graphviz ( const char * filename );
  /**
   * 
   * @return the balanced parentheses sequence of the Full Binary Tree.
   */
  std::vector < bool > bp_sequence ( void ) const;

  /**
   * 
   * @param it
   * @return the position of the opening parentheses in the balanced parentheses sequence of a given node. 
   */
  size_type opening_position ( iterator it ) const {
    return find_position_ ( it );
  }

  /**
   * 
   * @param it
   * @return the position of the closing parentheses in the balanced parentheses sequence of a given node. 
   */
  size_type closing_position ( iterator it ) const {
    return tree_ . find_close ( find_position_ ( it ) );
  }

  //
private:
  /**
   * Store the balanced parentheses sequence of the Full Binary Tree.
   */
  sdsl::bit_vector bp_;
  /**
   * Store the structure of the succinct representation of the Full Binary Tree,
   * to have access to Binary Tree queries in constant time. 
   */
  sdsl::bp_support_sada < > tree_;

  iterator begin_;
  iterator end_;
  size_type size_;

  size_type find_position_ ( iterator me ) const;
  /**
   * Initialize the structure of the succinct representation of the Full Binary Tree
   *  from the balanced parentheses sequence,
   * to have access to Binary Tree queries in constant time. 
   */
  void init_structures_ ( void );

  friend class boost::serialization::access;
  template<class Archive>
  void save ( Archive & ar , const unsigned int version ) const {
    std::vector < bool > bit_sequence;
    for ( size_type i = 0; i < 2*size_; ++ i ) bit_sequence . push_back ( bp_ [ i ] );
    ar & bit_sequence;   
  }
  template<class Archive>
  void load ( Archive & ar , const unsigned int version ) {
    vector < bool > bit_sequence;
    ar & bit_sequence;
    bp_ . resize ( bit_sequence . size ( ) );
    size_ = bp_ . size ( ) / 2;
    begin_ = 0;
    end_ = size_;
    for ( size_type i = 0; i < 2*size_; ++ i ) bp_ [ i ] = bit_sequence [ i ];
    init_structures_ ( );
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER ( );
  
};

inline void SDSLFullBinaryTree::erase ( iterator erase_me ) {
  // position of the opening parentheses
  size_type position ( find_position_ ( erase_me ) );
  // position of the closing parentheses
  size_type position_end ( tree_ . find_close ( position ) );
  //
  sdsl::bit_vector tmp ( bp_ );
  size_type newsize = size_ - ( position_end - position - 1 ) / 2;
  bp_ . resize ( 2 * newsize );
  // copy the first part
  std::copy ( tmp.begin ( ) , tmp.begin ( ) + position + 1 , bp_.begin ( ) );
  // copy the second part
  std::copy ( tmp.begin ( ) + position_end , tmp.end ( ) , bp_.begin ( ) + position + 1 );
  //
  size_ = newsize;
  init_structures_ ( );
  end_ = iterator ( size_ );
}

inline void SDSLFullBinaryTree::erase ( std::vector<iterator> it_list ) {

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

  // Now that we know the list of iterators to erase
  // we reconstruct the proper balanced parentheses sequence 

  pos = 0;
  std::vector < bool > newbp;

  BOOST_FOREACH ( iterator it , it_list2 ) {
    // position of the opening parentheses
    size_type position ( find_position_ ( it ) );
    // position of the closing parentheses
    size_type position_end ( tree_ . find_close ( position ) );
    //
    for ( size_type j = pos; j <= position; ++ j ) {
      newbp . push_back ( bp_ [ j ] );
    }
    pos = position_end;
  }
  // copy the last piece 
  for ( size_type j = pos; j < 2 * size_; ++ j ) {
    newbp . push_back ( bp_ [ j ] );
  }

  // this is probably not the best way
  bp_ . resize ( newbp.size ( ) );
  for ( size_type i = 0; i < newbp.size ( ); ++ i ) {
    bp_ [ i ] = newbp[i];
  }

  init_structures_ ( );
  size_ = ( int ) newbp.size ( ) / 2;
  end_ = iterator ( size_ );

}

inline std::vector < unsigned char > SDSLFullBinaryTree::prefix ( iterator it ) const {
  std::vector < unsigned char > result , reversed;
  iterator it2 ( it );
  while ( parent ( it2 ) != end ( ) ) {
    if ( isleft ( it2 ) ) {
      reversed . push_back ( 0 );
    } else {
      reversed . push_back ( 1 );
    }
    it2 = parent ( it2 );
  }
  for ( int i = reversed . size ( ) - 1; i >= 0; -- i ) {
    result . push_back ( reversed [ i ] );
  }
  return result;
}

inline void SDSLFullBinaryTree::subdivide ( iterator it ) {
  //
  if ( isleaf ( it ) ) {
    size_type position ( find_position_ ( it ) );
    //
    sdsl::bit_vector tmp ( bp_ );
    bp_ . resize ( 2 * size_ + 4 );
    // copy the first part
    std::copy ( tmp.begin ( ) , tmp.begin ( ) + position , bp_.begin ( ) );
    // insert the two new nodes
    bp_ . set_int ( position , 11 , 6 );
    // copy the second part
    std::copy ( tmp.begin ( ) + position + 2 , tmp.end ( ) , bp_.begin ( ) + position + 6 );
    //
    size_ += 2;
    init_structures_ ( );
    end_ = iterator ( size_ );
  } else {
    std::cout << "Trying to subdivide the interior node " << * it << ", nothing done.\n";
  }
}

inline void SDSLFullBinaryTree::subdivide ( std::vector<iterator> it_list ) {
  // first the list of iterators, just in case
  boost::sort ( it_list );

  // We should check for leaf or not
  std::vector<iterator> newlist;

  BOOST_FOREACH ( iterator it , it_list ) {
    if ( isleaf ( it ) ) newlist . push_back ( it );
  }

  std::vector < bool > newbp;

  size_type pos;
  pos = 0;

  BOOST_FOREACH ( iterator it , newlist ) {
    size_type position ( find_position_ ( it ) );
    //
    for ( size_type i = pos; i < position; ++ i ) {
      newbp . push_back ( bp_[i] );
    }
    // add the two leaves
    newbp . push_back ( 1 );
    newbp . push_back ( 1 );
    newbp . push_back ( 0 );
    newbp . push_back ( 1 );
    newbp . push_back ( 0 );
    newbp . push_back ( 0 );
    pos = position + 2;
  }

  // copy the last piece
  for ( size_type i = pos; i < 2 * size_; ++ i ) newbp.push_back ( bp_[i] );

  size_ = newbp.size ( ) / 2;
  end_ = iterator ( size_ );
  bp_ . resize ( newbp.size ( ) );
  for ( size_type i = 0; i < bp_.size ( ); ++ i ) bp_[i] = newbp[i];
  init_structures_ ( );
}

inline void SDSLFullBinaryTree::subdivide ( void ) {
  //
  int newtreesize = ( size_ + 2 * ( ( size_ - 1 ) / 2 + 1 ) );
  sdsl::bit_vector tmp ( 2 * size_ );
  tmp = bp_; // better to make a copy when vector are smaller
  bp_ . resize ( 2 * newtreesize );
  int counter = 0;
  unsigned int i = 0;
  while ( i < 2 * size_ - 1 ) {
    if ( tmp [ i ] == 1 && tmp [ i + 1 ] == 0 ) {
      // if '10' is encountered substitute with 110100=11 with 6 bits
      bp_ . set_int ( counter , 11 , 6 );
      counter += 6;
      i += 2;
    } else {
      bp_ [ counter ] = tmp [ i ];
      counter ++;
      i ++;
    }
  }
  init_structures_ ( );
  size_ = newtreesize;
  end_ = iterator ( size_ );
}

inline SDSLFullBinaryTree SDSLFullBinaryTree::subtree ( std::vector < iterator > & leaves ) {

  std::vector < iterator > visited;

  BOOST_FOREACH ( iterator it_node , leaves ) {
    visited . push_back ( *it_node );
    while ( it_node != begin ( ) ) {
      it_node = parent ( it_node );
      visited . push_back ( *it_node );
    }
  }

  // find the closing and opening parentheses position for the visited nodes from 
  // the fulltree_ representation
  // oc contains the position of opening parentheses (bool=1) or closing parentheses (bool=0)
  std::vector < std::pair < size_type , bool > > oc;

  BOOST_FOREACH ( iterator it_node , visited ) {
    std::pair < size_type , bool > pair;
    pair . first = opening_position ( it_node );
    pair . second = 1;
    oc . push_back ( pair );
    pair . first = closing_position ( it_node );
    pair . second = 0;
    oc . push_back ( pair );
  }

  // we sort oc with respect to the first argument of the pair and remove any repetition
  // in case leaves have common ancestor besides the root node
  std::sort ( oc.begin ( ) , oc.end ( ) ,
          boost::bind ( &std::pair<size_type , bool>::first , _1 ) <
          boost::bind ( &std::pair<size_type , bool>::first , _2 ) );
  boost::erase ( oc , boost::unique<boost::return_found_end > ( oc ) );

  // construction of the binary tree
  std::vector < bool > newbp;
  // root always there and active
  newbp . push_back ( 1 );
  //
  for ( unsigned int i = 1; i < oc.size ( ); ++ i ) {
    if ( oc[i].first - oc[i - 1].first > 1 ) {
      newbp . push_back ( 1 );
      newbp . push_back ( 0 );
    }
    newbp . push_back ( oc[i].second );
  }

  SDSLFullBinaryTree newtree ( newbp );

  return newtree;

}

inline std::vector < SDSLFullBinaryTree::iterator > SDSLFullBinaryTree::subtree_list ( iterator it ) const {
  iterator it_begin , it_end;
  it_begin = it;
  it_end = next ( it );
  std::vector < iterator > list;
  for ( iterator itt = it_begin; it != it_end; ++ itt ) list . push_back ( itt );
  return list;
}

inline SDSLFullBinaryTree::iterator SDSLFullBinaryTree::parent ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  if ( position > 0 ) {
    return iterator ( tree_ . rank ( ( tree_ . enclose ( position ) ) ) - 1 );
  } else {
    return iterator ( size_ );
  }
}

inline SDSLFullBinaryTree::iterator SDSLFullBinaryTree::left ( iterator it ) const {
  if ( ! isleaf ( it ) ) {
    return ++it;
  } else {
    return iterator ( size_ );
  }
}

inline SDSLFullBinaryTree::iterator SDSLFullBinaryTree::right ( iterator it ) const {
  // this if is not necessary
  if ( ! isleaf ( it ) ) {
    return next ( left ( it ) );
  } else {
    return iterator ( size_ );
  }
}

inline SDSLFullBinaryTree::iterator SDSLFullBinaryTree::next ( iterator it ) const {
  if ( *it < size_ - 1 ) {
    size_type position ( find_position_ ( it ) );
    return iterator ( tree_ . rank ( ( tree_ . find_close ( position ) + 1 ) ) - 1 );
  } else {
    return iterator ( size_ );
  }
}

inline bool SDSLFullBinaryTree::isleft ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  if ( position > 0 ) {
    return bp_ . get_int ( position - 1 , 1 );
  } else {
    return false; // in case it is root
  }
}

inline bool SDSLFullBinaryTree::isright ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  if ( position > 0 ) {
    return ! bp_ . get_int ( position - 1 , 1 );
  } else {
    return false; // in case it is root
  }
}

inline bool SDSLFullBinaryTree::isleaf ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  return ! bp_ . get_int ( position + 1 , 1 );
}

inline SDSLFullBinaryTree::size_type SDSLFullBinaryTree::depth ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  // -1 to have root node depth = 0
  return tree_ . excess ( position ) - 1;
}

inline SDSLFullBinaryTree::size_type SDSLFullBinaryTree::subtree_size ( iterator it ) const {
  size_type position ( find_position_ ( it ) );
  return ( tree_ . find_close ( position ) - position + 1 ) / 2;
}

inline SDSLFullBinaryTree::size_type SDSLFullBinaryTree::size ( void ) const {
  return size_;
}

inline SDSLFullBinaryTree::SDSLFullBinaryTree ( ) {
  //
  size_ = 1;
  bp_ . resize ( 2 * size_ );
  bp_ [ 0 ] = 1;
  bp_ [ 1 ] = 0;
  //
  init_structures_ ( );
  //
  begin_ = iterator ( 0 );
  end_ = iterator ( size_ );
}

inline SDSLFullBinaryTree::SDSLFullBinaryTree ( std::vector < bool > & v ) {
  //
  size_ = v . size ( ) / 2;
  bp_ . resize ( 2 * size_ );
  // BAD IMPLEMENTATION TO BE OPTIMIZED
  for ( unsigned int i = 0; i < 2 * size_; ++ i ) {
    bp_ [ i ] = v [ i ];
  }
  //
  init_structures_ ( );
  //
  begin_ = iterator ( 0 );
  end_ = iterator ( size_ );
}

inline std::vector < bool > SDSLFullBinaryTree::bp_sequence ( void ) const {
  std::vector < bool > output;
  for ( unsigned int i = 0; i < 2 * size_; ++ i ) {
    output . push_back ( bp_ [ i ] );
  }

  return output;
}

inline void SDSLFullBinaryTree::init_structures_ ( void ) {
  // There should be a better way
  sdsl::bp_support_sada < > tmp ( & bp_ );
  tree_ = tmp;
  //tree_ = sdsl::bp_support_sada < > ( & bp_ );
}

inline void SDSLFullBinaryTree::display_bp ( void ) const {
  std::cout << "\nBalanced parentheses sequence : ";
  std::cout << bp_;
  std::cout << "\n";
}

inline void SDSLFullBinaryTree::memory_usage ( void ) const {
  //
  double bp_mem_size ( sdsl::util::get_size_in_bytes ( bp_ ) );
  double tree_mem_size ( sdsl::util::get_size_in_bytes ( tree_ ) );
  //
  std::cout << "\nMemory usage for SDSLFullBinaryTree : ";
  std::cout << 8 * ( bp_mem_size + tree_mem_size ) / size_ << " bits per node\n";
  //
}

inline void SDSLFullBinaryTree::export_graphviz ( const char *filename ) {
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
        iterator it2;
        it2 = parent ( it );
        myfile << "\"" << * it2
                << "\"->\""
                << * it << "\";" << std::endl;
      }
    }
  }
  myfile << "}" << std::endl;
  myfile.close ( );
}

inline SDSLFullBinaryTree::size_type SDSLFullBinaryTree::find_position_ ( iterator it ) const {
  //std::cout << "SDSL find position of ( " << *it << ")\n";
  //std::cout << " answer = " << tree_ . select ( * it + 1 ) << "\n";
  return tree_ . select ( * it + 1 );
}

//
#endif
