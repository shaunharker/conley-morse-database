/*
 *  Compute_Conley_Morse_Graph4.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_

#include <vector>
#include <ctime>
#include "boost/foreach.hpp"
#include "program/Configuration.h"
#include "data_structures/GraphTheory.h"
#include "algorithms/Homology.h"

#ifndef MIN_PHASE_SUBDIVISIONS
#warning Specify header parameter MIN_PHASE_SUBDIVISIONS; using default value 10
#define MIN_PHASE_SUBDIVISIONS 12
#endif

#ifndef MAX_PHASE_SUBDIVISIONS
#warning Specify header parameter MAX_PHASE_SUBDIVISIONS; using default value 10
#define MAX_PHASE_SUBDIVISIONS 15
#endif

#ifndef COMPLEXITY_LIMIT
#warning Specify header parameter COMPLEXITY_LIMIT; using default value 200000
#define COMPLEXITY_LIMIT 200000
#endif



template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map >
void Compute_Conley_Morse_Graph4 (Conley_Morse_Graph * CMG,
                                 Toplex * phase_space ,
                                  const typename Parameter_Toplex::Geometric_Description & parameter_box,
                                  bool should_compute_reachability,
                                  bool should_compute_conley_index) {
  clock_t start0, start;
  float map_time = 0;
  float scc_time = 0;
  float conley_time = 0;

  start0 = clock ();
  typedef std::vector<typename Toplex::Top_Cell> CellContainer;
  typedef CombinatorialMap<Toplex,CellContainer> Graph;
  std::cout << "Compute_Conley_Morse_Graph.\n";
  //std::cout << "[" << parameter_box . lower_bounds [ 0 ] << ", " << parameter_box . upper_bounds [ 0 ] << "] X";
  //std::cout << "[" << parameter_box . lower_bounds [ 1 ] << ", " << parameter_box . upper_bounds [ 1 ] << "]\n";

  // create map object
  Map interval_map ( parameter_box );
  // Initialize: subdivide and create initial morse set
  
  std::vector < CellContainer > morse_sets;
  /* scope */ {
    CellContainer initial_morse_set;
    std::insert_iterator<CellContainer> ii ( initial_morse_set, initial_morse_set . begin () );
    phase_space -> subdivide (ii, phase_space -> cover ( phase_space -> bounds () ) ); //TODO: more elegant, don't use deprecated interface
    morse_sets . push_back ( initial_morse_set );
  }                                    
  
  unsigned long complexity = phase_space -> size ();
  /* Main Loop: */
  bool subdivided = true;
  int depth = 1;
  while ( subdivided && ++ depth <= MAX_PHASE_SUBDIVISIONS ) {
    subdivided = false;
    // DEBUG
    //std::cout << "depth = " << depth << " and phase_space . size () = " << phase_space -> size () << "\n";
    // compute combinatorial map
    std::cout << "Computing directed graph...\n";
    start = clock ();
    Graph G = compute_combinatorial_map ( morse_sets, * phase_space, interval_map );
    map_time += (float)(clock() - start) / (float) CLOCKS_PER_SEC;
    // compute morse sets
    std::cout << "Computing morse sets...\n";
    morse_sets . clear ();
    start = clock ();
    compute_morse_sets <Conley_Morse_Graph,Toplex,CellContainer> ( &morse_sets, G );
    scc_time += (float)(clock() - start) / (float) CLOCKS_PER_SEC;
    // subdivide morse sets
    std::cout << "Subdividing morse sets...\n";
    BOOST_FOREACH ( CellContainer & morse_set, morse_sets ) {
      if ( depth <= MIN_PHASE_SUBDIVISIONS ||
           morse_set . size () * ( (1 << phase_space -> dimension () ) - 1) + complexity < COMPLEXITY_LIMIT ) {
        CellContainer new_morse_set;
        std::insert_iterator<CellContainer> ii ( new_morse_set, new_morse_set . begin () );
        phase_space -> subdivide (ii, morse_set);
        std::swap ( new_morse_set, morse_set );
        complexity = phase_space -> size ();
        subdivided = true;
      } else {
        morse_set . clear (); // toss it out (we'll recover it after this while loop)
      }
    } /* boost_foreach */
  } /* for */
  // Finalize: Create entire directed graph in memory and determine morse sets
  //std::cout << "phase_space . size () = " << phase_space -> size () << "\n";
  std::cout << "Final: Computing directed graph...\n";
  start = clock ();
  Graph G = compute_combinatorial_map<Toplex,Map,CellContainer> ( * phase_space , interval_map );
  map_time += (float)(clock() - start) / (float) CLOCKS_PER_SEC;

  std::cout << "Final: Computing morse sets...\n";
  morse_sets . clear ();
  start = clock ();
  compute_morse_sets<Conley_Morse_Graph,Toplex,CellContainer> ( &morse_sets, G, CMG );
  scc_time += (float)(clock() - start) / (float) CLOCKS_PER_SEC;

  std::cout << "Time to compute map = " << map_time << "\n";
  std::cout << "Time to compute scc = " << scc_time << "\n";
  
  // produce conley index
  start = clock ();
  typedef typename Conley_Morse_Graph::Vertex cmg_vertex_t;
  if ( should_compute_conley_index ) {
    // loop through vertices of conley morse graph
    BOOST_FOREACH (cmg_vertex_t v, CMG -> Vertices () ) {
      Conley_Index < Toplex, CellContainer, Map > ( & CMG -> ConleyIndex ( v ),
                                    * phase_space, 
                                    CMG -> CubeSet ( v ),
                                    interval_map );
    }
  }
  conley_time = (float)(clock() - start) / (float) CLOCKS_PER_SEC;

  if ( should_compute_conley_index) std::cout << "Time to compute Conley Index = " << conley_time << "\n";

  // TODO
  return;
} /* Compute_Conley_Morse_Graph */


#endif
