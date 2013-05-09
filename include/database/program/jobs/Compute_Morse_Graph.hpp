/* Compute_Morse_Graph.hpp */
#ifndef _CMDP_COMPUTE_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_MORSE_GRAPH_HPP_

#ifdef CMG_VISUALIZE
#include <boost/unordered_map.hpp>
#include "CImg.h"
using namespace cimg_library;
#endif


#include <map>
#include <stack>
#include <vector>
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"

#include "database/algorithms/GraphTheory.h"
#include "database/structures/MapGraph.h"

#include <ctime>

#ifdef MEMORYBOOKKEEPING
uint64_t max_grid_internal_memory = 0;
uint64_t max_grid_external_memory = 0;
#endif

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


class MorseDecomposition {
public:

  // Constructor
  template < class GridPtr >
  MorseDecomposition ( GridPtr grid, int depth ) : grid_ ( grid ), spurious_(false), depth_(depth) {
    if ( grid_ . get () == NULL ) {std::cout << "Error Compute_Morse_Graph.hpp line 65\n"; abort (); }
#ifdef MEMORYBOOKKEEPING
    max_grid_external_memory += grid -> memory ();
    max_grid_internal_memory = std::max( max_grid_internal_memory, grid -> memory () );
#endif
  }
  
  // Deconstructor
  ~MorseDecomposition ( void ) {
    BOOST_FOREACH ( MorseDecomposition * child, children_ ) {
      delete child;
    }
  }
  
  /// MorseDecomposition::size
  /// return size of grid
  size_t size ( void ) const { return grid_ -> size (); }
  
  /// MorseDecomposition::depth
  /// tell how deep in hierarchical decomposition we are
  size_t depth ( void ) const { return depth_; }

  /// MorseDecomposition::children
  /// accessor method to return vector of MorseDecomposition * pointing to hierarchical children.
  /// note: empty until "decompose" is called.
  const std::vector < MorseDecomposition * > & children ( void ) const {
    return children_;
  }

  /// MorseDecomposition::reachability
  /// accessor method to obtain reachability_ private data member
  const std::vector < std::vector < unsigned int > > & reachability ( void ) const {
    return reachability_;
  }
  
  /// MorseDecomposition::grid
  /// accessor method to obtain grid_ shared_ptr data member
  boost::shared_ptr < Grid > grid ( void ) {
    return grid_;
  }
  
  /// MorseDecomposition::spurious
  /// accessor method to obtain spurious_ data member
  bool & spurious ( void ) { return spurious_; }
  
  /// MorseDecomposition::decompose
  ///
  /// Use graph theory to find SCC components, which are stored as type Grid
  /// Fill these into "decomposition_"
  /// Fill children_ with an equal sized vector of pointers to new MorseDecomposition objects seeded with those sets.
  /// Put reachability information obtained in "reachability_"
  template < class Map >
  const std::vector < MorseDecomposition * > & decompose ( const Map & f ) {
    //std::cout << "Perform Morse Decomposition\n"; // Perform Morse Decomposition
    computeMorseSetsAndReachability <Map> ( &decomposition_, &reachability_, * (grid_ . get ()), f );
    //std::cout << "Create Hierarchy Structure\n";// Create Hierarchy Structure with Subdivided Grids for Morse Sets
    for ( size_t i = 0; i < decomposition_ . size (); ++ i ) {      
      decomposition_ [ i ] -> subdivide ();
      children_ . push_back ( new MorseDecomposition ( decomposition_ [ i ], depth() + 1 ) );
    }
    return children_;
  }

  template < class Map >
  const std::vector < MorseDecomposition * > & decomposeODE ( const std::vector<Map> & maps ) {
    //std::cout << "Perform Morse Decomposition\n"; // Perform Morse Decomposition
    computeMorseSets <Map> ( &decomposition_, &reachability_, grid_, maps );
    //std::cout << "Create Hierarchy Structure\n";// Create Hierarchy Structure with Subdivided Grids for Morse Sets
    for ( size_t i = 0; i < decomposition_ . size (); ++ i ) {
      decomposition_ [ i ] -> subdivide ();
      children_ . push_back ( new MorseDecomposition ( decomposition_ [ i ], depth() + 1 ) );
    }
    return children_;
  }

  
private:
  // Member Data
  boost::shared_ptr<Grid> grid_;
  std::vector< boost::shared_ptr<Grid> > decomposition_;
  std::vector < MorseDecomposition * > children_;
  std::vector < std::vector < unsigned int > > reachability_;
  bool spurious_;
  size_t depth_;
};

