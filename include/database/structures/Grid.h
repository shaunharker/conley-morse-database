// Grid.h
#ifndef CMDB_GRID_H
#define CMDB_GRID_H

#include <stdint.h>
#include <memory>
#include <vector>
#include <stack>
#include <deque>

#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/unordered_set.hpp>
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"

#include "database/structures/Tree.h"

#include "chomp/Rect.h"
#include "chomp/Prism.h"
#include "chomp/RelativePair.h"
#include "chomp/CubicalComplex.h"
#include "chomp/BitmapSubcomplex.h"

#include "database/structures/CompressedGrid.h"

// Declaration
class Grid {
public:
  typedef uint64_t GridElement;
  typedef boost::counting_iterator < GridElement > iterator;
  typedef iterator const_iterator;
  typedef uint64_t size_type;
  
  // Virtual Methods
  virtual Tree::iterator GridToTree ( Grid::iterator it ) const = 0;
  virtual Grid::iterator TreeToGrid ( Tree::iterator it ) const = 0;
  virtual const Tree & tree ( void ) const = 0;
  virtual Tree & tree ( void ) = 0;
  virtual void subdivide ( void ) = 0;
  virtual void adjoin( const Grid & other ) = 0;
  virtual Grid * subgrid ( const std::deque < GridElement > & grid_elements ) const = 0;
  virtual std::vector<GridElement> subset ( const Grid & other ) const;
  virtual void rebuild ( void ) = 0;

  // Test and Debug
  virtual uint64_t memory ( void ) const = 0;
  
  // Construction methods
  template < class InputIterator >
  static CompressedGrid * join ( InputIterator start, InputIterator stop );
  void assign ( const CompressedGrid & compressed );
  
protected:
  Grid ( void );

public:
  void initialize ( const chomp::Rect & outer_bounds_of_grid );
  void initialize ( const chomp::Rect & outer_bounds_of_grid , const std::vector < bool > & periodic );
  virtual ~Grid ( void );

  // Container methods
  iterator find ( GridElement ge ) const;
  iterator begin ( void ) const;
  iterator end ( void ) const;
  size_type size ( void ) const;
  
  // Accessor methods
  const chomp::Rect & bounds ( void ) const;
  chomp::Rect & bounds ( void );
  int dimension ( void ) const;
  int & dimension ( void );
  const std::vector < bool > & periodicity ( void ) const;
  std::vector < bool > & periodicity ( void );

  
  // Geometric Methods
  chomp::Rect geometry ( const iterator & it ) const;
  chomp::Rect geometry ( GridElement ge ) const;
  
  template < class InsertIterator > InsertIterator
  cover ( InsertIterator & ii, const chomp::Rect & geometric_region ) const;
  
  template < class InsertIterator > InsertIterator
  cover ( InsertIterator & ii, const chomp::Prism & prism ) const;
  
  template < class InsertIterator, class T >
  inline void cover ( InsertIterator & ii, const std::vector < T > & V ) const;
  
  template < class InsertIterator, class S, class T >
  void cover ( InsertIterator & ii, const std::pair < S, T > & V ) const;
  
  // Interface to CHOMP
  

  size_type depth ( GridElement ge ) const;
  
  template < class Container >
  size_type getDepth ( const Container & cont ) const;
  
  void GridElementToCubes (std::vector<std::vector < uint32_t > > * cubes ,
                           const GridElement ge,
                           int depth ) const;
  
  
  template < class Container > void
  relativeComplex ( chomp::RelativePair * pair ,
                   const Container & XGridElements ,
                   const Container & AGridElements ,
                   int depth ) const;
  
protected:
  size_type size_;
  chomp::Rect bounds_;
  int dimension_;
  std::vector < bool > periodic_;
  
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & size_;
    ar & bounds_;
    ar & dimension_;
    ar & periodic_;
  }
  
};

// DEFINITIONS

