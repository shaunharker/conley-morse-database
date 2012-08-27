/* Compute_Morse_Graph.hpp */
#ifndef _CMDP_COMPUTE_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_MORSE_GRAPH_HPP_

#include <map>
#include <stack>
#include <vector>
#include "boost/foreach.hpp"
#include "database/algorithms/GraphTheory.h"
#include "database/structures/MapGraph.h"

#include <ctime>

#ifdef DO_CONLEY_INDEX
#include "chomp/ConleyIndex.h"
#endif
template < class Toplex, class CellContainer > 
void subdivide ( Toplex & phase_space, CellContainer & morse_set );


// Some macros for verbose output.
#ifdef CMG_VERBOSE

#define CMG_VERBOSE_PRINT(x) std::cout << x;

#define CMG_VERBOSE_START_CLOCK \
clock_t start = clock ();

#define CMG_VERBOSE_REPORT_CLOCK \
std::cout << "   Time Elapsed: " << (double)(clock() - start)/(double)CLOCKS_PER_SEC << "\n"; 

#define CMG_VERBOSE_REPORT_MORSE_SETS \
std::cout << "   Number of Morse Sets: " << morse_sets . size () << "\n";\
std::cout << "   Sizes of Morse Sets: ";\
BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) \
std::cout << " " << morse_set . size ();\
std::cout << ".\n";

#endif

#ifndef CMG_VERBOSE
#define CMG_VERBOSE_PRINT(x)  if(0){std::cout << x;}
#define CMG_VERBOSE_START_CLOCK
#define CMG_VERBOSE_REPORT_CLOCK
#define CMG_VERBOSE_REPORT_MORSE_SETS
#endif


template < class Toplex, class CellContainer >
void subdivide ( Toplex * phase_space, CellContainer & morse_set ) {
  CellContainer new_morse_set;
  std::insert_iterator<CellContainer> ii ( new_morse_set, new_morse_set . begin () );
  phase_space -> subdivide (ii, morse_set);
  std::swap ( new_morse_set, morse_set );
}

template < class Toplex >
class MorseDecomposition {
public:
  typedef std::vector<typename Toplex::Top_Cell> CellContainer;
  // Member Data
  CellContainer set;
  std::vector < MorseDecomposition * > children;
  std::vector < std::vector < unsigned int > > reachability;
  bool subdivided;
  bool spurious;
  // Constructor
  template < class Map >
  MorseDecomposition (Toplex * phase_space, 
                      const Map & interval_map,
                      const unsigned int Min, 
                      const unsigned int Max, 
                      const unsigned int Limit,
                      const unsigned int depth,
                      const CellContainer & set ) : set(set) {
    CMG_VERBOSE_PRINT("Depth = " << depth << "\n");
    typedef MapGraph<Toplex,Map,CellContainer> Graph;  
    // Check subdivision condition.
    if ( depth >= Min && ( set . size () > Limit || depth >= Max ) ) {
      subdivided = false;
      spurious = false;
      return;
    }
    // Subdivide
    subdivided = true;
    spurious = true; // This may be changed later.
    CellContainer newset = set;
    subdivide ( phase_space, newset );
    // Create children.
    std::vector < CellContainer > morse_sets;
    Graph G ( newset, * phase_space, interval_map );
    computeMorseSetsAndReachability <Graph,CellContainer> 
    ( &morse_sets, &reachability, G );
#ifdef CMG_VERBOSE 
    if ( morse_sets . size () > 1 ) {
      std::cout << "Splits into " << morse_sets . size () << " morse sets.\n";
    }
    std::cout << "Sizes of children: ";
    BOOST_FOREACH ( const CellContainer & morse_set, morse_sets ) {
      std::cout << morse_set . size (); 
    }
    std::cout << "\n";
#endif 
    BOOST_FOREACH ( const CellContainer & morse_set, morse_sets ) {
      MorseDecomposition * child = 
      new MorseDecomposition (phase_space,
                              interval_map,
                              Min,
                              Max,
                              Limit,
                              depth + 1,
                              morse_set);
      
      children . push_back ( child );
      if ( not child -> spurious ) spurious = false;
      if ( not spurious && depth >= Min ) {
        CMG_VERBOSE_PRINT("Not spurious.\n");
        break;
      }
    }
    // If deeper than min, erase sub-hierarchy and coarsen grid.
    if ( depth == Min ) {
      CMG_VERBOSE_PRINT("Coarsening to min-level\n");
      phase_space -> coarsen ( set );
      BOOST_FOREACH ( const MorseDecomposition * child, children ) {
        delete child;
      }
      children . clear ();
      reachability . clear ();
      
#ifdef DO_CONLEY_INDEX
      if ( not spurious ) {
      using namespace chomp;
      ConleyIndex_t output;
      ConleyIndex ( &output,
                   *phase_space, 
                   set,
                   interval_map );
      }
#endif

    }
    CMG_VERBOSE_PRINT("Returning from depth " << depth << ".\n");
  }
  ~MorseDecomposition ( void ) {
    BOOST_FOREACH ( const MorseDecomposition * child, children ) {
      delete child;
    }
  }
};