class MorseDecompCompare {
public:
  bool operator () ( const MorseDecomposition * lhs, const MorseDecomposition * rhs ) {
    return lhs -> size () < rhs -> size ();
  }
};

// ConstructMorseDecomposition
template < class Map >
void
ConstructMorseDecomposition (MorseDecomposition * root,
                             const Map & f,
                             const unsigned int Min,
                             const unsigned int Max,
                             const unsigned int Limit ) {
  size_t nodes_processed = 0;
  // We use a priority queue in order to do the more difficult computations first.
  std::priority_queue < MorseDecomposition *, std::vector<MorseDecomposition *>, MorseDecompCompare > pq;
  pq . push ( root );
  while ( not pq . empty () ) {
    ++ nodes_processed;
    if ( nodes_processed % 1000 == 0 ) std::cout << nodes_processed << " nodes have been encountered on Morse Decomposition Hierarchy.\n";
    MorseDecomposition * work_node = pq . top ();
    pq . pop ();
#ifdef ODE_METHOD
    if ( work_node -> size () < 8 ) continue; // DEBUG
#endif
#ifndef ODE_METHOD
    std::vector < MorseDecomposition * > children = work_node -> decompose ( f );
#else
    std::vector < MorseDecomposition * > children = work_node -> decomposeODE ( f );
#endif
    if ( children . empty () ) {
      // Mark as spurious
      work_node -> spurious () = true;
    }
    BOOST_FOREACH ( MorseDecomposition * child, children ) {
      if ( child -> depth () == Max ) continue;
      if ( child -> depth () >= Min && child -> size () >= Limit ) continue;
      pq . push ( child );
    }
  }
}