inline std::vector<Grid::GridElement> Grid::subset ( const Grid & other ) const {
  // Walk through other . tree () and tree () simultaneously, recording grid elements
  // If "other" goes deeper than "this", we do not mind.
  // If "this" goes deeper than "other", we collect all decendant leaves.
  std::vector<GridElement> result;
  std::stack < std::pair < Tree::iterator, Tree::iterator > > work_stack;
  work_stack . push ( std::make_pair ( tree () . begin (), other . tree () . begin () ) );
  while ( not work_stack . empty () ) {
    Tree::iterator this_it = work_stack . top () . first;
    Tree::iterator other_it = work_stack . top () . second;
    work_stack . pop ();
    if ( tree () . isleaf ( this_it ) ) {
      // Leaf on "this"
      result . push_back ( * TreeToGrid ( this_it ) );
      continue;
    }
    // Not leaf on "this"
    if ( other . tree () . isleaf ( other_it ) ) {
      // Leaf on "other" -- filter down and get all subtree leaves on "this"
      Tree::iterator left_this_it = tree () . left ( this_it );
      Tree::iterator right_this_it = tree () . right ( this_it );
      if ( left_this_it != tree () . end () ) {
        work_stack . push ( std::make_pair ( left_this_it, other_it ) );
      }
      if ( right_this_it != tree () . end () ) {
        work_stack . push ( std::make_pair ( right_this_it, other_it ) );
      }
    } else {
      // Not leaf on "other" -- follow branches both "this" and "other" share
      // Follow left branch if it exists:
      Tree::iterator left_this_it = tree () . left ( this_it );
      Tree::iterator left_other_it = other . tree () . left ( other_it );
      if ( (left_this_it != tree () . end ()) && (left_other_it != other . tree () . end ()) ) {
        work_stack . push ( std::make_pair ( left_this_it, left_other_it ) );
      }
      // Follow right branch if it exists:
      Tree::iterator right_this_it = tree () . right ( this_it );
      Tree::iterator right_other_it = other . tree () . right ( other_it );
      if ( (right_this_it != tree () . end ()) && (right_other_it != other . tree () . end ()) ) {
        work_stack . push ( std::make_pair ( right_this_it, right_other_it ) );
      }
      
    }
  }
  return result;
}

// Construction/Initialization Methods

template < class InputIterator >
CompressedGrid * Grid::join ( InputIterator start, InputIterator stop ) {
  CompressedGrid * result = new CompressedGrid;
  if ( start == stop ) return result;
  result -> bounds_ = (*start) -> bounds_;
  result -> dimension_ = (*start) -> dimension_;
  result -> periodic_ = (*start) -> periodic_;
  // TODO: the next couple of line are inelegant. (Will auto help?)
  std::vector < Tree * > tree_pointers;
  for ( InputIterator it = start; it != stop; ++ it ) tree_pointers . push_back ( &((*it)->tree()) );
  result -> tree_ . reset ( Tree::join ( tree_pointers . begin (), tree_pointers . end () ) );
  result -> size_ = 0;
  size_t valid_size = result -> tree_ -> valid_tree_nodes . size ();
  for ( uint64_t i = 0; i < valid_size; ++ i )
    if ( result -> tree_ -> valid_tree_nodes [ i ] )
      ++ result -> size_;
  
  return result;
}

inline void Grid::assign ( const CompressedGrid & compressed ) {
  size_ = compressed . size_;
  bounds_ = compressed . bounds_;
  dimension_ = compressed . dimension_;
  periodic_ = compressed . periodic_;
  tree () . assign ( * compressed . tree_ );
  rebuild ();
}

inline Grid::Grid ( void ) {
  size_ = 1;
}

inline void Grid::initialize ( const chomp::Rect & outer_bounds_of_grid ) {
  bounds_ = outer_bounds_of_grid;
  dimension_ = outer_bounds_of_grid . lower_bounds . size ();
  periodic_ . resize ( dimension_, false );
} /* Grid::initialize */

inline void Grid::initialize ( const chomp::Rect & outer_bounds_of_toplex,
                              const std::vector < bool > & periodic ) {
  initialize ( outer_bounds_of_toplex );
  periodic_ = periodic;
}

inline Grid::~Grid ( void ) {
}

// Container Methods

inline Grid::iterator Grid::find ( GridElement find_me ) const {
  return iterator ( find_me );
} /* Grid::find */

inline Grid::iterator Grid::begin ( void ) const {
  return iterator ( 0 );
} /* Grid::begin */

inline Grid::iterator Grid::end ( void ) const {
  return iterator ( size () );
} /* Grid::end */

inline Grid::size_type Grid::size ( void ) const {
  return size_;
} /* Grid::size */

inline const chomp::Rect & Grid::bounds ( void ) const {
  return bounds_;
} /* Grid::bounds */

inline chomp::Rect & Grid::bounds ( void )  {
  return bounds_;
} /* Grid::bounds */

inline int Grid::dimension ( void ) const {
  return dimension_;
} /* Grid::dimension */

inline int & Grid::dimension ( void ) {
  return dimension_;
} /* Grid::dimension */

inline const std::vector < bool > & Grid::periodicity ( void ) const {
  return periodic_;
} /* Grid::periodicity */

