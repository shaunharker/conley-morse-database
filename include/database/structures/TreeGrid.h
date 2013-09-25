#ifndef CMDB_TREEGRID_H
#define CMDB_TREEGRID_H

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

#include "database/structures/Grid.h"
#include "database/structures/Tree.h"

#include "database/structures/RectGeo.h"
#include "database/structures/PrismGeo.h"

// Chomp Interface
#include "chomp/Rect.h"
#include "chomp/RelativePair.h"
#include "chomp/CubicalComplex.h"
#include "chomp/BitmapSubcomplex.h"

#include "database/structures/CompressedGrid.h"

// Declaration
class TreeGrid : public Grid {
public:
  typedef uint64_t GridElement;
  typedef boost::counting_iterator < GridElement > iterator;
  typedef iterator const_iterator;
  typedef uint64_t size_type;
  
  // Virtual Methods inherited from Grid
  virtual TreeGrid * clone ( void ) const = 0;
  virtual void subdivide ( void ) = 0;
  virtual TreeGrid * subgrid ( const std::deque < GridElement > & grid_elements ) const = 0;
  virtual std::vector<GridElement> subset ( const Grid & other ) const;
  virtual boost::shared_ptr<Geo> geometry ( GridElement ge ) const;  
  using Grid::geometry;
  virtual std::vector<Grid::GridElement> cover ( const Geo & geo ) const;
  std::vector<GridElement> coverAccept ( const PrismGeo & visitor ) const;
  std::vector<GridElement> coverAccept ( const RectGeo & visitor ) const;
  using Grid::cover;
  // TreeGrid methods

  virtual Tree::iterator GridToTree ( TreeGrid::iterator it ) const = 0;
  virtual TreeGrid::iterator TreeToGrid ( Tree::iterator it ) const = 0;
  virtual const Tree & tree ( void ) const = 0;
  virtual Tree & tree ( void ) = 0;
  virtual void rebuild ( void ) = 0;
  void initialize ( const RectGeo & outer_bounds_of_grid );
  void initialize ( const RectGeo & outer_bounds_of_grid , const std::vector < bool > & periodic );
  const RectGeo & bounds ( void ) const;
  RectGeo & bounds ( void );
  int dimension ( void ) const;
  int & dimension ( void );
  const std::vector < bool > & periodicity ( void ) const;
  std::vector < bool > & periodicity ( void );

  // Test and Debug
  virtual uint64_t memory ( void ) const = 0;
  
  // Construction methods
  template < class InputIterator >
  static CompressedGrid * join ( InputIterator start, InputIterator stop );
  void assign ( const CompressedGrid & compressed ); // should this be virtual?
  
  // Constructor/Deconstructor
  protected:
  TreeGrid ( void );
  public:
  virtual ~TreeGrid ( void );

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
  //size_type size_;
  RectGeo bounds_;
  int dimension_;
  std::vector < bool > periodic_;
  
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<Grid>(*this);
    ar & bounds_;
    ar & dimension_;
    ar & periodic_;
  }
  
};

// DEFINITIONS


/////////////////////////////////////////////////
//               SUBSET                        //
/////////////////////////////////////////////////

inline std::vector<Grid::GridElement> TreeGrid::subset ( const Grid & other_in ) const {
  const TreeGrid & other = dynamic_cast<const TreeGrid &> (other_in);
  // Walk through other . tree () and tree () simultaneously, recording grid elements
  // If "other" goes deeper than "this", we do not mind.
  // If "this" goes deeper than "other", we collect all decendant leaves.
  std::vector<GridElement> result;
  std::stack < std::pair < Tree::iterator, Tree::iterator > > work_stack;
  //std::cout << "Grid::subset 1\n";
  work_stack . push ( std::make_pair ( tree () . begin (), other . tree () . begin () ) );
  //std::cout << "Grid::subset 2\n";

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
  //std::cout << "Grid::subset 3\n";
  return result;
}

/////////////////////////////////////////////////////////
//               TREEGRID ROUTINES                     //
/////////////////////////////////////////////////////////

inline void TreeGrid::initialize ( const RectGeo & outer_bounds_of_grid ) {
  bounds_ = outer_bounds_of_grid;
  dimension_ = outer_bounds_of_grid . lower_bounds . size ();
  periodic_ . resize ( dimension_, false );
} /* TreeGrid::initialize */

inline void TreeGrid::initialize ( const RectGeo & outer_bounds_of_toplex,
                              const std::vector < bool > & periodic ) {
  initialize ( outer_bounds_of_toplex );
  periodic_ = periodic;
}

inline const RectGeo & TreeGrid::bounds ( void ) const {
  return bounds_;
} /* TreeGrid::bounds */

inline RectGeo & TreeGrid::bounds ( void )  {
  return bounds_;
} /* TreeGrid::bounds */

inline int TreeGrid::dimension ( void ) const {
  return dimension_;
} /* TreeGrid::dimension */

