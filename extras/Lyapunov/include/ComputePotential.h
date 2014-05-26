#ifndef LYAPUNOV_COMPUTEPOTENTIAL_H
#define LYAPUNOV_COMPUTEPOTENTIAL_H

// ComputePotential.h

#include <vector>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <limits>
#include <exception>
#include <algorithm>

#include "AdjacencyGrid.h"
#include "database/structures/TreeGrid.h"

uint64_t dijkstra_internal_memory_use = 0;
uint64_t dijkstra_priority_queue_memory_use = 0;

inline
void DijkstraDistance ( std::vector<double> * distance_to_set,
												const std::vector<bool> & set,
  											const AdjacencyGrid & ag ) {
	uint64_t V = ag . num_vertices ();
	uint64_t processed_count = 0;
	int percent = 0;
	std::priority_queue<DoubleGridPair> pq;

	std::vector<bool> processed ( V, false );
	std::vector<bool> duplicate ( V, false );
	dijkstra_internal_memory_use = (2L*V)/8L;

	for ( uint64_t ge = 0; ge < set . size (); ++ ge ) {
		if ( not set [ ge ] ) continue;
		uint64_t v = ag . GridToTree ( ge );
		pq . push ( DoubleGridPair ( 0.0, v ) );
	}

	while ( not pq . empty () ) {
		DoubleGridPair pair = pq . top ();
		pq . pop ();
		uint64_t v = pair . vertex;
		if ( processed [ v ] ) continue;
		// Process
		processed [ v ] = true;
		++ processed_count;
		//if ( processed_count < 10 ) {
		//	std::cout << processed_count << "\n";
		//}
		if ( (100*processed_count) / V > percent ) {
			percent = (100*processed_count) / V;
			std::cout << "\r             \r " << percent << "\%";
			std::cout . flush ();
		}
		if ( ag . isGridElement ( v ) ) {
			uint64_t ge = ag . TreeToGrid ( v );
			(*distance_to_set)[ge] = pair . distance;
		}
#if 0 
		std::vector<uint64_t> adjacencies = ag . adjacencies ( v );
		BOOST_FOREACH ( uint64_t u, adjacencies ) {
			//DEBUG
			if ( u >= processed . size () ) {
				std::cout << "Error condition. One of the adjacencies of vertex v=" << v << " is u=" << u << "\n";
				std::cout << "However there are only " << V << " vertices.\n";
				abort ();
			}
			// END DEBUG
			if ( processed [ u ] ) continue;
			double distance = ag . distance ( v, u );
			//std::cout << "Distance(" << v << ", " << u << ") = " << distance << "\n";
			DoubleGridPair new_pair ( pair . distance + distance, u );
			pq . push ( new_pair );
		}
#endif
		std::vector<DoubleGridPair> adjacencies = ag . adjacenciesWithDistance ( v );
#if 0
		// DEBUG
		boost::shared_ptr<Geo> center_geo = treegrid_ -> geometryOfTreeNode ( Tree::iterator ( v ) );
		RectGeo center_rect = * boost::dynamic_pointer_cast<RectGeo> ( center_geo );

		std::cout << "Geometry of center grid element = " << center_rect << "\n";
		BOOST_FOREACH ( DoubleGridPair new_pair, adjacencies ) {

		}
#endif
		// END DEBUG
		BOOST_FOREACH ( DoubleGridPair new_pair, adjacencies ) {
			uint64_t u = new_pair . vertex;
			double distance = new_pair . distance;
			if ( processed [ u ] ) continue;
			new_pair . distance += pair . distance;
			pq . push ( new_pair );
		}
		dijkstra_priority_queue_memory_use = std::max( dijkstra_priority_queue_memory_use,
																									pq . size () * (uint64_t) sizeof ( DoubleGridPair ) );
		// Eliminate Duplicates
		if ( pq . size () > 2*V ) {
			std::queue<DoubleGridPair> temp; // stack or queue more efficient here?
			while ( not pq . empty () ) {
				DoubleGridPair top_pair = pq . top ();
				pq . pop ();
				if ( processed [ top_pair . vertex ] ) continue;
				if ( duplicate [ top_pair . vertex ] ) continue;
				duplicate [ top_pair . vertex ] = true;
				temp . push ( top_pair );
			}
			while ( not temp . empty () ) {
				DoubleGridPair top_pair = temp . front ();
				temp . pop ();
				pq . push ( top_pair );
				duplicate [ top_pair . vertex ] = false;
			}
		}
	}
	std::cout << "\r                \r";
	std::cout . flush ();
}

inline
void ComputePotential ( double * minimum_distance,
												std::vector<double> * potential,
												const std::vector<bool> & attractor,
  											const std::vector<bool> & repeller,
  											boost::shared_ptr<const TreeGrid> grid) {

	AdjacencyGrid ag ( grid );

	//ag . test ();

	double infinity = std::numeric_limits<double>::infinity();
	uint64_t N = attractor . size ();

	std::vector<double> distance_to_attractor ( N, infinity );
	std::vector<double> distance_to_repeller ( N, infinity );

	DijkstraDistance ( &distance_to_attractor, attractor, ag );
	DijkstraDistance ( &distance_to_repeller, repeller, ag );

	double minimum_distance_from_attractor_to_repeller = infinity;
	double minimum_distance_from_repeller_to_attractor = infinity;

	for ( uint64_t ge = 0; ge < N; ++ ge ) {
		if ( attractor [ ge ] ) {
			minimum_distance_from_attractor_to_repeller = 
				std::min ( minimum_distance_from_attractor_to_repeller, distance_to_repeller [ ge ] );
		}
		if ( repeller [ ge ] ) {
			minimum_distance_from_repeller_to_attractor = 
				std::min ( minimum_distance_from_repeller_to_attractor, distance_to_attractor [ ge ] );
		}
	}
	if ( std::abs(minimum_distance_from_repeller_to_attractor - minimum_distance_from_attractor_to_repeller) /
	      ( 1.0 + std::abs ( minimum_distance_from_repeller_to_attractor ) ) > 1.0e-9 ) {
		std::cout << minimum_distance_from_repeller_to_attractor << " != " << minimum_distance_from_attractor_to_repeller << "\n";
		throw std::logic_error ( "ComputePotential. Lack of symmetry in distance calculation.\n");
	} else {
		*minimum_distance = minimum_distance_from_repeller_to_attractor;
	}

	potential -> resize ( N, 0.0 );
	for ( uint64_t ge = 0; ge < N; ++ ge ) {
		(*potential)[ge] = 
			distance_to_attractor[ge]/(distance_to_attractor[ge]+distance_to_repeller[ge]);
	}
}

#endif

  	