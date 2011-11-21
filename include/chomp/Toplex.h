// Toplex.h
// Shaun Harker
// 9/16/11

#ifndef CHOMP_TOPLEX_H
#define CHOMP_TOPLEX_H

#include "chomp/Prism.h"
#include "chomp/RelativePair.h"
#include "chomp/CubicalComplex.h"
#include "chomp/ToplexDetail.h"
#include "chomp/BitmapSubcomplex.h"
/**********
 * Toplex *
 **********/

class Toplex {
public:
  /* typedefs */
  typedef uint32_t size_type;
  typedef Toplex_const_iterator iterator;
  typedef iterator const_iterator;
  typedef GridElement Top_Cell; //compatibility
  typedef Prism Geometric_Description; // compatibility
  /* Basic Container Interface */
  void erase ( iterator erase_me );
  void clear ( void );
  iterator find ( const GridElement & find_me ) const;
  iterator begin ( void ) const;
  iterator end ( void ) const;
  size_type size ( void ) const;
  
  /// tree_size
  size_type tree_size ( void ) const;
  
  /// dimension
  int dimension ( void ) const;
  
  /// bounds
  Prism bounds ( void ) const;

  /// geometry
  Prism geometry ( const GridElement & GridElement ) const;
  Prism geometry ( const const_iterator & cell_iterator ) const;
  
  /// prefix ( GridElement )
  ///   Return a vector with the prefix string of tree moves necessary to 
  ///   navigate to its leaf in the Toplex tree structure
  
  std::vector < unsigned char > prefix ( const GridElement & element ) const;

  /// children ( GridElement)
  ///    Return an array of children GridElements,
  ///    or else empty if the GridElement is not further subdivided.
  template < class InsertIterator > void 
  children ( InsertIterator & ii, const GridElement & element ) const;

  /// umbrella ( std::vector < GridElement > & elements )
  ///    Return the set of all GridElements whose every descendent is in "elements"
  template < class InsertIterator, class Container > void 
  umbrella ( InsertIterator & ii, const Container & elements ) const;

  /// cover
  template < class InsertIterator > void 
  cover ( InsertIterator & ii, const Prism & prism ) const;
  
  /// coarse cover   (whenever node containment, report parent, not children)
  template < class InsertIterator > void 
  coarseCover ( InsertIterator & ii, const Prism & geometric_region ) const;
  
  /// subdivide
  template < class InsertIterator > void 
  subdivide ( InsertIterator & ii, GridElement divide_me );

  template < class InsertIterator > void 
  subdivide ( InsertIterator & ii, iterator divide_me );
  
  template < class InsertIterator, class Container > void 
  subdivide ( InsertIterator & ii, const Container & subset_to_divide );

  template < class InsertIterator > void 
  subdivide ( InsertIterator & ii );
  
  void subdivide ( void );
  
  /// depth
  int getDepth ( const GridElement & ge ) const;
  
  template < class Container >
  int getDepth ( const Container & subset ) const;
  
  /// coarsen
  template < class Container > void 
  coarsen ( const Container & coarsen_to );
  
  /// GridElementToCubes
  void GridElementToCubes ( std::vector<std::vector < uint32_t > > * cubes, 
                        const GridElement e, int d ) const;
  
  /// relativeComplex
  template < class Container > void 
  relativeComplex ( RelativePair * pair,
                    const Container & XGridElements,
                    const Container & AGridElements,
                    int depth) const;
  // Construction
  Toplex ( void );
  Toplex ( const Prism & outer_bounds_of_toplex );
  void initialize ( const Prism & outer_bounds_of_toplex );
  ~Toplex ( void );
  
private:
  const_iterator begin_;
  const_iterator end_;
  size_type size_;
  size_type tree_size_;
  std::vector < iterator > find_;
  Node * root_;
  Prism bounds_;
  int dimension_;
};

