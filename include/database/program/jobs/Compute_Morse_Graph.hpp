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
    // Check subdivision condition. (condition true -> dont subdivide)
    if ( depth >= Min && ( set . size () > Limit || depth >= Max ) ) {
      subdivided = false;
      spurious = false;
#ifdef DO_CONLEY_INDEX
      if ( depth == Min ) {
        using namespace chomp;
        ConleyIndex_t output;
        ConleyIndex ( &output,
                   *phase_space, 
                   set,
                   interval_map );
      }
#endif
      CMG_VERBOSE_PRINT("Returning from depth " << depth << ".\n");

      return;
    }
    #ifdef DO_CONLEY_INDEX
    if ( (depth > 0) && (depth < Min) ) {
      using namespace chomp;
      ConleyIndex_t output;
      ConleyIndex ( &output,
                   *phase_space, 
                   set,
                   interval_map );
    }
    #endif
    // Subdivide
    subdivided = true;
    spurious = true; // This may be changed below.
    CellContainer newset = set;
    subdivide ( phase_space, newset );
    // Create children.
    std::vector < CellContainer > morse_sets;
    Graph G ( newset, * phase_space, interval_map );
    // PUT STUFF HERE
#ifdef CMG_VISUALIZE
    
    // Construct uniform measure
    boost::unordered_map < typename Toplex::Top_Cell, double > measure;
    double uniform = 1.0 / (double) newset . size ();
    BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
      measure [ t ] = uniform;
    }
    
    // Random "bleed-test" choice
#if 0
    size_t N = newset . size ();
    size_t i = 0;
    size_t choice = rand () % N;
    BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
      measure [ t ] = 0.0;//uniform;
      if ( i ++ == choice ) measure [ t ] = 1.0;
    }
#endif
    
    // Iterate through out-edges several times
    CImgDisplay display(512,512);
    //display . wait ();
    for ( int iterate = 0; iterate < 10; ++ iterate ) {
      std::cout << "iterate " << iterate << "\n";
      boost::unordered_map < typename Toplex::Top_Cell, double > newmeasure;
      BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
        chomp::Rect box = phase_space -> geometry ( t );
        std::vector < typename Toplex::Top_Cell > cover_of_box;
        std::insert_iterator < std::vector < typename Toplex::Top_Cell > > ii ( cover_of_box, cover_of_box . begin () );
        phase_space -> cover ( ii, box );
        std::vector < typename Toplex::Top_Cell > restricted_cover_of_box; // restrict to newset
        BOOST_FOREACH ( typename Toplex::Top_Cell s, cover_of_box ) {
          if ( measure . count ( s ) ) restricted_cover_of_box . push_back ( s );
        }
        double factor = 1.0 / (double) restricted_cover_of_box . size ();
        BOOST_FOREACH ( typename Toplex::Top_Cell s, restricted_cover_of_box ) {
          newmeasure [ s ] += measure [ t ] * factor;
        }
      }
      std::swap ( measure, newmeasure );
   // }
    
    // Get bounds.
    chomp::Rect outerbounds = phase_space -> bounds ();
    chomp::Rect bounds ( 2 );
    bounds . lower_bounds [ 0 ] = outerbounds . upper_bounds [ 0 ];
    bounds . upper_bounds [ 0 ] = outerbounds . lower_bounds [ 0 ];
    bounds . lower_bounds [ 1 ] = outerbounds . upper_bounds [ 1 ];
    bounds . upper_bounds [ 1 ] = outerbounds . lower_bounds [ 1 ];
    BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
      chomp::Rect box = phase_space -> geometry ( t );
      bounds . lower_bounds [ 0 ] = std::min ( bounds . lower_bounds [ 0 ], box . lower_bounds [ 0 ]);
      bounds . upper_bounds [ 0 ] = std::max ( bounds . upper_bounds [ 0 ], box . upper_bounds [ 0 ]);
      bounds . lower_bounds [ 1 ] = std::min ( bounds . lower_bounds [ 1 ], box . lower_bounds [ 1 ]);
      bounds . upper_bounds [ 1 ] = std::max ( bounds . upper_bounds [ 1 ], box . upper_bounds [ 1 ]);
    }
    
    // Draw Grid Elements
    int Width = 512;//2560;//1024;
    int Height = 512;//1600;//1024;
    CImg<unsigned char> visual(Width,Height,1,3,0);
    typedef std::pair < typename Toplex::Top_Cell, double > value_t;
    double max_bright = 0.0;
    BOOST_FOREACH ( value_t val, measure ) {
      max_bright = std::max ( max_bright, val . second );
    }
    BOOST_FOREACH ( value_t val, measure ) {
      typename Toplex::Top_Cell t = val . first;
      double brightness = val . second;
      chomp::Rect box = phase_space -> geometry ( t );
      double x0, x1, y0, y1;
      x0 = ((double)Width) * (box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ]) / ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] );
      x1 = ((double)Width) * (box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ]) / ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] );
      y0 = ((double)Height) * (box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ]) / ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] );
      y1 = ((double)Height) * (box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ]) / ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] );
      int X0, X1, Y0, Y1;
      X0 = x0; X1 = x1; Y0 = y0; Y1 = y1;
      if ( X1 == X0 ) ++ X1;
      if ( Y1 == Y0 ) ++ Y1;
      if ( X0 > Width ) X0 = Width;
      if ( X1 > Width ) X1 = Width;
      if ( Y0 > Height ) Y0 = Height;
      if ( Y1 > Height ) Y1 = Height;
      for ( int i = X0; i < X1; ++ i ) {
        for ( int j = Y0; j < Y1; ++ j ) {
          visual ( i, Height-j-1, 0, 1 ) = 50;//255;
          visual ( i, Height-j-1, 0, 0 ) = (unsigned char) ( 255.0*(brightness/max_bright) );
        }
      }
    }
      display = visual;
    } // iterates
    
    //char ch;
    //std::cout << "Press a key then press enter: ";
    //std::cin  >> ch;
    //CImgDisplay display(visual,"Intensity profile");
    //display . wait ();
    
