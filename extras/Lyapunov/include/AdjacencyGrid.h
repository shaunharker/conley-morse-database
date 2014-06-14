#ifndef LYAPUNOV_ADJACENCYGRID_H
#define LYAPUNOV_ADJACENCYGRID_H

#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include "boost/foreach.hpp"
#include <vector>
#include <exception>
#include <cstdlib>

#include "database/structures/Grid.h"
#include "database/structures/TreeGrid.h"
#include "database/structures/Tree.h"


class DoubleGridPair {
public:
	double distance;
	uint64_t vertex;
	DoubleGridPair ( void ) {}
	DoubleGridPair ( double x, uint64_t y ) : distance(x), vertex(y) {}
	// backwards
	bool operator < ( const DoubleGridPair & rhs ) const {
		if ( distance > rhs . distance ) return true;
		return false;
	}

};

class AdjacencyGrid {
private:
	boost::shared_ptr<const TreeGrid> treegrid_;
public:
	AdjacencyGrid ( boost::shared_ptr<const TreeGrid> ptr );
	std::vector<DoubleGridPair> adjacenciesWithDistance ( uint64_t v ) const;
	std::vector<uint64_t> adjacencies ( uint64_t v ) const;
	uint64_t num_vertices ( void ) const;
	double distance ( uint64_t u, uint64_t w ) const;
	uint64_t GridToTree ( uint64_t ge ) const;
  uint64_t TreeToGrid ( uint64_t v ) const;
  bool isGridElement ( uint64_t v ) const;

  void test ( void ) const;
};

inline
AdjacencyGrid::AdjacencyGrid ( boost::shared_ptr<const TreeGrid> ptr ) : treegrid_(ptr) {}

inline
std::vector<DoubleGridPair> AdjacencyGrid::adjacenciesWithDistance ( uint64_t v ) const {
	// Output variable
	std::vector<DoubleGridPair> result;

	// Variables for Tree algorithm
	int D = treegrid_ -> dimension ();
	const Tree & tree = treegrid_ -> tree ();
	Tree::iterator start ( v );
	Tree::iterator it;
	int start_split_dim = tree . depth ( start ) % D;

	// Get geometry of tree node
	boost::shared_ptr<Geo> geo = treegrid_ -> geometryOfTreeNode ( start );
	RectGeo rect = * boost::dynamic_pointer_cast<RectGeo> ( geo );	
	
	// Fetch parent
	it = tree . parent ( start );
	if ( it != tree . end () ) {
		int parent_split_dim = start_split_dim - 1;
		if ( parent_split_dim < 0 ) parent_split_dim = D - 1;
		double distance = 
			rect . upper_bounds [ parent_split_dim ] - rect . lower_bounds [ parent_split_dim ];
		distance /= 2.0;
		result . push_back ( DoubleGridPair ( distance, *it ) );
	}

	// Fetch children
	it = tree . left ( start );
	if ( it != tree . end () ) {
		double distance = 
			rect . upper_bounds [ start_split_dim ] - rect . lower_bounds [ start_split_dim ];
		distance /= 4.0;
		result . push_back ( DoubleGridPair ( distance, *it ) );
	}
	it = tree . right ( start );
	if ( it != tree . end () ) {
		double distance = 
			rect . upper_bounds [ start_split_dim ] - rect . lower_bounds [ start_split_dim ];
		distance /= 4.0;
		result . push_back ( DoubleGridPair ( distance, *it ) );
	}
	// Fetch congruent neighbors that share codim-1 face
	for ( int d = 0; d < D; ++ d ) {
		double distance = 
			rect . upper_bounds [ d ] - rect . lower_bounds [ d ];
		for ( int dir = 0; dir <= 1; ++ dir ) {
			it = start;
			int split_dim = start_split_dim;
			std::vector<bool> path;
			bool found = true;

			while ( 1 ) {
				if ( it == tree . begin () ) {
					found = false;
					break;
				}
				Tree::iterator parent = tree . parent ( it );
				int child_dir = ( tree . left ( parent ) == it ) ? 0 : 1;
				-- split_dim;
				if ( split_dim < 0 ) split_dim = D - 1;
				it = parent;

				if ( split_dim != d ) {
					path . push_back ( child_dir );
					continue;
				}
				path . push_back ( 1 - child_dir );
				if ( child_dir == dir ) break;
			}
			if ( not found ) continue;
			for ( int i = path.size() - 1; i >= 0; -- i ) {
				if ( path [ i ] ) {
					it = tree . right ( it );
				} else {
					it = tree . left ( it );
				}
				if ( it == tree . end () ) {
					found = false;
					break;
				}
			}
			if ( found ) {
				result . push_back ( DoubleGridPair ( distance, *it ) );
			}
		}
	}
	return result;
}