inline void Toplex::erase ( iterator erase_me ) {
  /* Update begin_ if necessary */
  /* TODO: if this is being used to kill an entire subtree, not just a leaf, then
   this will not update begin_ properly. */
  /* BUG: size not updated properly, because when we move onto parents
   it keeps decreasing the size erroneously */
  /* debug */
  if ( erase_me . node () -> left_ != NULL ||
      erase_me . node () -> right_ != NULL )
    std::cout << "Erasing a non-leaf node\n";
  /* end debug */
  if ( erase_me == begin_ ) ++ begin_;
  /* Disconnect from parent */
  Node * node = erase_me . node_;
  while ( 1 ) {
    Node * parent = node -> parent_;
    if ( parent == NULL ) {
      /* We are emptying the toplex -- the root is gone! */
      root_ = NULL;
      break;
    }
    /* This is not the root: a parent exists. */
    if ( parent -> left_ == node ) {
      /* This is a left-child; disconnect accordingly. */
      parent -> left_ = NULL;
    } else {
      /* This is a right-child; disconnect accordingly. */
      parent -> right_ = NULL;
    } /* if-else */
    if ( parent -> left_ != parent -> right_ ) break;
    /* We will erase this node and move on to erase its parent */
    find_ [ node -> contents_ ] = end_;
    delete node;
    node = parent;
  } /* while */
  /* Update find_ */
  find_ [ node -> contents_ ] = end_;
  /* Update size_ */
  -- size_;
  /* Deallocate the node */
  delete node;
} /* Adaptive_Cubical::Toplex::erase */

inline void Toplex::clear ( void ) {
  size_ = 0;
  tree_size_ = 0;
  find_ . clear ();
  begin_ = end_;
  if ( root_ != NULL ) delete root_;
} /* Adaptive_Cubical::Toplex::clear */

inline Toplex::iterator Toplex::find ( const GridElement & find_me ) const {
  return find_ [ find_me ];
} /* Adaptive_Cubical::Toplex::find */

inline Toplex::iterator Toplex::begin ( void ) const {
  return begin_;
} /* Adaptive_Cubical::Toplex::begin */

inline Toplex::iterator Toplex::end ( void ) const {
  return end_;
} /* Adaptive_Cubical::Toplex::end */

inline Toplex::size_type Toplex::size ( void ) const {
  return size_;
} /* Adaptive_Cubical::Toplex::size */

inline Toplex::size_type Toplex::tree_size ( void ) const {
  return tree_size_;
} /* Adaptive_Cubical::Toplex::tree_size */

inline int Toplex::dimension ( void ) const {
  return dimension_;
} /* Adaptive_Cubical::Toplex::dimension */

inline Prism Toplex::bounds ( void ) const {
  return bounds_;
} /* Adaptive_Cubical::Toplex::bounds */

inline std::vector < unsigned char > Toplex::prefix ( const GridElement & cell ) const {
  std::vector < unsigned char > result, reversed;
  iterator cell_iterator = find ( cell );
  Node * node_ptr = cell_iterator . node_;
  while ( node_ptr != root_ ) {
    Node * parent = node_ptr -> parent_;
    if ( parent -> left_ == node_ptr ) {
      /* This is a left-child */
      reversed . push_back ( 0 );
    } else {
      /* This is a right-child */
      reversed . push_back ( 1 );
    } /* if-else */
    node_ptr = parent;
  } /* while */
  /* Now reverse the order */
  for ( int i = reversed . size () - 1; i >= 0; -- i ) {
    result . push_back ( reversed [ i ] );
  }
  // WEIRD: the algorithm reverse_copy doesn't work.
  // I get this bizarre seg-fault, and gdb tells me we've have been hijacked
  // by some boost MPL crap (other than STL implementation). Apparently, it doesn't work right.
  // std::reverse_copy ( reversed . begin (), reversed . end (), result . begin () );
  return result;
} /* Adaptive_Cubical::Toplex::prefix */

template < class InsertIterator > void 
Toplex::children ( InsertIterator & ii, 
                   const GridElement & element ) const {
  iterator cell_iterator = find ( element );
  Node * node_ptr = cell_iterator . node_;
  std::stack < Node * > nodes;
  nodes . push ( node_ptr );
  while ( not nodes . empty () ) {
    Node * ptr = nodes . top ();
    nodes . pop ();
    if ( ptr -> dimension_ == 0 ) {
      * ii ++ = ptr -> contents_;
    } else {
      if ( ptr -> left_ != NULL ) {
        nodes . push ( ptr -> left_ );
      }
      if ( ptr -> right_ != NULL ) {
        nodes . push ( ptr -> right_ );
      }
    }
  }
}

