/*
 *  Compute_Conley_Morse_Graph3.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_

#include <vector>
#include <ctime>

#include "boost/foreach.hpp"

#include "program/Configuration.h"
#include "algorithms/Homology.h"
#include "data_structures/Directed_Graph.h"

#ifndef PHASE_SUBDIVISIONS
#warning Specify header parameter PHASE_SUBDIVISIONS; using default value 10
#define PHASE_SUBDIVISIONS 10
#endif

#ifndef COMPLEXITY_LIMIT
#warning Specify header parameter COMPLEXITY_LIMIT; using default value 200000
#define COMPLEXITY_LIMIT 200000
#endif



template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map >
void Compute_Conley_Morse_Graph3 (Conley_Morse_Graph * conley_morse_graph ,
                                 Toplex * phase_space ,
                                  const typename Parameter_Toplex::Geometric_Description & parameter_box,
                                  bool should_compute_conley_index,
                                  bool should_compute_reachability ) {
  clock_t start, stop;
  start = clock ();
  typedef DirectedGraph < Toplex > Graph;
  typedef typename Graph::Vertex vertex_t;
  //std::cout << "[" << parameter_box . lower_bounds [ 0 ] << ", " << parameter_box . upper_bounds [ 0 ] << "] X";
  //std::cout << "[" << parameter_box . lower_bounds [ 1 ] << ", " << parameter_box . upper_bounds [ 1 ] << "]\n";

  // create map object
  Map interval_map ( parameter_box );
  // Initialize: subdivide and create initial morse set
  std::vector < typename Toplex::Subset > morse_sets;
  typename Toplex::Subset initial_morse_set = phase_space -> subdivide ();
  morse_sets . push_back ( initial_morse_set );
  unsigned long complexity = phase_space -> size ();
  /* Main Loop: Inter */
  bool subdivided = true;
  int depth = 1;
  while ( subdivided && ++ depth <= PHASE_SUBDIVISIONS ) {
    subdivided = false;
    // DEBUG
    //std::cout << "depth = " << depth << " and phase_space . size () = " << phase_space -> size () << "\n";
    // compute combinatorial map
    //std::cout << "Computing directed graph...\n";
    Graph G = compute_directed_graph ( morse_sets, * phase_space , interval_map );
    // compute morse sets
    //std::cout << "Computing morse sets...\n";
    morse_sets . clear ();
    compute_morse_sets ( &morse_sets, G );
    // subdivide morse sets
    //std::cout << "Subdividing morse sets...\n";
    BOOST_FOREACH ( typename Toplex::Subset & morse_set, morse_sets ) {
      if ( morse_set . size () * ( (1 << phase_space -> dimension () ) - 1) + complexity < COMPLEXITY_LIMIT ) {
        morse_set = phase_space -> subdivide ( morse_set );
        complexity = phase_space -> size ();
        subdivided = true;
      } else {
        morse_set . clear (); // toss it out (we'll recover it after this while loop)
      }
    } /* boost_foreach */
  } /* for */
  // Finalize: Create entire directed graph in memory and determine morse sets
  //std::cout << "phase_space . size () = " << phase_space -> size () << "\n";
  //std::cout << "Computing directed graph...\n";
  Graph G = compute_directed_graph ( * phase_space , interval_map );
  //std::cout << "Computing morse sets...\n";
  morse_sets . clear ();
  compute_morse_sets ( &morse_sets, G );

  stop = clock ();
  //std::cout << "CCMG: Time to Compute Morse Sets = " << (float)(stop - start)/(float)CLOCKS_PER_SEC << "\n";
  
  /* Produce Morse Graph */
  start = clock ();
  // condense morse sets
  //std::cout << "Calculating Morse Graph...\n";
  std::vector< vertex_t > representatives;
  
  Graph H = collapseComponents < Toplex > ( G, morse_sets, /*output*/ representatives );
  
  // produce vertices on CMG and attach morse sets to them
  typedef typename Conley_Morse_Graph::Vertex cmg_vertex_t;
  std::map < vertex_t, cmg_vertex_t > translate;
  for (unsigned int s = 0; s < representatives . size (); ++ s ) {
    cmg_vertex_t new_vertex = conley_morse_graph -> AddVertex ();
    translate [ representatives [ s ] ] = new_vertex;
    conley_morse_graph -> CubeSet ( new_vertex ) = morse_sets [ s ];
  }
  // produce reachability information
  if ( should_compute_reachability ) {
    std::vector < std::vector < vertex_t > > reach_info;
    compute_reachability ( & reach_info, H, representatives );
    for (unsigned int s = 0; s < representatives . size (); ++ s ) {
      for (unsigned int t = 0; t < reach_info [ s ] . size (); ++ t ) {
        conley_morse_graph -> AddEdge ( translate [ representatives [ s ] ], 
                                       translate [ representatives [ t ] ] );
      }
    }
  }
  
  stop = clock ();
  //std::cout << "CCMG: Time to Compute Morse Graph = " << (float)(stop - start)/(float)CLOCKS_PER_SEC << "\n";
  
  // produce conley index
  if ( should_compute_conley_index ) {
    start = clock ();
    // loop through vertices of conley morse graph
    BOOST_FOREACH (cmg_vertex_t v, conley_morse_graph -> Vertices () ) {
      Conley_Index < Toplex, Map > ( & conley_morse_graph -> ConleyIndex ( v ),
                                    * phase_space, 
                                    conley_morse_graph -> CubeSet ( v ),
                                    interval_map );
    }
    stop = clock ();
    //std::cout << "CCMG: Time to Compute Conley Indices = " << (float)(stop - start)/(float)CLOCKS_PER_SEC << "\n";
  }
  // TODO
  return;
} /* Compute_Conley_Morse_Graph */


#endif