inline  std::vector < bool > & Grid::periodicity ( void )  {
  return periodic_;
} /* Grid::periodicity */


inline chomp::Rect Grid::geometry ( const const_iterator & cell_iterator ) const {
  using namespace chomp;
  chomp::Rect return_value ( dimension_, Real ( 0 ) );
  //std::cout << "Grid::geometry ( " << * cell_iterator << ")\n";
  /* Climb the tree */
  Tree::iterator root = tree () . begin ();
  Tree::iterator it = GridToTree ( cell_iterator );
  int division_dimension = tree () . depth ( it ) % dimension ();
  while ( it != root ) {
    //std::cout << "visiting " << *it << " with parent " <<  * tree().parent(it) << "\n";
    std::cout . flush ();
    Tree::iterator parent = tree () . parent ( it );
    -- division_dimension; if ( division_dimension < 0 ) division_dimension = dimension () - 1;
    if ( tree () . left ( parent ) == it ) {
      /* This is a left-child */
      return_value . upper_bounds [ division_dimension ] += Real ( 1 );
    } else {
      /* This is a right-child */
      return_value . lower_bounds [ division_dimension ] += Real ( 1 );
    } /* if-else */
    return_value . lower_bounds [ division_dimension ] /= Real ( 2 );
    return_value . upper_bounds [ division_dimension ] /= Real ( 2 );
    it = parent;
  } /* while */
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    //std::cout << "dimension_index =  " << dimension_index << "\n";
    /* Produce convex combinations */
    return_value . lower_bounds [ dimension_index ] = return_value . lower_bounds [ dimension_index ] * bounds_ . upper_bounds [ dimension_index ] +
    ( Real ( 1 ) - return_value . lower_bounds [ dimension_index ] ) * bounds_ . lower_bounds [ dimension_index ];
    return_value . upper_bounds [ dimension_index ] = return_value . upper_bounds [ dimension_index ] * bounds_ . lower_bounds [ dimension_index ] +
    ( Real ( 1 ) - return_value . upper_bounds [ dimension_index ] ) * bounds_ . upper_bounds [ dimension_index ];
    //DEBUG
    if ( return_value . lower_bounds [ dimension_index ] > return_value . lower_bounds [ dimension_index ] ) {
      std::cout << "Grid::geometry ERROR: constructed invalid region.\n";
      exit(1);
    }
  } /* for */
  //std::cout << "returning from Grid::geometry with " << return_value << ".\n";
  return return_value;
} /* Grid::geometry */

inline chomp::Rect Grid::geometry ( GridElement ge  ) const {
  return geometry ( find ( ge ) );
} /* Grid::geometry */


// TODO: combine these cover routines into one piece of code to ease maintenance

