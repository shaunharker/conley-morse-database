
#ifndef COMPUTELYAPUNOV_H
#define COMPUTELYAPUNOV_H

#include <iostream>
#include <vector>
#include <queue>
#include <ctime>

#include "database/structures/TreeGrid.h"
#include "database/structures/MapGraph.h"
#include "database/algorithms/GraphTheory.h"
#include "database/maps/Map.h"

#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"

#include "Draw.h"

#include "ComputePotential.h"

inline
void forward ( std::vector<bool> * output,
              const MapGraph & mapgraph,
              const std::deque < Grid::size_type > & topological_sort, 
              const std::deque < Grid::size_type > & SCC_root ) {
  std::vector<bool> & set = *output;
  size_t N = mapgraph . num_vertices ();
  typedef MapGraph::Vertex Vertex;
  for ( int i = 0; i < N; ++ i ) {
    Vertex v = topological_sort [ i ];
    set [ v ] = set [ SCC_root [ v ] ];
    if ( not set [ v ] ) continue;
    std::vector<Vertex> adj = mapgraph . adjacencies ( v );
    BOOST_FOREACH ( Vertex u, adj ) {
      set [ SCC_root [ u ] ] = true;
    }
  }
}

inline
void backward ( std::vector<bool> * output,
                const MapGraph & mapgraph,
                const std::deque < Grid::size_type > & topological_sort, 
                const std::deque < Grid::size_type > & SCC_root ) {
  std::vector<bool> & set = *output;
  size_t N = mapgraph . num_vertices ();
  typedef MapGraph::Vertex Vertex;
  for ( int i = N-1; i >= 0; -- i ) {
    Vertex v = topological_sort [ i ];
    Vertex r = SCC_root [ v ];
    if ( set [ r ] ) continue;
    std::vector<Vertex> adj = mapgraph . adjacencies ( v );
    BOOST_FOREACH ( Vertex u, adj ) {
      if ( set [ SCC_root [ u ] ] ) {
        set [ r ] = true;
        break;
      }
    }
  }
  for ( int i = 0; i < N; ++ i ) {
    set [ i ] = set [ SCC_root [ i ] ];
  }
}