inline int & TreeGrid::dimension ( void ) {
  return dimension_;
} /* TreeGrid::dimension */

inline const std::vector < bool > & TreeGrid::periodicity ( void ) const {
  return periodic_;
} /* TreeGrid::periodicity */

inline  std::vector < bool > & TreeGrid::periodicity ( void )  {
  return periodic_;
} /* TreeGrid::periodicity */


template < class InputIterator >
CompressedGrid * TreeGrid::join ( InputIterator start, InputIterator stop ) {
  CompressedGrid * result = new CompressedGrid;
  if ( start == stop ) return result;
  boost::shared_ptr<TreeGrid> start_ptr = boost::dynamic_pointer_cast<TreeGrid>(*start);
  if ( not start_ptr ) {
    std::cout << "TreeGrid::join error: not iterating over boost::shared_ptr<TreeGrid> container\n";
    abort ();
  }
  result -> bounds_ = start_ptr -> bounds_;
  result -> dimension_ = start_ptr -> dimension_;
  result -> periodic_ = start_ptr -> periodic_;

  std::vector < Tree * > tree_pointers;
  //int count = 0;
  for ( InputIterator it = start; it != stop; ++ it ) { 
    //std::cout << "Top. " << count++ << "\n";
    boost::shared_ptr<TreeGrid> it_ptr = boost::dynamic_pointer_cast<TreeGrid>(*it);
    if ( not it_ptr ) {
      std::cout << "TreeGrid::join error: not iterating over boost::shared_ptr<TreeGrid> container (B)\n";
      abort ();
    }
    tree_pointers . push_back ( &(it_ptr->tree()) );
  }

  result -> tree_ . reset ( Tree::join ( tree_pointers . begin (), tree_pointers . end () ) );
  result -> size_ = 0;
  size_t valid_size = result -> tree_ -> valid_tree_nodes . size ();
  for ( uint64_t i = 0; i < valid_size; ++ i )
    if ( result -> tree_ -> valid_tree_nodes [ i ] )
      ++ result -> size_;


  return result;
}

inline void TreeGrid::assign ( const CompressedGrid & compressed ) {
  size_ = compressed . size_;
  bounds_ = compressed . bounds_;
  dimension_ = compressed . dimension_;
  periodic_ = compressed . periodic_;
  tree () . assign ( * compressed . tree_ );
  rebuild ();
}

inline TreeGrid::TreeGrid ( void ) {
  size_ = 1;
}

inline TreeGrid::~TreeGrid ( void ) {
}

/////////////////////////////////////////////////
//               GEOMETRY                      //
/////////////////////////////////////////////////

inline boost::shared_ptr<Geo> TreeGrid::geometry ( GridElement ge ) const {
  //std::cout << "dimension_ = " << dimension_ << "\n";
  using namespace chomp;
  iterator cell_iterator ( ge ); 
  boost::shared_ptr<RectGeo> return_value ( new RectGeo ( dimension_, Real ( 0 ) ) );
  RectGeo & rect = * return_value ; 
  //std::cout << "Grid::geometry ( " << * cell_iterator << ")\n";
  /* Climb the tree */
  Tree::iterator root = tree () . begin ();
  Tree::iterator it = GridToTree ( cell_iterator );
  int division_dimension = tree () . depth ( it ) % dimension ();
  while ( it != root ) {
    //std::cout << "visiting " << *it << " with parent " <<  * tree().parent(it) << "\n";
    //std::cout . flush ();
    Tree::iterator parent = tree () . parent ( it );
    -- division_dimension; if ( division_dimension < 0 ) division_dimension = dimension () - 1;
    if ( tree () . left ( parent ) == it ) {
      /* This is a left-child */
      rect . upper_bounds [ division_dimension ] += Real ( 1 );
    } else {
      /* This is a right-child */
      rect . lower_bounds [ division_dimension ] += Real ( 1 );
    } /* if-else */
    rect . lower_bounds [ division_dimension ] /= Real ( 2 );
    rect . upper_bounds [ division_dimension ] /= Real ( 2 );
    it = parent;
  } /* while */
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    //std::cout << "dimension_index =  " << dimension_index << " out of " << dimension_ << "\n";
    //std::cout << "rect . lower_bounds . size () == " << rect . lower_bounds . size () << "\n";
    //std::cout << "bounds_ . lower_bounds . size () == " << bounds_ . lower_bounds . size () << "\n";

    /* Produce convex combinations */
    rect . lower_bounds [ dimension_index ] = rect . lower_bounds [ dimension_index ] * bounds_ . upper_bounds [ dimension_index ] +
    ( Real ( 1 ) - rect . lower_bounds [ dimension_index ] ) * bounds_ . lower_bounds [ dimension_index ];
    rect . upper_bounds [ dimension_index ] = rect . upper_bounds [ dimension_index ] * bounds_ . lower_bounds [ dimension_index ] +
    ( Real ( 1 ) - rect . upper_bounds [ dimension_index ] ) * bounds_ . upper_bounds [ dimension_index ];
    //DEBUG
    if ( rect . lower_bounds [ dimension_index ] > rect . lower_bounds [ dimension_index ] ) {
      std::cout << "Grid::geometry ERROR: constructed invalid region.\n";
      exit(1);
    }
  } /* for */
  //std::cout << "returning from TreeGrid::geometry with " << rect << ".\n";
  return return_value;
} /* TreeGrid::geometry */


