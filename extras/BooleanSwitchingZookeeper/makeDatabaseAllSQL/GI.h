
#include <string>
#include <vector>
#include <algorithm>
#include <boost/foreach.hpp>

class VertexInfo {
public:
	uint64_t number_in_edges;
	uint64_t number_out_edges;
	uint64_t number_of_ancestors;
	uint64_t number_of_descendants;
	std::string label;
	bool operator == (VertexInfo const& b) const {
  	if ( number_in_edges != b . number_in_edges ) return false;
  	if ( number_out_edges != b . number_out_edges ) return false;
  	if ( number_of_ancestors != b . number_of_ancestors ) return false;
  	if ( number_of_descendants != b . number_of_descendants ) return false;
  	if ( label != b . label ) return false;
  	return true;
  }
  bool operator != (VertexInfo const& b) const {
  	return not (*this == b);
  }
  bool operator < ( VertexInfo const& b) const {
  	if ( number_in_edges < b . number_in_edges ) return true;
  	if ( number_in_edges > b . number_in_edges ) return false;

  	if ( number_out_edges < b . number_out_edges ) return true;
  	if ( number_out_edges > b . number_out_edges ) return false;

  	if ( number_of_ancestors < b . number_of_ancestors ) return true;
  	if ( number_of_ancestors > b . number_of_ancestors ) return false;

  	if ( number_of_descendants < b . number_of_descendants ) return true;
  	if ( number_of_descendants > b . number_of_descendants ) return false;

  	if ( label < b . label ) return true;
  	if ( label > b . label ) return false;
  	return false;
  }
};

std::size_t hash_value(VertexInfo const& info) {
 	std::size_t seed = 0;
  boost::hash_combine(seed, info.number_in_edges);
  boost::hash_combine(seed, info.number_out_edges);
  boost::hash_combine(seed, info.number_of_ancestors);
  boost::hash_combine(seed, info.number_of_descendants);
  boost::hash_combine(seed, info.label);
  return seed;
 }

class DAG {
public:
	typedef std::string Label;
	typedef int64_t Vertex;
	typedef std::pair<Vertex, Vertex> Edge;
	Vertex num_vertices_;
	std::vector<Label> labels_;
	boost::unordered_set<Edge> edges_;

	std::vector < std::string > annotation;
	std::vector < std::vector<std::string> > annotation_vertex;