template < class Morse_Graph, class Toplex, class Map >
void Compute_Morse_Graph (Morse_Graph * MG, 
                          Toplex * phase_space, 
                          const Map & interval_map,
                          const unsigned int Min, 
                          const unsigned int Max, 
                          const unsigned int Limit) {
  using namespace chomp;
  typedef std::vector<typename Toplex::Top_Cell> CellContainer;
  
  // Produce initial Morse Set
  CellContainer morse_set;
  std::insert_iterator < CellContainer > ii (morse_set, 
                                             morse_set . begin () );
  phase_space -> cover ( ii, phase_space -> bounds () );
  
  // Produce Morse Set Decomposition
  MorseDecomposition<Toplex> D (phase_space,
                                interval_map,
                                Min,
                                Max,
                                Limit,
                                0,
                                morse_set);
  
  // Produce Morse Graph
  typedef typename Morse_Graph::Vertex Vertex;
  std::map < MorseDecomposition<Toplex> *, std::vector<Vertex> > temp;
  std::stack < std::pair < MorseDecomposition<Toplex> *, unsigned int > > eulertourstack;
  eulertourstack . push ( std::make_pair( &D, 0 ) );
  while (  not eulertourstack . empty () ) {
    MorseDecomposition<Toplex> * MD = eulertourstack . top () . first;
    unsigned int childnum = eulertourstack . top () . second;
    eulertourstack . pop ();
    unsigned int N = MD -> children . size ();
    if ( childnum == N ) {
      // Exhausted children already.
      // Amalgamate reachability information
      for ( unsigned int i = 0; i < N; ++ i ) {
        const std::vector < unsigned int > & reaches = MD -> reachability [ i ];
        //std::cout << "reaching info: " << reaches . size () << "\n";
        BOOST_FOREACH ( unsigned int j, reaches ) {
          if ( i == j ) continue;
          BOOST_FOREACH ( Vertex u, 
                         temp [ MD -> children [ i ] ] ) {
            BOOST_FOREACH ( Vertex v, 
                           temp [ MD -> children [ j ] ] ) {
              //std::cout << "Adding edge (" << u << ", " << v << ")\n";
              MG -> AddEdge ( u, v );
            }
          }
        }
      }
      // Aggregate temp data 
      temp [ MD ] = std::vector < Vertex > ();
      if ( N == 0 ) {
        if ( MD -> spurious == false ) {  
          Vertex v = MG -> AddVertex ();
          MG -> CellSet ( v ) = MD -> set;
          temp [ MD ] . push_back ( v );
        }
      } else {
        for ( unsigned int i = 0; i < N; ++ i ) {
          temp [ MD ] . insert (temp [ MD ] . begin (), 
                                temp [ MD -> children [ i ] ] . begin (),
                                temp [ MD -> children [ i ] ] . end ());
          temp . erase ( MD -> children [ i ] );
        }
      }
    } else {
      eulertourstack . push ( std::make_pair ( MD, childnum + 1 ) );
      eulertourstack . push ( std::make_pair ( MD -> children [ childnum ], 0 ) );
    }
  }
}

#endif