/////////////////////////////////////////////////////////
//                  COVER ROUTINES                     //
/////////////////////////////////////////////////////////
inline std::vector<Grid::GridElement>
TreeGrid::cover ( const Geo & geo ) const {
  //std::cout << "TreeGrid::cover dispatching.\n";
  const Geo * geo_ptr = & geo;
  if ( const RectGeo * rect_geo = dynamic_cast < const RectGeo * > ( geo_ptr ) ) {
    return coverAccept ( * rect_geo );
  } else if ( const PrismGeo * prism_geo = dynamic_cast < const PrismGeo * > ( geo_ptr ) ) {
    return coverAccept ( * prism_geo );
  } 
  std::cout << "Bad Geo type in TreeGrid::cover\n";
  abort ();
  return std::vector<Grid::GridElement> (); // suppress warning
}

inline std::vector<Grid::GridElement>
TreeGrid::coverAccept ( const RectGeo & visitor ) const  {
  const RectGeo & geometric_region = visitor;
  std::vector<Grid::GridElement> results;
  using namespace chomp;
  //std::cout << "RectGeo version of Cover\n";
  //std::cout << "Covering " << geometric_region << "\n";
  //std::cout << "cover tree debug ---------\n";
  //tree () . debug ();
  // Deal with periodicity
  
  boost::unordered_set < GridElement > redundancy_check;
  
  std::vector < double > width ( dimension_ );
  for ( int d = 0; d < dimension_; ++ d ) {
    width [ d ] = bounds_ . upper_bounds [ d ] - bounds_ . lower_bounds [ d ];
  }
  
  std::stack < RectGeo > work_stack;
  RectGeo R = geometric_region;
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
    RectGeo r = R;
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
    RectGeo GR = work_stack . top ();
    work_stack . pop ();
    //std::cout << "Trying to cover " << GR << "\n";
    // Step 1. Convert input to standard coordinates.
    RectGeo region ( dimension_ );
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
                results . push_back ( ge ); // OUTPUT
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
  //std::cout << "Returning from cover.\n";
  return results;
} // cover

inline std::vector<Grid::GridElement>
TreeGrid::coverAccept ( const PrismGeo & visitor ) const {
  const PrismGeo & prism = visitor;
  using namespace chomp;
  std::vector<Grid::GridElement> results;

  //std::cout << "chomp::Prism version of Cover\n";
  static RectGeo G ( dimension_ );
  
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
            results . push_back ( ge ); // OUTPUT
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
  return results;
} // cover

///////////////////////////////
// INTERFACE TO CHOMP

inline TreeGrid::size_type TreeGrid::depth ( GridElement ge ) const {
  Tree::iterator it = GridToTree ( iterator ( ge ) );
  size_type result = 0;
  while ( it != tree () . begin () ) {
    it = tree () . parent ( it );
    ++ result;
  }
  return result;
}

template < class Container >
Grid::size_type TreeGrid::getDepth ( const Container & cont ) const {
  // TODO inefficient, intersecting tree climbs
  size_type result = 0;
  BOOST_FOREACH ( GridElement ge, cont ) {
    size_type d = depth ( ge );
    //std::cout << "Depth of " << geometry ( ge ) << " is " << d << "\n";
    if ( d > result ) result = d;
  }
  return result;
}

inline void TreeGrid::GridElementToCubes ( std::vector<std::vector < uint32_t > > * cubes,
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
inline void TreeGrid::relativeComplex ( chomp::RelativePair * pair,
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
  RectGeo newbounds ( D );
  for ( int d = 0; d < D; ++ d ) {
		newbounds . lower_bounds [ d ] = bounds () . upper_bounds [ d ];
		newbounds . upper_bounds [ d ] = bounds () . lower_bounds [ d ];
  }
#ifdef RELATIVECOMPLEXOUTPUT
long count = 0;
long endcount = XGridElements . size ();
int percent = 0;
#endif
  BOOST_FOREACH ( GridElement e, XGridElements ) {
#ifdef RELATIVECOMPLEXOUTPUT
    ++ count;
    if ( (100.0*count)/endcount > percent ) {
      percent = (100.0*count)/endcount;
      std::cout << "\r" << percent << "%    ";
      std::cout . flush ();
    }
#endif
    RectGeo geo = * boost::dynamic_pointer_cast<RectGeo>(geometry ( e ));
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
  
  X . bounds () = static_cast<chomp::Rect>(newbounds);
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