template < class InsertIterator, class Container > void 
Toplex::umbrella ( const Container & elements ) const {
  std::vector<GridElement> result ( elements . begin (), elements . end () );
  boost::unordered_set < GridElement > umb;
  int N = result . size ();
  for ( int i = 0; i < N; ++ i ) {
    GridElement element = result [ i ];
    iterator cell_iterator = find ( element );
    Node * parent_node = find ( element ) . node_ -> parent_;
    if ( parent_node == NULL ) continue;
    GridElement parent =  parent_node -> contents_;
    if ( umb . insert ( parent ) . second ) {
      result . push_back ( parent );
      ++ N;
    }
  }
  for ( int i = 0; i < N; ++ i ) * ii ++ = result [ i ];
}


inline Prism Toplex::geometry ( const const_iterator & cell_iterator ) const {
  Prism return_value ( dimension_, Real ( 0 ) );
  //std::cout << "geometry of " << cell_iterator . node_ << " (" << cell_iterator . node_ -> contents_ << ")\n";
  //std::cout << "root = " << root_ << "\n";
  /* Climb the tree */
  Node * node_ptr = cell_iterator . node_;
  while ( node_ptr != root_ ) {
    //std::cout << "visiting " << node_ptr << " with parent " << node_ptr -> parent_ << "\n";
    Node * parent = node_ptr -> parent_;
    int division_dimension = parent -> dimension_;
    if ( parent -> left_ == node_ptr ) {
      /* This is a left-child */
      return_value . upper_bounds [ division_dimension ] += Real ( 1 );
    } else {
      /* This is a right-child */
      return_value . lower_bounds [ division_dimension ] += Real ( 1 );
    } /* if-else */
    return_value . lower_bounds [ division_dimension ] /= Real ( 2 );
    return_value . upper_bounds [ division_dimension ] /= Real ( 2 );
    node_ptr = parent;
  } /* while */
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    /* Produce convex combinations */
    return_value . lower_bounds [ dimension_index ] = return_value . lower_bounds [ dimension_index ] * bounds_ . upper_bounds [ dimension_index ] +
    ( Real ( 1 ) - return_value . lower_bounds [ dimension_index ] ) * bounds_ . lower_bounds [ dimension_index ];
    return_value . upper_bounds [ dimension_index ] = return_value . upper_bounds [ dimension_index ] * bounds_ . lower_bounds [ dimension_index ] +
    ( Real ( 1 ) - return_value . upper_bounds [ dimension_index ] ) * bounds_ . upper_bounds [ dimension_index ];
    //DEBUG
    if ( return_value . lower_bounds [ dimension_index ] > return_value . lower_bounds [ dimension_index ] ) {
      std::cout << "Toplex::geometry ERROR: constructed invalid region.\n";
      exit(1);
    }
  } /* for */
  //std::cout << "returning.\n";
  return return_value;
} /* Adaptive_Cubical::Toplex::geometry */

inline Prism Toplex::geometry ( const GridElement & cell  ) const {
  return geometry ( find ( cell ) );
} /* Adaptive_Cubical::Toplex::geometry */