#endif
    
    computeMorseSetsAndReachability <Graph,CellContainer>
    ( &morse_sets, &reachability, G );
#ifdef CMG_VERBOSE 
    if ( morse_sets . size () > 1 ) {
      std::cout << "Splits into " << morse_sets . size () << " morse sets.\n";
    }
    std::cout << "Sizes of children: ";
    BOOST_FOREACH ( const CellContainer & morse_set, morse_sets ) {
      std::cout << morse_set . size () << " "; 
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
#ifdef MONOTONICSUBDIVISIONPROPERTY
    if ( not spurious )
#endif
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
  
#if 0
  std::cout << "c = (" << interval_map . c . lower () << ", " << interval_map . c . upper () << ")\n";
  std::cout << "phi = (" << interval_map . phi . lower () << ", " << interval_map . phi . upper () << ")\n";

  std::cout << "phase_space bounds = " << phase_space -> bounds () << "\n";
  std::cout << "phase_space tree size = " << phase_space -> tree_size () << "\n";
  std::cout << "phase_space periodic:" << phase_space -> periodic () . size () << "\n";
  for ( int d = 0; d < phase_space -> periodic () . size (); ++ d ) {
    std::cout << "PERIODIC[" << d << "] = " << (phase_space -> periodic ()[d]? "true":"false") << "\n";
  }

  std::cout << "Min = " << Min << "\n";
  std::cout << "Max = " << Max << "\n";
  std::cout << "Limit = " << Limit << "\n";
#endif
  
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
  
#ifndef MONOTONICSUBDIVISIONPROPERTY
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
#else
  typedef MapGraph<Toplex,Map,CellContainer> Graph;
  Graph G ( * phase_space, interval_map );
  std::vector < CellContainer > morse_sets;
  compute_morse_sets<Morse_Graph,Graph,CellContainer> ( &morse_sets, G, MG );
#endif
}

#endif