  mutable std::vector< std::pair<VertexInfo, Vertex> > vertex_info;
  mutable bool vertex_info_cached;
  DAG ( void ) { vertex_info_cached = false; }
  bool operator < ( DAG const& b) const {
  	return false; // dummy
	}
	const std::vector< std::pair<VertexInfo, Vertex> > & getVertexInfo ( void ) const {
		if ( vertex_info_cached ) return vertex_info;
		std::vector<uint64_t> out_edges ( num_vertices_, 0);
		std::vector<uint64_t> in_edges ( num_vertices_, 0 );
		std::vector<uint64_t> descendants ( num_vertices_, 0 );
		std::vector<uint64_t> ancestors ( num_vertices_, 0 );
		// assumption: the vertices are indexed in reverse topological order
		// Check this assumption.
		BOOST_FOREACH ( const Edge & e, edges_ ) {
			if ( e . second > e . first ) {
				std::cout << "The vertices are not indexed in a reverse topological order, GI.h must be generalized\n";
				abort ();
			}
		}
		boost::unordered_map < Vertex, boost::unordered_set < Vertex > > out_edge_map;
		boost::unordered_map < Vertex, boost::unordered_set < Vertex > > in_edge_map;
		BOOST_FOREACH ( const Edge & e, edges_ ) {
			out_edge_map [ e . first ] . insert ( e . second );
			in_edge_map [ e . second ] . insert ( e . first );
		}
		for ( Vertex v = 0; v < num_vertices_; ++ v ) {
			in_edges [ v ] = in_edge_map [ v ] . size ();
			BOOST_FOREACH ( Vertex w, in_edge_map [ v ] ) {
				descendants [ w ] += descendants [ v ] + 1;
			}
		}
		for ( Vertex v = num_vertices_ - 1; v >= 0; -- v ) {
			out_edges [ v ] = out_edge_map [ v ] . size ();
			BOOST_FOREACH ( Vertex w, out_edge_map [ v ] ) {
				ancestors [ w ] += ancestors [ v ] + 1;
			}
		}

		for ( Vertex v = 0; v < num_vertices_; ++ v ) {
			std::pair<VertexInfo, Vertex> item;
			item . first . number_in_edges = in_edges [ v ];
			item . first . number_out_edges = in_edges [ v ];
			item . first . number_of_ancestors = ancestors [ v ];
			item . first . number_of_descendants = descendants [ v ];
			item . first . label = labels_ [ v ];
			item . second = v;
			vertex_info . push_back ( item );
		}
		std::sort ( vertex_info . begin (), vertex_info . end () );
		vertex_info_cached = true;
		return vertex_info;
	}
	bool operator == (DAG const& b) const {
		DAG const& a = *this;
		if ( a . num_vertices_ != b . num_vertices_ ) return false;
		if ( a . edges_ . size () != b . edges_ . size () ) return false;

// make these const references?
		std::vector < std::pair<VertexInfo, Vertex> > a_info = a . getVertexInfo ();
		std::vector < std::pair<VertexInfo, Vertex> > b_info = b . getVertexInfo ();

		if ( a_info . size () != b_info . size () ) return false;
		std::vector < uint64_t > partition;
		partition . push_back ( 0 );
		for ( uint64_t i = 0; i < a_info . size (); ++ i ) {
			if ( a_info [ i ] . first != b_info [ i ] . first ) return false;
			if ( i > 0 ) if ( a_info [ i ] != a_info [ i - 1 ] ) partition . push_back ( i );
		}

		//std::cout << "Potential isomorphism candidate.\n";
		partition . push_back ( a_info . size () );
		// [ partition[i], partition[i+1] ) is begin and end of a VertexInfo.
		// Loop through all possible matchings respecting "partition"
		// This involves a variable-depth nested loop through permutations of each partition range
		std::vector < uint64_t > bijection ( b_info . size () );
		for ( uint64_t i = 0; i < b_info . size (); ++ i ) {
			bijection [ i ] = b_info [ i ] . second;
		}
		for ( uint64_t d = 0; d < partition . size () - 1; ++ d ) {
			std::sort ( &bijection [ partition [ d ] ], &bijection [ partition [ d + 1 ] ] );
		}
		bool possibilities_exhausted = false;
		while ( not possibilities_exhausted ) {
			// Check the bijection
			//std::cout << "Checking bijection.\n";
			boost::unordered_map < Vertex, Vertex > isomorphism;
			for ( uint64_t i = 0; i < bijection . size (); ++ i ) {
				isomorphism [ a_info [ i ] . second ] = bijection [ i ];
				//std::cout << a_info[i].second << " " << bijection [ i ] << "\n";
			}
			bool isomorphism_failed = false;
			BOOST_FOREACH ( const Edge & e, a . edges_ ) {
				Edge mapped_edge ( isomorphism [ e . first ], isomorphism [ e . second ] );
				if ( b . edges_ . count ( mapped_edge ) == 0 ) isomorphism_failed = true;
			}
			if ( not isomorphism_failed ) { 
				//std::cout << "Isomorphism found!\n";
				return true;
			}
			// Try the next permutation
			for ( uint64_t d = 0; d < partition . size (); ++ d ) {
				if ( d == partition . size () - 1 ) {
					possibilities_exhausted = true;
					continue;
				}
				if ( std::next_permutation ( &bijection [ partition [ d ] ], &bijection [ partition [ d + 1 ] ]) ) break;
			}
		}
		//std::cout << "No bijections were isomorphisms.\n";
		return false;
  }
};


 std::size_t hash_value(DAG const& dag) {
 	std::vector< std::pair<VertexInfo, DAG::Vertex> > info = dag . getVertexInfo ();
 	std::size_t seed = 0;
 	boost::hash_combine ( seed, dag . num_vertices_ );
 	boost::hash_combine ( seed, dag . edges_ . size () );
 	for ( int i = 0; i < info . size (); ++ i ) {
  	boost::hash_combine(seed, info [ i ] );
	}
  return seed;
 }
