#ifndef CHOMP_SUCCINCTGRID_H
#define CHOMP_SUCCINCTGRID_H


#include <iostream>
#include <fstream>

#include <vector>
#include <stack>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "database/structures/Grid.h"
#include "database/structures/Tree.h"
#include "database/structures/SuccinctTree.h"



/**
 * @file
 * @author Arnaud Goullet, Shaun Harker
 * @version 1.0
 *
 * @description
 * Implementation of a grid class.
 * A phase space in d-dimension is being decomposed non uniformily into set of rectangular elements
 * or grid elements. The space decomposition or resulting grid is being viewed as a
 * binary tree with the leaves corresponding to the 'smallest' grid element.
 */



/*! \class SuccinctGrid */

// For a full tree with number of nodes > 100,000
// the memory usage is about
// 2.9 bits / node for the full binary tree
// 1.29 bits / nodes for the rank select of the node validity
// 1.18 bits / nodes for the rank select of the leaves
// Total ~ 5.37 bits / nodes

class SuccinctGrid : public Grid {
public:
  // Constructor/Deconstructor Methods
  SuccinctGrid ( void );
  ~SuccinctGrid ( void );
  
  // Virtual Methods
  virtual Tree::iterator GridToTree ( Grid::iterator it ) const;
  virtual Grid::iterator TreeToGrid ( Tree::iterator it ) const;
  virtual const SuccinctTree & tree ( void ) const;
  virtual SuccinctTree & tree ( void );
  virtual void subdivide ( void );
  virtual void adjoin( const Grid & other );
  virtual SuccinctGrid * subgrid ( const std::vector < GridElement > & grid_elements ) const;
  void rebuild ( void );

  // Features
  size_type depth ( const iterator & it ) const;
  template < class Container > int depth ( const Container & subset ) const;
  void info ( void ) const;
  void export_graphviz ( const char * filename ) const;
  void memory_usage ( void ) const;
  
private:
  // Data
  SuccinctTree tree_;
  RankSelect leaves_rs_;
  
  // Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & tree_;
    ar & leaves_rs_;
  }
  
};

inline SuccinctGrid::SuccinctGrid ( void ) {
  // Create the basic binary tree, with only one node
  std::vector < bool > leaves;
  leaves . push_back ( 1 );
  leaves_rs_ = RankSelect ( leaves );
  size_ = 1;
  
} /* SuccinctGrid::SuccinctGrid */


inline SuccinctGrid::~SuccinctGrid ( void ) {
}

inline Grid::iterator SuccinctGrid::TreeToGrid ( Tree::iterator it_tree ) const {
  if ( tree_ . isleaf ( it_tree ) ) {
    return leaves_rs_ . rank ( *it_tree ) - 1;
  } else {
    return size_;
  }
}

inline Tree::iterator SuccinctGrid::GridToTree ( iterator it ) const {
  return leaves_rs_ . select ( *it + 1 );
}

inline const SuccinctTree & SuccinctGrid::tree ( void ) const {
  return tree_;
}

inline SuccinctTree & SuccinctGrid::tree ( void ) {
  return tree_;
}

inline void SuccinctGrid::subdivide(void) {
  tree_ . subdivide();
  rebuild ();
}

inline void SuccinctGrid::adjoin( const Grid & other ) {
  tree_ . adjoin ( other . tree () );
  rebuild ();
}

inline SuccinctGrid * SuccinctGrid::subgrid ( const std::vector < Grid::GridElement > & grid_elements ) const {
  // First we must convert the grid_elements to leaves
  std::vector < Tree::iterator > leaves;
  BOOST_FOREACH ( GridElement ge, grid_elements ) {
    leaves . push_back ( GridToTree ( iterator ( ge ) ) );
  }
 
  // Now we build the subgrid
  SuccinctGrid * result = new SuccinctGrid;
  result -> tree_ =  * tree () . subtree ( leaves );
  result -> size_ = grid_elements . size ();
  result -> initialize ( bounds (), periodicity () );
  return result;
}


inline void SuccinctGrid::rebuild ( void ) {
  std::vector < bool > leaf_sequence;
  Tree::iterator it;
  Tree::iterator end_it = tree () . end ();
  size_ = 0;
  for ( it = tree () . begin ( ); it != end_it; ++ it ) {
    if ( tree () . isleaf ( it ) ) {
      leaf_sequence . push_back ( 1 );
      ++ size_;
    } else {
      leaf_sequence . push_back ( 0 );
    }
  }
  leaves_rs_ = RankSelect( leaf_sequence );
}

// Features

inline SuccinctGrid::size_type SuccinctGrid::depth ( const iterator & it ) const {
  return tree_ . depth ( GridToTree ( it ) );
}


template < class Container >
inline int SuccinctGrid::depth ( const Container & subset ) const {
  int thedepth = 0;
  BOOST_FOREACH ( const GridElement & ge , subset ) {
    int ge_depth = depth ( TreeToGrid ( ge ) );
    if ( ge_depth > thedepth ) thedepth = ge_depth;
  }
  return thedepth;
}

/**
 * Display information
 */
inline void SuccinctGrid::info ( void ) const {
  tree_ . info ( );
  std::cout << "\nLeaves ";
  leaves_rs_ . info ( );
  std::cout << "\nNumber of leaves " << size_ << "\n";
}

/**
 * Export the binary tree corresponding to the SuccinctGrid decomposition of the
 * phase space to a file "filename" in the GraphViz format .
 * @param filename
 */
inline void SuccinctGrid::export_graphviz ( const char * filename ) const {
  tree_ . export_graphviz ( filename );
}

/**
 * display the memory usage
 */
inline void SuccinctGrid::memory_usage ( void ) const {
  tree_ . memory_usage ( );
  leaves_rs_ . memory_usage ( );
}

#endif
