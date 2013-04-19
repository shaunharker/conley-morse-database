// PointerGrid.h
// Shaun Harker
// 9/16/11

#ifndef CMDP_POINTERGRID_H
#define CMDP_POINTERGRID_H

#include <vector>
#include <stack>
#include <boost/unordered_set.hpp>

#include "database/structures/Grid.h"
#include "database/structures/Tree.h"
#include "database/structures/PointerTree.h"

#include "boost/shared_ptr.hpp"

#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/export.hpp"
#include "boost/serialization/shared_ptr.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/***************
 * PointerGrid *
 ***************/

class PointerGrid : public Grid {
public:

  // Constructor/Deconstructor Methods
  PointerGrid ( void );
  ~PointerGrid ( void );
  
  // Virtual Methods
  virtual Tree::iterator GridToTree ( Grid::iterator it ) const;
  virtual Grid::iterator TreeToGrid ( Tree::iterator it ) const;
  virtual const PointerTree & tree ( void ) const;
  virtual PointerTree & tree ( void );
  virtual void subdivide ( void );
  virtual void adjoin( const Grid & other );
  virtual PointerGrid * subgrid ( const std::vector < GridElement > & grid_elements ) const;

private:
  
  // Data to translate between tree and grid iterators
  boost::shared_ptr<PointerTree> tree_;
  std::vector < Grid::iterator > grid_iterators_;
  std::vector < Tree::iterator > tree_iterators_;
  void rebuild ( void );
  
  friend class boost::serialization::access;
public:
  template<typename Archive>
  void serialize(Archive & ar, const unsigned int file_version) {
    ar & boost::serialization::base_object<Grid>(*this);
    ar & tree_;
    rebuild();
  }
  // file operations
  void save ( const char * filename ) const {
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::text_oarchive oa(ofs);
    oa << * this;
    ofs . close ();
  }
  
  void load ( const char * filename ) {
    std::ifstream ifs(filename);
    if ( not ifs . good () ) {
      std::cout << "Could not load " << filename << "\n";
      exit ( 1 );
    }
    boost::archive::text_iarchive ia(ifs);
    ia >> * this;    
    ifs . close ();
  }
 
  
};

BOOST_CLASS_EXPORT_KEY(PointerGrid);

// Definitions

inline PointerGrid::PointerGrid ( void ) : tree_ ( new PointerTree ) {
  rebuild ();
}

inline PointerGrid::~PointerGrid ( void ) {
}


// To convert iterators between Grid and BinaryTree

inline Grid::iterator PointerGrid::TreeToGrid ( Tree::iterator tree_it ) const {
  return grid_iterators_ [ * tree_it ];
}

inline Tree::iterator PointerGrid::GridToTree ( Grid::iterator grid_it ) const {
  //std::cout << "grid_it = " << * grid_it << "\n";
  //std::cout << "size of tree_iterators_ = " << tree_iterators_ . size () << "\n";
  return tree_iterators_ [ * grid_it ];
}

inline const PointerTree & PointerGrid::tree ( void ) const {
  return * tree_ . get ();
}

inline PointerTree & PointerGrid::tree ( void ) {
  return * tree_ . get ();
}

inline void PointerGrid::subdivide ( void ) {
  // First we subdivide the underlying tree
  //std::cout << "PointerGrid::subdivide, currently have " << size() << " grid elements.\n";
  tree () . subdivide ();
  rebuild ();
  //std::cout << "finished PointerGrid::subdivide, now have " << size() << " grid elements.\n";
}

inline void PointerGrid::adjoin( const Grid & other ) {
  tree_ -> adjoin ( other . tree () );
  rebuild ();
}


inline PointerGrid * PointerGrid::subgrid ( const std::vector < GridElement > & grid_elements ) const {
  // First we must convert the grid_elements to leaves
  std::vector < Tree::iterator > leaves;
  BOOST_FOREACH ( GridElement ge, grid_elements ) {
    leaves . push_back ( GridToTree ( iterator ( ge ) ) );
  }
  // Now we build the subtree
  boost::shared_ptr<PointerTree> subtree ( tree () . subtree ( leaves ) );
  
  // Now we build the subgrid
  PointerGrid * result = new PointerGrid;
  result -> tree_ = subtree; // Remark: This deallocates the initial "pointertree" object
  result -> size_ = grid_elements . size ();
  result -> initialize ( bounds (), periodicity () );
  result -> rebuild ();
  return result;
}

// Private methods for PointerGrid
inline
void PointerGrid::rebuild ( void ) {
  // Now we rebuild the GridIterator to TreeIterator conversions
  grid_iterators_ . clear ();
  tree_iterators_ . clear ();
  uint64_t leaf_count = 0;
  Tree::iterator end = tree () . end ();
  for ( Tree::iterator it = tree () . begin (); it != end; ++ it ) {
    if ( tree () . isleaf ( it ) ) {
      grid_iterators_ . push_back ( Grid::iterator (leaf_count ++ ) );
      tree_iterators_ . push_back ( it );
    } else {
      grid_iterators_ . push_back ( tree () . size () ); // doesn't matter
    }
  }
  size_ = leaf_count;
}

#endif
