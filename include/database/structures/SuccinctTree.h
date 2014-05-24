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
  // DEBUG BEGIN
  //std::cout << "SuccinctTree::initialize. bp size = " << bp . size () << " valid size = " << valid . size () << "\n";
  //std::cout << "SuccinctTree::initialize. fulltree_ . size () = " << fulltree_ . size () << "\n";
  // DEBUG END
  size_ = valid_rs_ . rank ( fulltree_ . size ( ) );
  // DEBUG BEGIN
  //std::cout << "SuccinctTree::initialize. size_ = " << size_ << "\n";
  // DEBUG END
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
  
  // Method : we construct the new balance parentheses sequence
  //            and rebuild the rank/select structures once at the end
  // Issue : need a copy of the balance parentheses and valid sequences
  //         since currently, we don't have access to their element

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
  size_ = valid_rs_ . rank ( newvalid . size () );
  //std::cout << "Postsubdivision:\n";
  //debug ();
}

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
  return valid_rs_ . select ( *it );
}

inline Tree::iterator SuccinctTree::FullToActual ( SDSLFullBinaryTree::iterator it ) const {
  if ( valid_rs_ . bits ( *it ) ) return valid_rs_ . rank ( * it );
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

#endif
