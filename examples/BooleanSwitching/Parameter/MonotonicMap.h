/// MonotonicMap.h
/// Author: Shaun Harker
/// Date: August 20, 2014

#ifndef MONOTONICMAP_H
#define MONOTONICMAP_H

#include <iostream>
#include <vector>
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"

/// class MonotonicMap
/// a "smart vertex" class representing the dynamics of a node of
/// a boolean switching network
class MonotonicMap {
public:
  // data
  int n; // number of in edges  -- domain is {0,1,...,2^n-1}
  int m; // number of out edges -- codomain is {0,1,...,m}

  /// logic_
  ///    An expression of the form (a+b+c)(d+e)f(g+h)
  ///    would be encoded as vector of ints 3,2,1,2 (the number of summands in each
  ///    factor). Up-regulation and down-regulation are of no concern here (they are
  ///    treated elsewhere)
  ///    The entries in logic_ line up to the bits in the domain of data_
  ///    in such a way that the last entry of logic_ corresponds to the least
  ///    significant bit of data_'s domain.
  std::vector< int> logic_; // logic
  
  /// data_
  ///    data_ is an array storing the values of
  ///    a map from {0,1,...2^n-1} -> {0,1,...m}
  std::vector< int > data_;  // mapping


  // constructors
  MonotonicMap ( void ) {}
  MonotonicMap ( int n, int m ) : n(n), m(m) {
    data_ . resize ( (1 << n), 0 );
    logic_ . resize ( 1, n ); // all-sum by default
  }
  MonotonicMap ( int n, int m, const std::vector<int> & logic ): n(n), m(m), logic_(logic) {
    data_ . resize ( (1 << n), 0 );
  }
  MonotonicMap ( int n, 
                 int m, 
                 const std::vector<int> & logic, 
                 const std::vector<int> & data )
    : n(n), m(m), logic_(logic), data_(data) {}

  // Check if monotonic
  bool monotonic ( void ) const {
    //std::cout << "Calling monotonic\n";
    int N = (1 << n);
    for ( int i = 0; i < N; ++ i ) {
      std::vector<int> children;
      for ( int pos = 0; pos < n; ++ pos ) {
        int bit = 1 << pos;
        if ( not ( i & bit ) ) { 
          //std::cout << "Pushing child " << (i|bit) << " of " << i << "\n";
          children . push_back ( i | bit );
        }
      }
      BOOST_FOREACH ( int child, children ) {
        if ( data_[child] < data_[i] ) { 
          //std::cout << " Return false because data_[" << child << "] < data_[" << i << "]\n";
          //std::cout << " data_[" << child << "] = " << data_[child] << "\n";
          //std::cout << " data_[" << i << "] = " << data_[i] << "\n";

          return false;
        }
      }
    }
    //std::cout << "Returning true.\n";
    return true;
  }

  bool realizable ( void ) const {
    // currently this is sum-sum realizable

    // What is realizable?
    // For each subset of variables I,
    // define the complement to be J,
    // We want to realize a partial order on 2^I
    // by inducing it via every substitution of selection
    // of J variables.
    // This means I have to loop through all choice I of subsets of variables.
    // That shouldn't be hard. Then I have to loop through 
    // substitutions of the J variables.
    // And within that, I need to loop through pairs of choices of substitutions for
    // the I variables.
    // An upper bound for this procedure is 8^n. For small n it should be tractable.
    // Actually it would be better to loop through the pairs first and the
    // substitutions second, to avoid having to store information
    int max_terms_in_factor = 0;
    for ( int i = 0; i < logic_ . size (); ++ i ) {
      max_terms_in_factor = std::max ( max_terms_in_factor, logic_[i] );
    }

    if ( (logic_ . size () == 1) || (max_terms_in_factor == 1) ) {
      // Case (n) (all sum case) or Case (1,1,1,1...,1) (n-times, all product case)
      int N = (1 << n);
      for ( int i = 0; i < N; ++ i ) {
        // I corresponds to the bits of i that are on.
        //std::cout << "Top of loop\n";

        for ( int a = 0; a < N; ++ a ) {
          if ( (a & i) != a ) continue;
          for ( int b = 0; b < N; ++ b ) {
            if ( (b & i) != b ) continue;
            bool less = false;
            bool greater = false;
            for ( int c = 0; c < N; ++ c ) {
              if ( (c & i ) != 0 ) continue;
              int x = data_[a|c];
              int y = data_[b|c];
              /*
              std::cout << "\n a = " << a << "\n";
              std::cout << " b = " << b << "\n";
              std::cout << " c = " << c << "\n";
              std::cout << " a|c = " << (a|c) << "\n";
              std::cout << " b|c = " << (b|c) << "\n";
              std::cout << " data_[a|c] = " << data_[a|c] << "\n";
              std::cout << " data_[b|c] = " << data_[b|c] << "\n";
              */

              if ( x < y ) { 
                less = true;
              }
              if ( x > y ) { 
                greater = true;
              }
              if ( less && greater ) {
                //std::cout << "Returning false.\n";
                return false;
              }
            }
          }
        }
      }
      return true;
    } else if ( logic_ . size () == 2 ) {
      if ( logic_[0] == 2 && logic_[1] == 1 ) {
        // Case (2, 1)
        // In this notation the (correct) rules I gave for sum-product amount to
        // " 010 < 001 implies 110 <= 101 "  (Rule A)
        // " 100 < 001 implies 110 <= 011 "  (Rule B)
        // and then we have two versions of Rule C, which does allow a reverse:
        // " 010 > 100 implies 011 >= 101 "  (Rule C)
        // " 010 < 100 implies 011 <= 101 "  (Rule C, reversed)
        int D010 = data_[2];
        int D001 = data_[1];
        int D110 = data_[6]; 
        int D101 = data_[5];
        int D100 = data_[4];
        int D011 = data_[3];

        if ( (D010 < D001) && not (D110 <= D101) ) return false;
        if ( (D100 < D001) && not (D110 <= D011) ) return false;
        if ( (D010 > D100) && not (D011 >= D101) ) return false;
        if ( (D010 < D100) && not (D011 <= D101) ) return false;
        return true;
      }
      if ( logic_[0] == 1 && logic_[1] == 2 ) {
        // Case (1,2). Symmetric to case (2,1). (We just rotate the bits)
        int D010 = data_[2];
        int D001 = data_[1];
        int D110 = data_[6]; 
        int D101 = data_[5];
        int D100 = data_[4];
        int D011 = data_[3];

        if ( (D001 < D100) && not (D011 <= D110) ) return false;
        if ( (D010 < D100) && not (D011 <= D101) ) return false;
        if ( (D001 > D010) && not (D101 >= D110) ) return false;
        if ( (D001 < D010) && not (D101 <= D110) ) return false;
        return true;     
      }
    } 
    std::cout << "BooleanSwitching Node realizability condition unknown.\n";
    std::cout << "Logic size = " << logic_ . size () << "\n";
    for ( int i = 0; i < logic_ . size (); ++ i ) std::cout << logic_[i] << " ";
    std::cout << "\n";
    throw std::logic_error ( "MonotonicMap:: realizability algorithm cannot handle current situation\n");
    return false;
  }