template < class InsertIterator >
inline InsertIterator
Grid::cover ( InsertIterator & ii, const chomp::Rect & geometric_region ) const {
  
  using namespace chomp;
  //std::cout << "chomp::Rect version of Cover\n";
  //std::cout << "Covering " << geometric_region << "\n";
  //std::cout << "cover tree debug ---------\n";
  //tree () . debug ();
  // Deal with periodicity
  
  boost::unordered_set < GridElement > redundancy_check;
  
  std::vector < double > width ( dimension_ );
  for ( int d = 0; d < dimension_; ++ d ) {
    width [ d ] = bounds_ . upper_bounds [ d ] - bounds_ . lower_bounds [ d ];
  }
  
  std::stack < chomp::Rect > work_stack;
  chomp::Rect R = geometric_region;
  for ( int d = 0; d < dimension_; ++ d ) {
    if ( periodic_ [ d ] == false ) continue;
    if ( R . upper_bounds [ d ] > bounds_ . upper_bounds [ d ] ) {
      R . lower_bounds [ d ] -= width [ d ];
      R . upper_bounds [ d ] -= width [ d ];
    }
    if ( R . upper_bounds [d] - R . lower_bounds [ d ] > width [ d ] )
      R . upper_bounds [ d ] = R . lower_bounds [ d ] + width [ d ];
  }
  
  long periodic_long = 0;
  for ( int d = 0; d < dimension_; ++ d ) {
    if ( periodic_ [ d ] ) periodic_long += (1 << d );
  }
  
  // loop through all 2^l periodic images, avoiding repeats
  std::set < long > periodic_images;
  long hypercube = 2 << dimension_;
  for ( long k = 0; k < hypercube; ++ k ) {
    if ( periodic_images . count ( k & periodic_long ) ) continue;
    periodic_images . insert ( k & periodic_long );
    chomp::Rect r = R;
    for ( int d = 0; d < dimension_; ++ d ) {
      if ( periodic_ [ d ] == false ) continue;
      if ( k & ( 1 << d ) ) {
        r . lower_bounds [ d ] += width [ d ];
        r . upper_bounds [ d ] += width [ d ];
      }
    }
    work_stack . push ( r );
    //std::cout << "Pushed " << r << "\n";
  }
  
  //std::cout << "ready to cover pushed things\n";
  /* Use a stack, not a queue, and do depth first search.
   The advantage of this is that we can maintain the geometry during our Euler Tour.
   We can maintain our geometry without any roundoff error if we use the standard box
   [0,1]^d. To avoid having to translate to real coordinates at each leaf, we instead
   convert the input to these standard coordinates, which we put into integers. */
  
  while ( not work_stack . empty () ) {
    //std::cout << "Top of cover loop. Size of work stack = " << work_stack . size () << "\n";
    chomp::Rect GR = work_stack . top ();
    work_stack . pop ();
    //std::cout << "Trying to cover " << GR << "\n";
    // Step 1. Convert input to standard coordinates.
    chomp::Rect region ( dimension_ );
    static std::vector<uint64_t> LB ( dimension_);
    static std::vector<uint64_t> UB ( dimension_);
#define INTPHASEWIDTH (((uint64_t)1) << 60)
    static Real bignum ( INTPHASEWIDTH );
    bool out_of_bounds = false;
    for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
      region . lower_bounds [ dimension_index ] =
      (GR . lower_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
      (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
      region . upper_bounds [ dimension_index ] =
      (GR . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
      (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
      
      if (region . upper_bounds [ dimension_index ] < Real ( 0 ) ||
          region . lower_bounds [ dimension_index ] > Real ( 1 ) )  {
        out_of_bounds = true;
        break;
      }
      //std::cout << "dim " << dimension_index << ": [" << region . lower_bounds [ dimension_index ] << ", " << region . upper_bounds [ dimension_index ] << "]\n";
      if ( region . lower_bounds [ dimension_index ] < Real ( 0 ) )
        region . lower_bounds [ dimension_index ] = Real ( 0 );
      if ( region . lower_bounds [ dimension_index ] > Real ( 1 ) )
        region . lower_bounds [ dimension_index ] = Real ( 1 );
      LB [ dimension_index ] = (uint64_t) ( bignum * region . lower_bounds [ dimension_index ] );
      if ( region . upper_bounds [ dimension_index ] < Real ( 0 ) )
        region . upper_bounds [ dimension_index ] = Real ( 0 );
      if ( region . upper_bounds [ dimension_index ] > Real ( 1 ) )
        region . upper_bounds [ dimension_index ] = Real ( 1 );
      UB [ dimension_index ] = (uint64_t) ( bignum * region . upper_bounds [ dimension_index ] );
    }
    if ( out_of_bounds ) continue;
    // Step 2. Perform DFS on the Grid tree, recursing whenever we have intersection,
    //         (or adding leaf to output when we have leaf intersection)
    static std::vector<uint64_t> NLB ( dimension_);
    static std::vector<uint64_t> NUB ( dimension_);
    for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
      //if ( LB [ dimension_index ] < (1 << 20) ) LB [ dimension_index ] = 0;
      //if ( LB [ dimension_index ] >= (1 << 20) ) LB [ dimension_index ] -= (1 << 20);
      //if ( UB [ dimension_index ] < (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] += (1 << 20);
      //if ( UB [ dimension_index ] >= (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] = INTPHASEWIDTH;
      NLB [ dimension_index ] = 0;
      NUB [ dimension_index ] = INTPHASEWIDTH;
    }
    //std::cout << "C\n";
    
    /* Strategy.
     We will take the Euler Tour using a 4-state machine.
     There are Four states.
     0 = Just Descended. Check for an intersection.
     1 = Descend to the left
     2 = Descend to right
     3 = Rise.
     */
    
    Tree::iterator root = tree () . begin ();
    Tree::iterator N = root;
    Tree::iterator end = tree () . end ();
    
    char state = 0;
    int depth = -1;
    //std::cout << "D\n";
    //std::cout << "end = " << * tree () . end ();
    while ( 1 ) {
      //std::cout << "Entering Loop, state = " << (int) state << "\n";
      //std::cout << " N = " << *N << "\n";
      if ( state == 0 ) {
        ++ depth;
        //std::cout << "State " << (int)state << "\n";
        //std::cout << "Depth = " << depth << " \n";
        //std::cout << " N = " << *N << "\n";
        // If we have descended here, then we should check for intersection.
        bool intersect_flag = true;
        for ( int d = 0; d < dimension_; ++ d ) {
          if ( LB[d] > NUB[d] || UB[d] < NLB [d] ) {  // INTERSECTION CHECK
            intersect_flag = false;
            break;
          }
        }
        
        if ( intersect_flag ) {
          //std::cout << "Detected intersection.\n";
          // Check if its a leaf.
          if ( tree () . left ( N ) == end ) { 
            if ( tree () . right ( N ) == end ) {
              // Here's what we are looking for.
              GridElement ge = * TreeToGrid ( N );
              if ( redundancy_check . count ( ge ) == 0 ) {
                * ii ++ = ge; // OUTPUT
                //std::cout << "output " << ge << "\n";
                redundancy_check . insert ( ge );
              }
              //std::cout << "cover -- " << ge << "\n";
              // Issue the order to rise.
              //std::cout << "Issue rise.\n";
              state = 3;
            } else {
              // Issue the order to descend to the right.
              //std::cout << "Issue descend right.\n";
              state = 2;
            }
          } else {
            // Issue the order to descend to the left.
            //std::cout << "Issue descend left.\n";
            state = 1;
          }
        } else {
          // No intersection, issue order to rise.
          //std::cout << "No intersection. \n";
          //std::cout << "Issue Rise.\n";
          state = 3;
        } // intersection check complete
      } // state 0
      
      if ( state == 1 ) {
        // We have been ordered to descend to the left.
        //std::cout << "State " << (int)state << "\n";
        //std::cout << "Depth = " << depth << " \n";
        //std::cout << " N = " << *N << "\n";
        //std::cout << "Descend left.\n";
        int div_dim = depth % dimension ();
        NUB[div_dim] -= ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
        N = tree () . left ( N ) ;
        state = 0;
        continue;
      } // state 1
      
      if ( state == 2 ) {
        // We have been ordered to descend to the right.
        //std::cout << "State " << (int)state << "\n";
        //std::cout << "Depth = " << depth << " \n";
        //std::cout << " N = " << *N << "\n";
        //std::cout << "Descend right.\n";
        int div_dim = depth % dimension ();
        NLB[div_dim] += ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
        N = tree () . right ( N ) ;
        state = 0;
        continue;
      } // state 2
      
      if ( state == 3 ) {
        // We have been ordered to rise.
        //std::cout << "State " << (int)state << "\n";
        //std::cout << "Depth = " << depth << " \n";
        //std::cout << " N = " << *N << "\n";
        //std::cout << "Rise.\n";
        Tree::iterator P = tree () . parent ( N );
        // Can't rise if root.
        if ( P == end ) break; // algorithm complete
        -- depth;
        int div_dim = depth % dimension ();
        if ( tree () . left ( P )  == N ) {
          // This is a left child.
          //std::cout << "We are rising from left.\n";
          NUB[div_dim] += NUB[div_dim]-NLB[div_dim];
          // If we rise from the left child, we order parent to go right.
          // Unless there is no right child.
          if ( tree () . right ( P ) == end ) state = 3;
          else state = 2;
        } else {
          // This is the right child.
          //std::cout << "We are rising from right.\n";
          NLB[div_dim] -= NUB[div_dim]-NLB[div_dim];
          // If we rise from the right child, we order parent to rise.
          state = 3;
        }
        N = P;
      } // state 3
      
    } // while loop
  }
  return ii;
} // cover

template < class InsertIterator > inline InsertIterator
Grid::cover ( InsertIterator & ii, const chomp::Prism & prism ) const {
  using namespace chomp;
  //std::cout << "chomp::Prism version of Cover\n";
  static chomp::Rect G ( dimension_ );
  
  /* Use a stack, not a queue, and do depth first search.
   The advantage of this is that we can maintain the geometry during our Euler Tour.
   We can maintain our geometry without any roundoff error if we use the standard box
   [0,1]^d. To avoid having to translate to real coordinates at each leaf, we instead
   convert the input to these standard coordinates, which we put into integers. */
  
  // Step 1. Convert input to standard coordinates.
  
#define INTPHASEWIDTH (((uint64_t)1) << 60)
  
  // Step 2. Perform DFS on the Grid tree, recursing whenever we have intersection,
  //         (or adding leaf to output when we have leaf intersection)
  static std::vector<uint64_t> NLB ( dimension_);
  static std::vector<uint64_t> NUB ( dimension_);
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    NLB [ dimension_index ] = 0;
    NUB [ dimension_index ] = INTPHASEWIDTH;
  }
  //std::cout << "Cover\n";
  
  /* Strategy.
   We will take the Euler Tour using a 4-state machine.
   There are Four states.
   0 = Just Descended. Check for an intersection.
   1 = Descend to the left
   2 = Descend to right
   3 = Rise.
   */
  
  Tree::iterator N = tree () . begin ();
  Tree::iterator end = tree () . end ();
  char state = 0;
  int depth = -1;
  //std::cout << "Above main loop.\n";
  
  while ( 1 ) {
    //std::cout << "Entering Loop, state = " << (int) state << "\n";
    //std::cout << " N = " << N << "\n";
    if ( state == 0 ) {
      // If we have descended here, then we should check for intersection.
      ++ depth;
      // INTERSECTION CHECK
      for ( int d = 0; d < dimension_; ++ d ) {
        G . lower_bounds [ d ] = bounds () . lower_bounds [ d ] +
        (bounds () . upper_bounds [ d ] - bounds () . lower_bounds [ d ] ) * ( (Real) NLB [ d ] / (Real) INTPHASEWIDTH );
        G . upper_bounds [ d ] = bounds () . lower_bounds [ d ] +
        (bounds () . upper_bounds [ d ] - bounds () . lower_bounds [ d ] ) * ( (Real) NUB [ d ] / (Real) INTPHASEWIDTH );
      }
      
      //std::cout << "checking intersection:\n";
      if ( prism . intersects ( G ) ) {
        //std::cout << "Detected intersection.\n";
        // Check if its a leaf.
        if ( tree () . left ( N ) == end ) {
          if ( tree () . right ( N ) == end ) {
            // Here's what we are looking for.
            GridElement ge = * TreeToGrid ( N );
            * ii ++ = ge; // OUTPUT
            //std::cout << "cover -- " << *N << "\n";
            // Issue the order to rise.
            //std::cout << "Issue rise.\n";
            state = 3;
          } else {
            // Issue the order to descend to the right.
            //std::cout << "Issue descend right.\n";
            state = 2;
          }
        } else {
          // Issue the order to descend to the left.
          //std::cout << "Issue descend left.\n";
          state = 1;
        }
      } else {
        // No intersection, issue order to rise.
        //std::cout << "No intersection. \n";
        //std::cout << "Issue Rise.\n";
        state = 3;
      } // intersection check complete
    } // state 0
    
    if ( state == 1 ) {
      // We have been ordered to descend to the left.
      //std::cout << "Descend left.\n";
      int div_dim = depth % dimension ();
      NUB[div_dim] -= ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = tree () . left ( N ) ;
      state = 0;
      continue;
    } // state 1
    
    if ( state == 2 ) {
      // We have been ordered to descend to the right.
      //std::cout << "Descend right.\n";
      int div_dim = depth % dimension ();
      NLB[div_dim] += ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = tree () . right ( N ) ;
      state = 0;
      continue;
    } // state 2
    
    if ( state == 3 ) {
      // We have been ordered to rise.
      //std::cout << "Rise.\n";
      Tree::iterator P = tree () . parent ( N );

      // Can't rise if root.
      if ( P == end ) break; // algorithm complete
      -- depth;
      int div_dim = depth % dimension ();
      if ( tree () . left ( P ) == N ) {
        // This is a left child.
        //std::cout << "We are rising from left.\n";
        NUB[div_dim] += NUB[div_dim]-NLB[div_dim];
        // If we rise from the left child, we order parent to go right.
        if ( tree () . right ( P ) == end ) state = 3;
        else state = 2;
      } else {
        // This is the right child.
        //std::cout << "We are rising from right.\n";
        NLB[div_dim] -= NUB[div_dim]-NLB[div_dim];
        // If we rise from the right child, we order parent to rise.
        state = 3;
      }
      N = P;
    } // state 3
    
  } // while loop
  return ii;
} // cover


// UNION version of cover
template < class InsertIterator, class T >
inline void Grid::cover ( InsertIterator & ii, const std::vector < T > & V ) const {
  BOOST_FOREACH ( const T & geo, V ) {
    cover ( ii, geo );
  }
}

// INTERSECTION (pair) for cover
template < class InsertIterator, class S, class T >
inline void Grid::cover ( InsertIterator & ii, const std::pair < S, T > & V ) const {
  // Cover V . first, store in "firstcover"
  boost::unordered_set < GridElement > firstcover;
  std::insert_iterator < boost::unordered_set < GridElement > > fcii ( firstcover, firstcover . begin () );
  cover ( fcii, V . first );
  // Cover V . second, store in "secondcover"
  boost::unordered_set < GridElement > secondcover;
  std::insert_iterator < boost::unordered_set < GridElement > > scii ( secondcover, secondcover . begin () );
  cover ( scii, V . second );
  // Without loss, let firstcover be no smaller than secondcover.
  if ( firstcover . size () < secondcover. size () ) std::swap ( firstcover, secondcover );
  // Compute intersection by checking if each element in smaller cover is in larger cover
  BOOST_FOREACH ( const GridElement & ge, secondcover ) {
    if ( firstcover . count ( ge ) ) {
      // Use "ii" to insert "ge" into output.
      * ii ++ = ge;
    }
  }
}

// INTERFACE TO CHOMP

inline Grid::size_type Grid::depth ( GridElement ge ) const {
  Tree::iterator it = GridToTree ( iterator ( ge ) );
  size_type result = 0;
  while ( it != tree () . begin () ) {
    it = tree () . parent ( it );
    ++ result;
  }
  return result;
}

template < class Container >
Grid::size_type Grid::getDepth ( const Container & cont ) const {
  // TODO inefficient, intersecting tree climbs
  size_type result = 0;
  BOOST_FOREACH ( GridElement ge, cont ) {
    size_type d = depth ( ge );
    if ( d > result ) result = d;
  }
  return result;
}

inline void Grid::GridElementToCubes ( std::vector<std::vector < uint32_t > > * cubes,
                                             const GridElement ge, int depth ) const {
  // Obtain the prefix
  //std::cout << "GEtoCubes: " << geometry ( ge ) << "\n";
  int D = dimension ();
  
  // Determine width
  typedef std::vector < uint32_t > Cube;
  Cube cube ( D, 0 );
  
  int dim = 0;
  Tree::iterator root = tree () . begin ();
  Tree::iterator it = GridToTree ( find ( ge ) );
  std::vector < unsigned char > p;
  while ( it != root ) {
    if ( tree () . isleft ( it ) ) p . push_back ( 0 );
    else p . push_back ( 1 );
    it = tree () . parent ( it );
  }
  int GridElement_depth = p . size (); // == getDepth ( ge );
  //std::cout << "  gedepth = " << GridElement_depth << ", from " << p . size () << "\n";
  if ( GridElement_depth > depth ) GridElement_depth = depth; //effectively truncates the prefix
  
  for ( int d = 0; d < GridElement_depth; ++ d ) {
    if ( dim == D ) dim = 0;
    cube [ dim ] <<= 1;
    cube [ dim ] |= (uint32_t) p [ GridElement_depth - d - 1 ];
    ++ dim;
  }
  // make the cubes
  if ( GridElement_depth == depth ) {
    cubes -> push_back ( cube );
    //std::cout << "    simple case.\n";
    return;
  }
  
  // We must make more than one output cube;
  // the user has requested a greater depth than
  // the toplex provides.
  //std::cout << "    hard case.\n";
  
  std::vector < Cube > work_stack, split_stack;
  work_stack . push_back ( cube );
  for ( int dim = GridElement_depth; dim < depth; ++ dim ) {
    std::vector < Cube > split_stack;
    BOOST_FOREACH ( Cube c, work_stack ) {
      c [ dim % D ] <<= 1;
      split_stack . push_back ( c );
      c [ dim % D ] |= 1;
      split_stack . push_back ( c );
    }
    std::swap ( work_stack, split_stack );
  }
  BOOST_FOREACH ( const Cube & outcube, work_stack ) {
    cubes -> push_back ( outcube );
  }
  
}

template < class Container >
inline void Grid::relativeComplex ( chomp::RelativePair * pair,
                                    const Container & XGridElements,
                                    const Container & AGridElements,
                                    int depth ) const {
  using namespace chomp;
  // DEBUG
  // check A \subset X
  /*
   boost::unordered_set < GridElement > XS, AS;
   BOOST_FOREACH ( GridElement x, XGridElements ) XS . insert ( x );
   BOOST_FOREACH ( GridElement a, AGridElements ) {
   AS . insert ( a );
   if ( XS . count ( a ) == 0 ) {
   std::cout << "couldn't find " << a << "\n";
   exit ( 1 );
   }
   }
   */
  
  // Produce the full complex.
  CubicalComplex * full_complex = new CubicalComplex;
  CubicalComplex & X = *full_complex;
  int D = dimension ();
  
  //std::cout << "relativeComplex.\n";
  //std::cout << "XGridElements:\n";
  
  // Make set of cubes, and learn bounds of the cubes.
  typedef std::vector < uint32_t > Cube;
  Cube mincube ( D, -1 );
  Cube maxcube ( D, 0 );
  chomp::Rect newbounds ( D );
  for ( int d = 0; d < D; ++ d ) {
		newbounds . lower_bounds [ d ] = bounds () . upper_bounds [ d ];
		newbounds . upper_bounds [ d ] = bounds () . lower_bounds [ d ];
  }
  BOOST_FOREACH ( GridElement e, XGridElements ) {
    chomp::Rect geo = geometry ( e );
    for ( int d = 0; d < D; ++ d ) {
      if ( newbounds . lower_bounds [ d ] > geo . lower_bounds [ d ] )
        newbounds . lower_bounds [ d ] = geo . lower_bounds [ d ];
      if ( newbounds . upper_bounds [ d ] < geo . upper_bounds [ d ] )
        newbounds . upper_bounds [ d ] = geo . upper_bounds [ d ];
    }
    std::vector < Cube > cubes;
    GridElementToCubes ( &cubes, e, depth );
    BOOST_FOREACH ( Cube & cube, cubes ) {
      for ( int d = 0; d < D; ++ d ) {
      	if ( mincube [ d ] > cube [ d ] ) mincube [ d ] = cube [ d ];
      	if ( maxcube [ d ] < cube [ d ] ) maxcube [ d ] = cube [ d ];
      }
    }
  }
  
  std::vector < uint32_t > dimension_sizes ( D, 1 );
  std::vector < bool > is_periodic = periodic_;
  for ( int d = 0; d < D; ++ d ) {
    dimension_sizes [ d ] = maxcube [ d ] - mincube [ d ] + 1;
    if ( newbounds . lower_bounds [ d ] > bounds () . lower_bounds [ d ] )
    	is_periodic [ d ] = false;
    if ( newbounds . upper_bounds [ d ] < bounds () . upper_bounds [ d ] )
    	is_periodic [ d ] = false;
  }
  
  X . bounds () = newbounds;
  X . initialize ( dimension_sizes, is_periodic );
  
  BOOST_FOREACH ( GridElement e, XGridElements ) {
    //std::cout << e << "\n";
    //std::cout << "GEOMETRY = " << geometry ( e ) << "\n";
    typedef std::vector < uint32_t > Cube;
    std::vector < Cube > cubes;
    GridElementToCubes ( &cubes, e, depth );
    BOOST_FOREACH ( Cube & cube, cubes ) {
      //std::cout << "XCube = ";
      //for ( int d = 0; d < dimension (); ++ d ) std::cout << cube [ d ] << " ";
      //std::cout << "\n";
      Cube offset ( D );
      for ( int d = 0; d < D; ++ d ) offset [ d ] = cube [ d ] - mincube [ d ];
      X . addFullCube ( offset );
      //std::cout << " CC-Geometry = " << X . geometryOfCube ( cube ) << "\n";
    }
  }
  X . finalize ();
  
  // Produce the relative complex
  BitmapSubcomplex * pair_complex = new BitmapSubcomplex ( X, true );
  BitmapSubcomplex * rel_complex = new BitmapSubcomplex ( X, false );
  BitmapSubcomplex & XA = * pair_complex;
  BitmapSubcomplex & A = * rel_complex;
  //std::cout << "AGridElements:\n";
  BOOST_FOREACH ( GridElement e, AGridElements ) {
    //std::cout << e << "\n";
    typedef std::vector < uint32_t > Cube;
    std::vector < Cube > cubes;
    GridElementToCubes ( &cubes, e, depth );
    BOOST_FOREACH ( Cube & cube, cubes ) {
      //std::cout << "ACube = ";
      //for ( int d = 0; d < dimension (); ++ d ) std::cout << cube [ d ] << " ";
      //std::cout << "\n";
      Cube offset ( D );
      for ( int d = 0; d < D; ++ d ) offset [ d ] = cube [ d ] - mincube [ d ];
      
      std::vector < std::vector < Index > > cells =
      X . fullCubeIndexes ( offset );
      for ( int d = 0; d <= dimension (); ++ d ) {
        BOOST_FOREACH ( Index cell, cells [ d ] ) {
          XA . erase ( cell, d );
          A . insert ( cell, d );
        }
      }
    }
  }
  XA . finalize ();
  A . finalize ();
  pair -> initialize ( &X, &XA, &A ); 
}

#endif