inline std::vector<double>
ComputeLyapunov ( boost::shared_ptr<TreeGrid> grid,
									boost::shared_ptr<const Map> map ) {

  std::cout << "Computing Lyapunov function.\n";
  clock_t start_time = clock ();

  std::cout << "The grid X has " << grid -> size () << " grid elements.\n";
  // Memory usage monitoring
  uint64_t grid_memory_use = grid -> memory ();
  uint64_t scc_root_memory_use = 0;
  uint64_t components_memory_use = 0;
  uint64_t topological_sort_memory_use = 0;
  uint64_t global_lyapunov_result_memory_use = 0;
  uint64_t local_lyapunov_memory_use = 0; 
  uint64_t attractor_memory_use = 0;
  uint64_t repeller_memory_use = 0;
  uint64_t potential_memory_use = 0;
  // global: uint64_t dijkstra_internal_memory_use = 0;
  // global: uint64_t dijkstra_priority_queue_memory_use = 0;
  // global: uint64_t max_scc_memory_internal = 0;
  // global: uint64_t max_scc_memory_external = 0;

	// Obtain directed graph (mapgraph)  
  std::cout << "Realizing combinatorial map F on X as a directed graph G.\n";
  typedef MapGraph::Vertex Vertex;
  MapGraph mapgraph ( grid, map );
  size_t N = mapgraph . num_vertices ();

  // Produce Strong Components and (generalized) topological sort
  std::cout << "Computing Strong Components of G.\n";
  std::vector < std::deque < Grid::GridElement > > components;
  std::deque < Grid::size_type > topological_sort;
  std::deque < Grid::size_type > reversed_topological_sort;


  std::deque < Grid::size_type > SCC_root;
  computeStrongComponents ( &components, mapgraph, &reversed_topological_sort, &SCC_root );

  // Update memory use.
  scc_root_memory_use += SCC_root . size () * sizeof ( Grid::size_type );
  for ( int morse_set = 0; morse_set < components . size (); ++ morse_set ) {
    components_memory_use += 
      components [ morse_set ] . size () * (uint64_t) sizeof ( Grid::size_type );
  }
  topological_sort_memory_use += reversed_topological_sort . size () * sizeof ( Grid::size_type );
  global_lyapunov_result_memory_use = N * (uint64_t) sizeof ( double );
  attractor_memory_use = N / 8;
  repeller_memory_use = N / 8;
  potential_memory_use = ((uint64_t) sizeof(double))*N;
  local_lyapunov_memory_use = ((uint64_t)sizeof(double))*N;


  // Produce topological_sort from reversed_topological_sort
  while ( not reversed_topological_sort . empty () ) {
    topological_sort . push_back ( reversed_topological_sort . back () );
    reversed_topological_sort . pop_back ();
  }

	//draw2DMorseSets ( components, grid );

  // Compute Lyanpunov Function
  std::vector<double> global_lyapunov ( N, 0.0 );

  double total_weight = 0.0;

  //// Compute Maximal Invariant Set
  //std::vector<bool> maximal_invariant_set ( N, false );
 	//std::vector<bool> down ( N, false );
  //std::vector<bool> up (N, false );
  //
  // Seed "up" and "down" sets for maximal invariant set computation
  //for ( int morse_set = 0; morse_set < components . size (); ++ morse_set ) {
  //	typedef MapGraph::Vertex Vertex;
  //	BOOST_FOREACH ( Vertex v, components [ morse_set ] ) {
  //		down [ v ] = true;
  //		up [ v ] = true;
  //	}
	//}
	//// Down set
  //forward ( &down, mapgraph, topological_sort, SCC_root );
  //std::cout << "Down Set\n";
  //draw2Dimage ( down, grid );
  //
  //// Up set
  //backward ( &up, mapgraph, topological_sort, SCC_root );
	//std::cout << "Up Set\n";
  //draw2Dimage ( up , grid );
  //
  //// M.I.S.
  //  std::cout << "Maximal Invariant Set\n";
	//for ( int i = 0; i < N; ++ i ) {
  //	maximal_invariant_set [ i ] = up[i] && down[i];
  //}
  //draw2Dimage ( maximal_invariant_set, grid );
	
	std::cout << "There were " << components . size () << " combinatorial Morse sets found.\n";
  // Loop through Morse Sets
  for ( int morse_set = 0; morse_set < components . size (); ++ morse_set ) {
  	 std::cout << "Now processing Morse Set M" << morse_set << ":\n";


  	std::vector<bool> attractor ( N, false );
  	std::vector<bool> repeller (N, false );
  	std::vector<double> potential ( N );
  	std::vector<double> lyapunov ( N, 0.0 );

 
  	// Compute Attractor

  	// Seed attractor with morse set.
  	std::cout << "Computing combinatorial attractor A" << morse_set 
              << ", the omega limit set of M" << morse_set << ".\n";

  	BOOST_FOREACH ( Vertex v, components [ morse_set ] ) {
  		attractor [ v ] = true;
  	}

  	// debug
  	//std::cout << "Morse Set\n";
  	//draw2Dimage ( attractor, grid );
  	// end debug
  	
  	// Use "forward" algorithm to compute attractor
  	forward ( &attractor, mapgraph, topological_sort, SCC_root );
    /*
  	for ( int i = 0; i < N; ++ i ) {
  		attractor [ i ] = attractor [ i ] && maximal_invariant_set [ i ];
		}
    */
  	
  	// debug
  	//std::cout << "Showing Attractor\n";
  	//draw2Dimage ( attractor, grid );
		// end debug

  	// Compute Repeller
    std::cout << "Computing combinatorial dual repeller R" << morse_set 
              << ", the alpha limit set of X\\A" << morse_set << ".\n";
  	// seed Repeller with every Morse set not in attractor
  	for ( int ms = 0; ms < components . size (); ++ ms ) {
  		BOOST_FOREACH ( Vertex v, components [ ms ] ) {
  			if ( attractor [ v ] ) break;
  			repeller [ v ] = true;
  		}
  	}
  	// Build dual repeller with "backward" algorithm
  	backward ( &repeller, mapgraph, topological_sort, SCC_root );

    /*
  	for ( int i = 0; i < N; ++ i ) {
  		repeller [ i ] = repeller [ i ] && maximal_invariant_set [ i ];
		}
    */

    //std::cout << "repeller\n";
  	//draw2Dimage ( repeller, grid );

  	// Draw attractor/repeller pair
  	//draw2DAttractorRepeller ( maximal_invariant_set, attractor, repeller, grid );

    // Skip step if empty repeller
    bool empty_repeller = true;
    for ( int i = 0; i < N; ++ i ) {
      if ( repeller [ i ] == true ) {
        empty_repeller = false;
        break;
      }
    }
    if ( empty_repeller ) { 
      std::cout << "Skipping computation of a local Lyapunov function "
                   "for this combinatorial Morse set "
                   "because the dual repeller is empty.\n";
      continue;
    }

		// Compute Potential
		std::cout << "Computing Distance Potential v via Dijkstra's algorithm.\n";

    double weight;

		ComputePotential ( &weight,
                       &potential, 
											 attractor,
											 repeller,
											 grid );

		//draw2Dimage ( potential, grid );
		// Update potential so it is "potential star"
		std::cout << "Computing Distance Potential Star v^*.\n";

  	for ( int i = N-1; i >= 0; -- i ) {
  		Vertex v = topological_sort [ i ];
			std::vector<Vertex> adj = mapgraph . adjacencies ( v );
			double max_potential = potential [ v ];
			BOOST_FOREACH ( Vertex u, adj ) {
				max_potential = std::max ( max_potential, potential [ u ] );
			}
			potential [ v ] = max_potential;
		}

    // DEBUG
    /*
    for ( int i = N-1; i >= 0; -- i ) {
      Vertex v = topological_sort [ i ];
      std::vector<Vertex> adj = mapgraph . adjacencies ( v );
      BOOST_FOREACH ( Vertex u, adj ) {
        if ( potential [ u ] > potential [ v ] ) {
          std::cout << "There is a bug.\n";
          abort ();
        }
      }
    }
    */
    // END DEBUG
		
    //draw2Dimage ( potential, grid );

		// Compute Lyanpunov
		std::cout << "Computing local Lyapunov function for " 
                 "attractor/dual repeller pair " << morse_set << "\n";

		 for ( int i = N-1; i >= 0; -- i ) {
		 	Vertex v = topological_sort [ i ];

		 	if ( repeller [ v ] ) {
		 		lyapunov [ v ] = 1.0;
		 		continue;
		 	}
		 	if ( attractor [ v ] ) {
		 		lyapunov [ v ] = 0.0;
		 		continue;
		 	}
			std::vector<Vertex> adj = mapgraph . adjacencies ( v );
			double max_lyapunov = 0.0;
			BOOST_FOREACH ( Vertex u, adj ) {
				max_lyapunov = std::max ( max_lyapunov, lyapunov [ u ] );
			}
			lyapunov [ v ] = 0.5 * potential [ v ] + 0.5 * max_lyapunov;

			// debug
			if ( lyapunov [v ] < 0.0 || lyapunov [ v ] > 1.0 ) {
				std::cout << "lyapunov[" << v << "] = " << lyapunov[v] << "\n";
			}
			// end debug
		}
		
    //std::cout << "Showing Lyapunov Summand \n";
		//draw2Dimage ( lyapunov, grid );

		//std::cout << "Previous averaged Lyapunov.\n";
		//draw2Dimage ( global_lyapunov, grid );

    std::cout << "Summing local Lyapunov function into global Lyapunov function.\n";
		for ( int i = 0; i < N; ++ i ) {
			global_lyapunov [ i ] += weight * lyapunov [ i ];
		}

    total_weight += weight;

		//std::cout << "Current averaged Lyapunov.\n";
		//draw2Dimage ( global_lyapunov, grid );
  }
  
  std::cout << "Local Lyapunov functions all computed. Renormalizing global Lyapunov function.\n";
  for ( int i = 0; i < N; ++ i ) {
  	global_lyapunov [ i ] /= total_weight;
  	//if ( not maximal_invariant_set [ i ] ) {
  	//	global_lyapunov [ i ] = 0.0;
  	//}
	}

  std::cout << "Lyapunov function calculation complete.\n";
  clock_t stop_time = clock ();

  if ( grid -> dimension () == 2 ) draw2Dimage ( global_lyapunov, grid );

  // generate file with memory usage statistics
  std::ofstream stats_file ( "lyapunov_memory_statistics.txt" );
  stats_file << "Lyapunov calculation resource usage statistics.\n";
  stats_file << "The grid had " << N << " grid elements.\n";
  stats_file << "The computation took " << ((double)(stop_time-start_time)/(double)CLOCKS_PER_SEC)
             << " seconds.\n";
  stats_file << "All memory figures are in bytes:\n";
  stats_file << "grid_memory_use = " << grid_memory_use << "\n";
  stats_file << "max_graph_memory = " << max_graph_memory << "\n";
  stats_file << "max_scc_memory_internal = " << max_scc_memory_internal << "\n";
  stats_file << "max_scc_memory_external = " << max_scc_memory_external << "\n";
  stats_file << "scc_root_memory_use = " << scc_root_memory_use << "\n";
  stats_file << "components_memory_use = " << components_memory_use << "\n";
  stats_file << "topological_sort_memory_use = " << topological_sort_memory_use << "\n";
  stats_file << "global_lyapunov_result_memory_use = " << global_lyapunov_result_memory_use << "\n";
  stats_file << "local_lyapunov_memory_use = " << local_lyapunov_memory_use << "\n";
  stats_file << "attractor_memory_use = " << attractor_memory_use << "\n";
  stats_file << "repeller_memory_use = " << repeller_memory_use << "\n";
  stats_file << "potential_memory_use = " << potential_memory_use << "\n";
  stats_file << "dijkstra_internal_memory_use = " << dijkstra_internal_memory_use << "\n";
  stats_file << "dijkstra_priority_queue_memory_use = " << dijkstra_priority_queue_memory_use << "\n";
 
  stats_file . close ();

  return global_lyapunov;
}

#endif