inline
std::vector<uint64_t> AdjacencyGrid::adjacencies ( uint64_t v ) const {
	// Approach:
	//  Compute geometry of intermediate tree node
	//  Cover geometry with grid elements (tree leaves)
	//  Convert grid elements back to tree iterators
	//  Return list of all parents.
	// TODO: Prevent set from growing too big (switch method if gets too large)
	boost::unordered_set<uint64_t> set_result;
	const Tree & tree = treegrid_ -> tree ();
	
	Tree::iterator it ( v );
	boost::shared_ptr<Geo> geo = treegrid_ -> geometryOfTreeNode ( it );
	std::vector<Grid::GridElement> leaves = treegrid_ -> cover ( * geo );
	set_result . insert ( * tree . begin () );
	BOOST_FOREACH ( Grid::GridElement ge, leaves ) {
		uint64_t u = GridToTree ( ge );
		while ( set_result . count ( u ) == 0 ) {
			set_result . insert ( u );
			u = * tree . parent ( Tree::iterator(u) );
		}
	}
	std::vector<uint64_t> result;
	BOOST_FOREACH ( uint64_t u, set_result ) {
		result . push_back ( u );
	}
	return result;
}

inline
uint64_t AdjacencyGrid::num_vertices ( void ) const {
	return treegrid_ -> tree () . size ();
}

// distance
// Return Manhattan distance between centers of rectangles
// corresponding to two tree nodes
inline
double AdjacencyGrid::distance ( uint64_t u, uint64_t v ) const {
	Tree::iterator it1 ( u );
	boost::shared_ptr<Geo> geo1 = treegrid_ -> geometryOfTreeNode ( it1 );

	Tree::iterator it2 ( v );
	boost::shared_ptr<Geo> geo2 = treegrid_ -> geometryOfTreeNode ( it2 );

	RectGeo rect1 = * boost::dynamic_pointer_cast<RectGeo> ( geo1 );
	RectGeo rect2 = * boost::dynamic_pointer_cast<RectGeo> ( geo2 );

	double result = 0.0;

	int D = rect1 . dimension ();
	for ( int d = 0; d < D; ++ d ) {
		double mid1 = (rect1 . lower_bounds [ d ] + rect1 . upper_bounds [ d ])/2.0;
		double mid2 = (rect2 . lower_bounds [ d ] + rect2 . upper_bounds [ d ])/2.0;
		result += std::abs ( mid1 - mid2 );
	}
	return result;
}

inline
uint64_t AdjacencyGrid::GridToTree ( uint64_t ge ) const {
	return * (treegrid_ -> GridToTree ( TreeGrid::iterator (ge) ));
}

inline
uint64_t AdjacencyGrid::TreeToGrid ( uint64_t v ) const {
	return * (treegrid_ -> TreeToGrid ( Grid::iterator (v) ));
}

inline
bool AdjacencyGrid::isGridElement ( uint64_t v ) const {
	if ( TreeToGrid(v) == num_vertices () ) return false;
	return true;
}

inline void 
AdjacencyGrid::test ( void ) const {
	// not efficient, meant as a debug routine
	uint64_t N = num_vertices ();
	double biggest_asymmetry = 0.0;
	for ( uint64_t u = 0; u < N; ++ u ) {
		std::vector<DoubleGridPair> adj_u = adjacenciesWithDistance ( u );
		BOOST_FOREACH ( const DoubleGridPair & pair_u, adj_u ) {
			uint64_t v = pair_u . vertex;
			bool found = false;
			std::vector<DoubleGridPair> adj_v = adjacenciesWithDistance ( v );
			BOOST_FOREACH ( const DoubleGridPair & pair_v, adj_v ) {
				if ( pair_v . vertex != u ) continue;
				found = true;
				biggest_asymmetry = std::max(biggest_asymmetry, std::abs(pair_u.distance - pair_v.distance ));
			}
			if ( found == false ) {
				throw std::logic_error ( "AdjacencyGrid does not yield a symmetric graph!\n" );
			}
		}
	}
	std::cout << "Biggest asymmetry found in AdjacencyGrid: " << biggest_asymmetry << "\n";
}

#endif