template < class InsertIterator >
inline void Toplex::cover ( InsertIterator & ii, const Prism & geometric_region ) const {
  
  /* Use a stack, not a queue, and do depth first search.
   The advantage of this is that we can maintain the geometry during our Euler Tour.
   We can maintain our geometry without any roundoff error if we use the standard box
   [0,1]^d. To avoid having to translate to real coordinates at each leaf, we instead
   convert the input to these standard coordinates, which we put into integers. */
  
  // Step 1. Convert input to standard coordinates. 
  Prism region ( dimension_ );
  static std::vector<uint64_t> LB ( dimension_);
  static std::vector<uint64_t> UB ( dimension_);
#define INTPHASEWIDTH (((uint64_t)1) << 60)
  static Real bignum ( INTPHASEWIDTH );
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    region . lower_bounds [ dimension_index ] = 
    (geometric_region . lower_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
    (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
    region . upper_bounds [ dimension_index ] = 
    (geometric_region . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
    (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
    
    if ( region . upper_bounds [ dimension_index ] < Real ( 0 ) ) return;
    if ( region . lower_bounds [ dimension_index ] > Real ( 1 ) ) return;
    
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
  
  // Step 2. Perform DFS on the Toplex tree, recursing whenever we have intersection,
  //         (or adding leaf to output when we have leaf intersection)
  static std::vector<uint64_t> NLB ( dimension_);
  static std::vector<uint64_t> NUB ( dimension_);
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    if ( LB [ dimension_index ] < (1 << 20) ) LB [ dimension_index ] = 0;
    if ( LB [ dimension_index ] >= (1 << 20) ) LB [ dimension_index ] -= (1 << 20);
    if ( UB [ dimension_index ] < (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] += (1 << 20);
    if ( UB [ dimension_index ] >= (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] = INTPHASEWIDTH;
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
  
  Node * N = root_;
  char state = 0; 
  
  //std::cout << "D\n";
  
  while ( 1 ) {
    //std::cout << "Entering Loop, state = " << (int) state << "\n";
    //std::cout << " N = " << N << "\n";
    if ( state == 0 ) {
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
        if ( N -> left_ == NULL ) {
          if ( N -> right_ == NULL ) {
            // Here's what we are looking for.
            * ii ++ = N -> contents_; // OUTPUT
                                      //std::cout << "cover -- " << N -> contents_ << "\n";
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
      int div_dim = N -> dimension_;
      NUB[div_dim] -= ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = N -> left_;
      state = 0;
      continue;
    } // state 1
    
    if ( state == 2 ) {
      // We have been ordered to descend to the right.
      //std::cout << "Descend right.\n";
      int & div_dim = N -> dimension_;
      NLB[div_dim] += ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = N -> right_;
      state = 0;
      continue;
    } // state 2
    
    if ( state == 3 ) {
      // We have been ordered to rise.
      //std::cout << "Rise.\n";
      Node * P = N -> parent_;
      // Can't rise if root.
      if ( P == NULL ) break; // algorithm complete
      int & div_dim = P -> dimension_;
      if ( P -> left_ == N ) {
        // This is a left child.
        //std::cout << "We are rising from left.\n";
        NUB[div_dim] += NUB[div_dim]-NLB[div_dim];
        // If we rise from the left child, we order parent to go right.
        state = 2;
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
} // cover


template < class InsertIterator >
inline void Toplex::coarseCover ( InsertIterator & ii, const Prism & geometric_region ) const {
  
  /* Use a stack, not a queue, and do depth first search.
   The advantage of this is that we can maintain the geometry during our Euler Tour.
   We can maintain our geometry without any roundoff error if we use the standard box
   [0,1]^d. To avoid having to translate to real coordinates at each leaf, we instead
   convert the input to these standard coordinates, which we put into integers. */
  
  // Step 1. Convert input to standard coordinates. 
  Prism region ( dimension_ );
  static std::vector<uint64_t> LB ( dimension_);
  static std::vector<uint64_t> UB ( dimension_);
#define INTPHASEWIDTH (((uint64_t)1) << 60)
  static Real bignum ( INTPHASEWIDTH );
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    region . lower_bounds [ dimension_index ] = 
    (geometric_region . lower_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
    (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
    region . upper_bounds [ dimension_index ] = 
    (geometric_region . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]) /
    (bounds_ . upper_bounds [ dimension_index ] - bounds_ . lower_bounds [ dimension_index ]);
    
    if ( region . upper_bounds [ dimension_index ] < Real ( 0 ) ) return;
    if ( region . lower_bounds [ dimension_index ] > Real ( 1 ) ) return;
    
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
  
  // Step 2. Perform DFS on the Toplex tree, recursing whenever we have intersection,
  //         (or adding leaf to output when we have leaf intersection)
  static std::vector<uint64_t> NLB ( dimension_);
  static std::vector<uint64_t> NUB ( dimension_);
  for ( int dimension_index = 0; dimension_index < dimension_; ++ dimension_index ) {
    if ( LB [ dimension_index ] < (1 << 20) ) LB [ dimension_index ] = 0;
    if ( LB [ dimension_index ] >= (1 << 20) ) LB [ dimension_index ] -= (1 << 20);
    if ( UB [ dimension_index ] < (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] += (1 << 20);
    if ( UB [ dimension_index ] >= (INTPHASEWIDTH - (1 << 20)) ) UB [ dimension_index ] = INTPHASEWIDTH;
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
  
  Node * N = root_;
  char state = 0; 
  
  //std::cout << "D\n";
  
  while ( 1 ) {
    //std::cout << "Entering Loop, state = " << (int) state << "\n";
    //std::cout << " N = " << N << "\n";
    if ( state == 0 ) {
      // If we have descended here, then we should check for intersection.
      bool intersect_flag = true;
      bool contain_flag = true;
      for ( int d = 0; d < dimension_; ++ d ) {
        if ( LB[d] > NUB[d] || UB[d] < NLB [d] ) {  // INTERSECTION CHECK
          intersect_flag = false;
          break;
        }
        if ( LB[d] > NLB[d] || UB[d] < NUB [d] ) {  // CONTAINMENT CHECK
          contain_flag = false;
        }
      }
      
      if ( contain_flag ) {
        // Here's what we are looking for.
        * ii ++ = N -> contents_; // OUTPUT
                                  //std::cout << "cover -- " << N -> contents_ << "\n";
                                  // Issue the order to rise.
                                  //std::cout << "Issue rise.\n";
        state = 3;
      } else if ( intersect_flag ) {
        //std::cout << "Detected intersection.\n";
        // Check if its a leaf.
        if ( N -> left_ == NULL ) {
          if ( N -> right_ == NULL ) {
            // Here's what we are looking for.
            * ii ++ = N -> contents_; // OUTPUT
                                      //std::cout << "cover -- " << N -> contents_ << "\n";
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
      int div_dim = N -> dimension_;
      NUB[div_dim] -= ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = N -> left_;
      state = 0;
      continue;
    } // state 1
    
    if ( state == 2 ) {
      // We have been ordered to descend to the right.
      //std::cout << "Descend right.\n";
      int & div_dim = N -> dimension_;
      NLB[div_dim] += ( (NUB[div_dim]-NLB[div_dim]) >> 1 );
      N = N -> right_;
      state = 0;
      continue;
    } // state 2
    
    if ( state == 3 ) {
      // We have been ordered to rise.
      //std::cout << "Rise.\n";
      Node * P = N -> parent_;
      // Can't rise if root.
      if ( P == NULL ) break; // algorithm complete
      int & div_dim = P -> dimension_;
      if ( P -> left_ == N ) {
        // This is a left child.
        //std::cout << "We are rising from left.\n";
        NUB[div_dim] += NUB[div_dim]-NLB[div_dim];
        // If we rise from the left child, we order parent to go right.
        state = 2;
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
} // cover

template < class InsertIterator > 
inline void Toplex::subdivide ( InsertIterator & ii, GridElement divide_me ) {
  subdivide ( ii, find ( divide_me ) );
}

template < class InsertIterator >
inline void Toplex::subdivide ( InsertIterator & ii, iterator cell_to_divide ) {
  std::deque < std::pair < const_iterator, int > > work_deque;
  work_deque . push_back ( std::pair < const_iterator, int >
                          (cell_to_divide,
                           cell_to_divide . node () -> dimension_ ) );
  while ( not work_deque . empty () ) {
    std::pair < const_iterator, int >  work_pair = work_deque . front ();
    work_deque . pop_front ();
    if ( work_pair . second < dimension_ ) {
      work_pair . first . node_ -> dimension_ = work_pair . second;
      /* We must subdivide further */
      work_pair . first . node_ -> left_ = new Node;
      work_pair . first . node_ -> right_ = new Node;
      /* Update begin_, size_, tree_size_, find_ and initialize new nodes.*/
      if ( begin_ == work_pair . first . node_ )
        begin_ . node_ = work_pair . first . node_ -> left_;
      ++ size_;
      work_pair . first . node_ -> left_ -> contents_ = tree_size_ ++;
      work_pair . first . node_ -> left_ -> parent_ = work_pair . first . node_;
      find_ . push_back ( const_iterator ( work_pair . first . node_ -> left_ ) );
      work_pair . first . node_ -> right_ -> contents_ = tree_size_ ++;
      work_pair . first . node_ -> right_ -> parent_ = work_pair . first . node_;
      find_ . push_back ( const_iterator ( work_pair . first . node_ -> right_ ) );
      /* Push the children onto the work_deque */
      work_deque . push_back ( std::pair < const_iterator, int >
                              (const_iterator ( work_pair . first . node_ -> left_ ),
                               work_pair . second + 1 ) );
      work_deque . push_back ( std::pair < const_iterator, int >
                              (const_iterator ( work_pair . first . node_ -> right_ ),
                               work_pair . second + 1 ) );
    } else {
      work_pair . first . node_ -> dimension_ = 0;
      //std::cout << "subdivide: inserting " << work_pair . first . node_ -> contents_ << "\n";
      * ii ++ = work_pair . first . node_ -> contents_;
    } /* if-else */
  } /* while */
} /* Adaptive_Cubical::Toplex::subdivide */

template < class InsertIterator, class Container >
inline void Toplex::subdivide ( InsertIterator & ii, const Container & subset_to_divide ) {
  BOOST_FOREACH ( GridElement cell, subset_to_divide ) {
    //std::cout << "subdividing " << cell << "\n";
    subdivide ( ii, find ( cell ) );
  }
}

template < class InsertIterator >
inline void Toplex::subdivide ( InsertIterator & ii ) {
  // TODO: CHECK THIS
  std::vector < GridElement > all;
  std::insert_iterator<std::vector<GridElement> > all_ii ( all, all . end () );
  cover ( all_ii, bounds () );
  subdivide ( ii, all );
}

inline void Toplex::subdivide ( void ) {
  std::vector < GridElement > dummy;
  std::insert_iterator<std::vector<GridElement> > ii ( dummy, dummy . end () );
  subdivide ( ii );
}

/// depth
inline int Toplex::getDepth ( const GridElement & ge ) const {
  std::vector < unsigned char > p = prefix ( ge );
  return p . size () / dimension ();
}

template < class Container >
int Toplex::getDepth ( const Container & subset ) const {
  int depth = 0;
  BOOST_FOREACH ( const GridElement & ge, subset ) {
    int ge_depth = getDepth ( ge );
    if ( ge_depth > depth ) depth = ge_depth;
  }
  return depth;
}

/////////////////////// COARSEN /////////////////////////////
inline void branch ( std::vector < Node * > & nodes, Node * node ) {
  nodes . push_back ( node );
  // std::cout << "  pushing " << node << "  " << node -> contents_ << "\n";
  if ( node -> left_ != NULL ) branch ( nodes, node -> left_ );
  if ( node -> right_ != NULL ) branch ( nodes, node -> right_ );
}

template < class CellContainer >
inline void Toplex::coarsen ( const CellContainer & coarsen_to ) {
  std::vector < Node * > nodes;
  // Produce a list "nodes" of all descendant nodes
  BOOST_FOREACH ( GridElement cell, coarsen_to ) {
    Node * node = find ( cell ) . node ();
    //std::cout << "Coarsen to: " << node << "  " << node -> contents_ << "\n";
    // If this will be a new leaf and was not before, increment size
    if ( node -> left_ != NULL || node -> right_ != NULL ) {
      ++ size_;
      //std::cout << "  Will be a new leaf.\n";
    } 
    if ( node -> left_ != NULL ) branch ( nodes, node -> left_ );
    if ( node -> right_ != NULL ) branch ( nodes, node -> right_ );  
    node -> left_ = node -> right_ = NULL;
  }
  // Remove the top cells from the find structure
  BOOST_FOREACH ( Node * node, nodes ) {
    //std::cout <<  "Removing node " << node << "  " << node -> contents_ << "\n";
    find_ [ node -> contents_ ] = end ();
    // If we are deleting what used to be a leaf, decrement size
    if ( node -> left_ == NULL && node -> right_ == NULL ) -- size_;
    // Single delete (not recursive, so forget children first)
    node -> left_ = node -> right_ = NULL;
    delete node;
  }
  // Recompute begin_
  Node * n = root_;
  while ( n -> left_ != NULL || n -> right_ != NULL ) {
    while ( n -> left_ != NULL ) n = n -> left_;
    if ( n -> right_ != NULL ) n = n -> right_;
  }
  begin_ = iterator ( n );
}

inline void Toplex::GridElementToCubes ( std::vector<std::vector < uint32_t > > * cubes, 
                     const GridElement e, int depth ) const {
  // Obtain the prefix
  //std::cout << "GEtoCubes: " << geometry ( e ) << "\n";
  int D = dimension ();
  std::vector < unsigned char > p = prefix ( e );
  int GridElement_depth = p . size () / D;
  //std::cout << "  gedepth = " << GridElement_depth << ", from " << p . size () << "\n";
  if ( GridElement_depth > depth ) GridElement_depth = depth; //effectively truncates the prefix
  // Determine width
  std::vector < uint32_t > cube ( D, 0 );
  int pos = 0;
  for ( int d = 0; d < GridElement_depth; ++ d ) {
    for ( int dim = 0; dim < D; ++ dim ) {
      cube [ dim ] <<= 1;
      cube [ dim ] |= (uint32_t) p [ pos ++ ];
    }
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
  int depth_diff = depth - GridElement_depth; 
  for ( int dim = 0; dim < D; ++ dim ) {
    cube [ dim ] <<= depth_diff;
  }
  uint64_t num_cubes = ((uint64_t) 1) << (D * depth_diff);
  uint64_t mask = (1 << depth_diff) - 1;
  for ( uint64_t cube_num = 0; cube_num < num_cubes; ++ cube_num ) {
    std::vector < uint32_t > outcube ( D, 0 );
    for ( int dim = 0; dim < D; ++ dim ) {
      uint32_t value = (uint32_t) ( mask & ( cube_num >> (dim * depth_diff) ) );
      //std::cout << "value = " << value << "\n";
      outcube [ dim ] = ( cube [ dim ] | value );
    }
    cubes -> push_back ( outcube );
  }
  
}

template < class Container >
inline void Toplex::relativeComplex ( RelativePair * pair,
                                      const Container & XGridElements,
                                      const Container & AGridElements,
                                      int depth ) const {
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
  X . bounds () = bounds ();
  uint32_t width = 1 << depth;
  std::vector < uint32_t > dimension_sizes ( dimension (), width );
  X . initialize ( dimension_sizes );
  
  //std::cout << "relativeComplex.\n";
  //std::cout << "XGridElements:\n";
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
      X . addFullCube ( cube );
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
      std::vector < std::vector < Index > > cells = 
      X . fullCubeIndexes ( cube );
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

inline void Toplex::initialize ( const Prism & outer_bounds_of_toplex ) {
  if ( root_ != NULL ) clear ();
  dimension_ = outer_bounds_of_toplex . lower_bounds . size ();
  bounds_ = outer_bounds_of_toplex;
  root_ = new Node;
  tree_size_ = 1;
  size_ = 1;
  begin_ = const_iterator ( root_ );
  find_ . push_back ( begin_ );
} /* Adaptive_Cubical::Toplex::initialize */

inline Toplex::Toplex ( void ) {
  end_ = const_iterator ( NULL );
  begin_ = end_;
  size_ = 0;
  tree_size_ = 0;
  root_ = NULL;
  dimension_ = 0;
} /* Adaptive_Cubical::Toplex::Toplex */

inline Toplex::Toplex ( const Prism & outer_bounds_of_toplex ) {
  end_ = const_iterator ( NULL );
  begin_ = end_;
  size_ = 0;
  tree_size_ = 0;
  root_ = NULL;
  dimension_ = 0;
  initialize ( outer_bounds_of_toplex );
} /* Adaptive_Cubical::Toplex::Toplex */

inline Toplex::~Toplex ( void ) {
  if ( root_ != NULL ) delete root_;
} /* Adaptive_Cubical::Toplex::~Toplex */

#endif