  // return adjacent monotonic maps
  std::vector<boost::shared_ptr<MonotonicMap> > neighbors ( void ) const {
    //std::cout << "Calling neighbors.\n";
    std::vector<boost::shared_ptr<MonotonicMap> > results;

    // Obtain neighbors via changing the monotone function
    std::vector<int> copy = data_;
    int N = (1 << n);
    for ( int i = 0; i < N; ++ i ) {
      if ( copy[i] > 0 ) {
        -- copy[i];
        boost::shared_ptr<MonotonicMap> new_map ( new MonotonicMap ( n, m, logic_, copy ) );
        if ( new_map -> monotonic () && new_map -> realizable () ) 
          results . push_back ( new_map );
        ++ copy[i];
      }
      if ( copy[i] < m ) {
        ++ copy[i];
        boost::shared_ptr<MonotonicMap> new_map ( new MonotonicMap ( n, m, logic_, copy ) );
        if ( new_map -> monotonic () && new_map -> realizable () ) 
          results . push_back ( new_map );
        -- copy[i];
      }
    }

    return results;
  }

  bool operator == ( const MonotonicMap & rhs ) const {
    if ( n != rhs . n ) return false;
    if ( m != rhs . m ) return false;
    int N = (1 << n);
    for ( int i = 0; i < N; ++ i ) {
      if ( data_[i] != rhs.data_[i] ) return false;
    }
    return true;
  }

#if 0
  /// polytope
  ///   return polytope corresponding to monotonic map
  Polytope polytope ( void ) const {
    // TODO
    // Case division based on logic type.
    int max_terms_in_factor = 0;
    for ( int i = 0; i < logic_ . size (); ++ i ) {
      max_terms_in_factor = std::max ( max_terms_in_factor, logic_[i] );
    }
    if ( logic_ . size () == 1 ) { 
      // Case (n) (all sum case)
      for ( int k = 0; k < logic_[0]; ++ k ) {
        poly . addVariable ( k );
      }
    } else if ( max_terms_in_factor == 1 ) {
      // Case (1,1,1,1...,1) (n-times, all product case)
      
      // TODO 1. 
    } else if ( logic_ . size () == 2 ) {
      if ( logic_[0] == 2 && logic_[1] == 1 ) {
        // Case (2, 1)
        // TODO 2. 
      }
      if ( logic_[0] == 1 && logic_[1] == 2 ) {
        // Case (1,2). Symmetric to case (2,1). (We just rotate the bits)
        // TODO 3. 
      }
    } 
  }
#endif
  
  friend std::size_t hash_value ( const MonotonicMap & p ) {
    std::size_t seed = 0;
    int N = (1 << p.n);
    for ( int i = 0; i < N; ++ i ) {
      boost::hash_combine(seed, p.data_[i] );
    }
    return seed;
  }

  friend std::ostream & operator << ( std::ostream & stream, 
                                      const MonotonicMap & print_me ) {
    stream << "{(In,Out)=(" << print_me . n << ", " << print_me . m << "), Logic=(";
    for ( int i = 0; i < print_me . logic_ . size (); ++ i ) { 
      if ( i != 0 ) stream << ",";
      std::cout << print_me . logic_[i];
    }
    stream << "), Data=(";
    for ( int i = 0; i < print_me . data_ . size (); ++ i ) { 
      if ( i != 0 ) stream << ",";
      std::cout << print_me . data_[i];
    }
    stream << ")}";
    return stream;
  }
};

#endif