// ConstructMorseDecomposition
template < class Map >
void ConstructMorseGraph (boost::shared_ptr<Grid> master_grid,
                     MorseGraph * MG,
                     MorseDecomposition * root,
                     const unsigned int Min ) {
  std::vector < boost::shared_ptr < Grid > > grids;
  // Produce Morse Graph
  typedef MorseGraph::Vertex Vertex;
  // "temp" will store which MorseGraph vertices are hierarchically under a given decomposition node
  std::map < MorseDecomposition *, std::vector<Vertex> > temp;
  std::stack < std::pair < MorseDecomposition *, unsigned int > > eulertourstack;
  eulertourstack . push ( std::make_pair( root, 0 ) );
  while (  not eulertourstack . empty () ) {
    MorseDecomposition * MD = eulertourstack . top () . first;
    unsigned int childnum = eulertourstack . top () . second;
    eulertourstack . pop ();
    unsigned int N = MD -> children () . size ();
    if ( childnum == N ) {
      // Exhausted children already.
      // Check for Spuriousness
      // If it has children that are all marked spurious, then it is spurious.
      // If it does not have children, it is spurious if and only if it is already marked spurious
      if ( N > 0 ) {
        MD -> spurious () = true;
        for ( unsigned int i = 0; i < N; ++ i ) {
          if ( not MD -> children () [ i ] -> spurious () ) MD -> spurious () = false;
        }
      }
      if ( MD -> spurious () ) continue;

      // Obtain grid to form master grid
      //TODO: GET POINTER GRID WORKING WITH BETTER JOIN MECHANISM
#ifdef USE_SUCCINCT
        grids . push_back ( MD -> grid () );
#else
#ifdef IGNORE_SMALL_MORSE
      if ( MD -> grid () -> size () > 20 )
#endif
      master_grid -> adjoin ( * MD -> grid () ); //SNOWBALL, QUADRATIC INEFFICIENCY. REPLACED WITH JOIN TECHNIQUE.
#endif
      if ( MD -> depth () > Min ) continue;
      // Amalgamate reachability information
#ifndef NO_REACHABILITY
      for ( unsigned int i = 0; i < N; ++ i ) {
        // "reaches" will tell us which children of MD are reachable from the ith child of MD
        const std::vector < unsigned int > & reaches = MD -> reachability () [ i ];
        //std::cout << "reaching info: " << reaches . size () << "\n";
        // We loop through the MorseGraph vertices corresponding to the ith child
        // and the jth child, and record the reachability.
        BOOST_FOREACH ( unsigned int j, reaches ) {
          if ( i == j ) continue;
          BOOST_FOREACH ( Vertex u,
                         temp [ MD -> children () [ i ] ] ) {
            BOOST_FOREACH ( Vertex v,
                           temp [ MD -> children () [ j ] ] ) {
              //std::cout << "Adding edge (" << u << ", " << v << ")\n";
              MG -> AddEdge ( u, v );
            }
          }
        }
      }
#endif
      // Aggregate temp data
      temp [ MD ] = std::vector < Vertex > ();
      
      if ( (MD -> depth () == Min) && (MD -> spurious () == false) ) {
        if ( MD -> grid () . get () == NULL ) {
          std::cout << "Error at ComputeMorseGraph.hpp line 261\n";
          abort ();
        }
        Vertex v = MG -> AddVertex ();
        MG -> grid ( v ) = MD -> grid ();
        if ( MG -> grid ( v ) . get () == NULL ) {
          std::cout << "Error at ComputeMorseGraph.hpp line 267\n";
          abort ();
        }
        temp [ MD ] . push_back ( v );
      } else if ( N > 0 ) {
        for ( unsigned int i = 0; i < N; ++ i ) {
          temp [ MD ] . insert (temp [ MD ] . begin (),
                                temp [ MD -> children () [ i ] ] . begin (),
                                temp [ MD -> children () [ i ] ] . end ());
          temp . erase ( MD -> children () [ i ] );
        }
      }
    } else {
      eulertourstack . push ( std::make_pair ( MD, childnum + 1 ) );
      eulertourstack . push ( std::make_pair ( MD -> children () [ childnum ], 0 ) );
    }
  }
#ifdef USE_SUCCINCT
  boost::shared_ptr<CompressedGrid> joinup ( Grid::join ( grids . begin (), grids . end () ) );
  master_grid -> assign ( * joinup );
#endif
}


template < class Map >
void Compute_Morse_Graph (MorseGraph * MG,
                          boost::shared_ptr<Grid> phase_space,
                          const Map & f,
                          const unsigned int Min, 
                          const unsigned int Max, 
                          const unsigned int Limit) {
  // Produce Morse Set Decomposition Hierarchy
  //std::cout << "COMPUTE MORSE GRAPH\n";
  //std::cout << "Initializing root MorseDecomposition\n";
  MorseDecomposition * root = new MorseDecomposition ( phase_space, 0 );
  //std::cout << "Calling ConstructMorseDecomposition\n";
  ConstructMorseDecomposition<Map> (root,
                                    f,
                                    Min,
                                    Max,
                                    Limit);
  //std::cout << "Calling ConstructMorseGraph\n";
  // Stitch together Morse Graph from Decomposition Hierarchy
  ConstructMorseGraph<Map> ( phase_space, MG, root, Min );
  // Free memory used in decomposition hierarchy
  delete root;
#ifdef MEMORYBOOKKEEPING

  std::cout << "Total Grid Memory (can be external) = " << max_grid_external_memory << "\n";
  std::cout << "Max Memory For Single Grid (must be internal)= " << max_grid_internal_memory << "\n";
  std::cout << "Max SCC Random Acces memory use (must be internal)= " << max_scc_memory_internal << "\n";
  std::cout << "Max SCC stack memory use (can be external memory) = " << max_scc_memory_external << "\n";
  std::cout << " ---- SUMMARY ---- \n";
  std::cout << "Internal Memory Requirement = " << max_grid_internal_memory + max_scc_memory_internal << "\n";
  std::cout << "External Memory Requirement = " << max_grid_external_memory + max_scc_memory_external << "\n";
  std::cout << "Max graph memory size (never stored, however) = " << max_graph_memory << "\n";
#endif
  //std::cout << "Returning from COMPUTE MORSE GRAPH\n";
}

#endif
