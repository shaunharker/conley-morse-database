#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <exception>
#include <vector>
#include <stack>
#include <fstream>


// Two problems.
// 1. Speed. Easiest optimization is to record factorgraph edges
// 2. Size. Easiest optimization is to make equivalence of some axes.
//
//  Unfortunately the "closestFace" routine takes a lot of time.

/// class ConnectedSmartGraph where vertices are smart in the sense that they
///  (a) supply neighbors via a method "neighbors"
///  (b) the graph is connected
template < class T >
class ConnectedSmartGraph {
public:
	// gives indexing
	boost::unordered_map<T, int> preorder;
	// array of monotonic functions
	std::vector<T> vertices;

	// optional
	std::vector<std::vector< int > > adjacencies_;

	void construct ( const T & start ) {
		vertices . push_back ( start );
		preorder [ start ] = 0;

		std::stack<T> dfs_stack;
		dfs_stack . push ( start );

		size_t count = 0;
		while ( not dfs_stack . empty () ) {
			std::cout << ++ count << "   and stack size is " << dfs_stack . size () << "\n";
			T vertex = dfs_stack . top ();
			dfs_stack . pop ();
			std::vector<boost::shared_ptr<T> > neighbors = vertex . neighbors ();
			//std::cout << "Found " << neighbors . size () << " neighbors.\n";
			BOOST_FOREACH ( boost::shared_ptr<T> ptr, neighbors ) {
				if ( preorder . count ( *ptr ) ) continue;
				preorder [ * ptr ] = vertices . size ();
				vertices . push_back ( * ptr );
				dfs_stack . push ( * ptr );
			}
		}

		std::cout << "Number of unique monotonic maps = " << vertices . size () << "\n";

	}

	void compute_adjacencies ( void ) {
		adjacencies_ . resize ( vertices . size () );
		for ( int v = 0; v < vertices . size (); ++ v ) {
			T vertex = vertices [ v ];
			std::vector<boost::shared_ptr<T> > neighbors = vertex . neighbors ();
			BOOST_FOREACH ( boost::shared_ptr<T> ptr, neighbors ) {
				adjacencies_ [ v ] . push_back ( preorder [ *ptr ] );
			}
		}
	}

	const std::vector<int> & adjacencies ( int v ) const {
		return adjacencies_ [ v ];
	}

	void saveToFile ( void ) const {
		std::ofstream outfile ( "graph.gv" );
		outfile << "graph factorgraph {\n";
		for ( int v = 0; v < vertices . size (); ++ v ) {
			const T & vertex = vertices [ v ];
			std::vector<boost::shared_ptr<T> > neighbors = vertex . neighbors ();
			BOOST_FOREACH ( boost::shared_ptr<T> ptr, neighbors ) {
				int u = preorder . find ( * ptr) -> second;
				if ( v < u ) {
					outfile << v << " -- " << u << "\n";
				}
			}
		}
		outfile << "}\n\n";
	}

};

/// class MonotonicMap
/// a "smart vertex" class representing the dynamics of a node of
/// a boolean switching network
class MonotonicMap {
public:
	// data
	int n; // number of in edges  -- domain is {0,1,...,2^n-1}
	int m; // number of out edges -- codomain is {0,1,...,m}
	std::vector< int > data_;  // mapping
	// constructors
	MonotonicMap ( void ) {}
	MonotonicMap ( int n, int m ) : n(n), m(m) {
		data_ . resize ( (1 << n), 0 );
	}
	MonotonicMap ( int n, int m, std::vector<int> data ): n(n), m(m), data_(data) {}

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
				boost::shared_ptr<MonotonicMap> new_map ( new MonotonicMap ( n, m, copy ) );
				if ( new_map -> monotonic () && new_map -> realizable () ) 
					results . push_back ( new_map );
				++ copy[i];
			}
			if ( copy[i] < m ) {
				++ copy[i];
				boost::shared_ptr<MonotonicMap> new_map ( new MonotonicMap ( n, m, copy ) );
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

	friend std::size_t hash_value ( const MonotonicMap & p ) {
    std::size_t seed = 0;
    int N = (1 << p.n);
		for ( int i = 0; i < N; ++ i ) {
			boost::hash_combine(seed, p.data_[i] );
		}
    return seed;
  }
};

// Specialization of ConnectedSmartGraph to MonotonicMap smart vertices
typedef ConnectedSmartGraph<MonotonicMap> FactorGraph;

/// class Network
/// Represents a Boolean Switching Network
class Network {
public:

	int numVariables;
	// An array storing which vertices feed-in information
	std::vector<std::vector<int> > inputs;
	// An array storing which feed-in vertices are up regulators (true)
	// and which are down-regulators (false)
	std::vector< std::vector<bool> > regulation;
	// An array storing which vertices use information
	std::vector<std::vector<int> > outputs;
	// An array storing how many thresholds there are per variable 
	// (which implies there will be one more interval, however:
	//   (0, t_1) (t_1,t_2) ... (t_{n-1}, t_n) (t_n, \infty) )
	std::vector< int > threshold_count;

	// FactorGraphs
	std::vector<FactorGraph> factors_;

private:

};


/// class Parameter
///    A smart vertex class representing parameters of interest
///    for a Boolean Switching network
class Parameter {
private:
	std::vector<int> monotonic_function_; // given by preorder
	std::vector<int> axis_; // given by semi-axis -- negative means negative direction
	boost::shared_ptr<Network> network_;
public:
	
	/// constructor
	Parameter ( const boost::shared_ptr<Network> & network ) : network_(network) {
		int N = network_ -> numVariables;
		monotonic_function_ . resize ( N, 0 );
		int M = 1;
		for ( int i = 0; i < N; ++ i ) {
			M *= network_ -> threshold_count [ i ] + 1;
		}
		axis_ . resize ( M );
		// arbitrarily initialize axes with realizable positions
		for ( int i = 0; i < M; ++ i ) {
			std::vector<int> domain = indexToDomain ( i );
			std::vector<int> face = closestFace ( domain );
			int new_axis = 0;
			for ( int d = 1; d <= N; ++ d ) {
				if ( face [ d - 1 ] == 0 ) new_axis = -d;
				if ( face [ d - 1 ] == 2 ) new_axis = d;
			}
			axis_ [ i ] = new_axis;
		}
	}

	/// operator ==
	/// check equality
	bool operator == ( const Parameter & rhs ) const {
		if ( rhs . monotonic_function_ . size () != monotonic_function_ . size () ) {
			return false;
		}
		if ( rhs . axis_ . size () != axis_ . size () ) {
			return false;
		}
		if ( rhs . network_ != network_ ) return false;
		int N = monotonic_function_ . size ();
		int M = axis_ . size ();
		for ( int i = 0; i < N; ++ i ) {
			if ( monotonic_function_ [ i ] != rhs . monotonic_function_ [ i ] ) return false;
		}
		for ( int i = 0; i < M; ++ i ) {
			if ( axis_ [ i ] != rhs . axis_ [ i ] ) return false;
		}
		return true;
	}

	/// hash_value
	/// hash_value for class Parameter
	friend std::size_t hash_value ( const Parameter & p ) {
    std::size_t seed = 0;
    int N = p . monotonic_function_ . size ();
		int M = p . axis_ . size ();
		for ( int i = 0; i < N; ++ i ) {
			boost::hash_combine(seed, p.monotonic_function_[i] );
		}
		for ( int i = 0; i < M; ++ i ) {
			boost::hash_combine(seed, p.axis_[i] );
		}
    return seed;
  }

	/// clone
	/// Make a copy of this object and return a shared_ptr to it
	boost::shared_ptr<Parameter> clone ( void ) const {
		boost::shared_ptr<Parameter> result ( new Parameter ( network_ ) );
		result -> monotonic_function_ = monotonic_function_;
		result -> axis_ = axis_;
		result -> network_ = network_;
		return result;
	}

	/// canonicalize
	/// "Fix" a parameter by changing the chosen semi-axis to be correct
	/// There are two types of fixes: switching to an unambiguious semi-axis
	/// and changing a semi-axis to another in the same equivalence class.
	/// The essential idea is this takes care of the problem of moving a fixed point
	/// into or out of its own domain
	void canonicalize ( void ) {
		int N = monotonic_function_ . size ();
		int M = axis_ . size ();
		for ( int i = 0; i < M; ++ i ) {
			std::vector<int> domain = indexToDomain ( i );
			std::vector<int> face = closestFace ( domain );
			int reduced_count = 0;
			int new_axis = 0;
			for ( int d = 1; d <= N; ++ d ) {
				if ( face [ d - 1 ] != 1 ) ++ reduced_count;
				if ( face [ d - 1 ] == 0 ) new_axis = -d;
				if ( face [ d - 1 ] == 2 ) new_axis = d;
			}
			int & axis = axis_ [ i ];

			if ( (axis == 0) && (reduced_count == 1) ) {
				// codimension 1 face.
				axis = new_axis;
			}
			if ( reduced_count == 0 ) {
				// fixed point inside own domain
				axis = 0;
			}
			// TODO: handle equivalence class
			bool special_case = false;
			int dir = abs(axis) - 1;
			if ( axis < 0 ) {
				if ( domain [ dir ] == network_ -> threshold_count [ dir ] ) {
					special_case = true;
				}
			}
			if ( axis > 0 ) {
				if ( domain [ dir ] == 0 ) {
					special_case = true;
				}
			}
			if ( special_case ) {
				for ( int i = -N; i <= N; ++ i ) {
					if ( i == 0 ) continue;
					int other_d = abs(i) - 1;
					if ( i < 0 ) {
						if ( domain [ other_d ] == network_ -> threshold_count [ other_d ] ) {
							if ( face [ other_d ] == 0 ) { 
								axis = i;
								break;
							}
						}
					}
					if ( i > 0 ) {
						if ( domain [ other_d ] == 0 ) {
							if ( face [ other_d ] == 2 ) { 
								axis = i;
								break;
							}
						}
					}						
				}
			}
		}
	}

	/// neighbors
	/// Give a list of neighbors
	std::vector<boost::shared_ptr<Parameter> > neighbors ( void ) const {

		if ( not realizable () ) {
			std::cout << "This isn't even realizable to begin with!\n";
		}
		std::vector<boost::shared_ptr<Parameter> > result;
		int N = monotonic_function_ . size ();
		int M = axis_ . size ();
		if ( N != network_ -> numVariables ) {
			std::cout << "error. Parameter::neighbors. Incorrect number of monotonic functions.\n";
			//throw std::logic_error;
		}

		// Loop through first N coordinates and change monotonic functions by one
		for ( int i = 0; i < N; ++ i ) {
			const std::vector<int> & neighbors = 
				network_ -> factors_ [ i ] . adjacencies ( monotonic_function_ [ i ] );

			BOOST_FOREACH ( int neighbor, neighbors ) {
				boost::shared_ptr<Parameter> p = clone ();
				p -> monotonic_function_ [ i ] = neighbor;
				p -> canonicalize ();
				//std::cout << "Checking a candidate neighbor...\n";
				if ( p -> realizable () ) {
					result . push_back ( p );
					//std::cout << "  realizable!\n";
				}
			}

		}

		// Loop through next M coordinates and change axis choice by one
		for ( int i = 0; i < M; ++ i ) {
			if ( axis_ [ i ] == 0 ) continue;
			for ( int j = -N; j <= N; ++ j ) {
				if ( j == 0 ) continue;
				boost::shared_ptr<Parameter> p = clone ();
				p -> axis_ [ i ] = j;
				p -> canonicalize ();
				if ( (p -> realizable ()) && ( not (*p == *this ) ) ) {
					result . push_back ( p );
				}
			}
		}
		return result;

	}

	/// domainIndex
	/// Given a domain, return an index for the domain.
	int domainIndex ( const std::vector<int> & domain ) const {
		size_t jump = 1;
		int index = 0;
		for ( int i = 0; i < domain . size (); ++ i ) {
			index += jump * domain [ i ];
			jump *= network_ -> threshold_count [ i ] + 1;
		}
		// d[0] + j_0 d[1] + j_0 j_1 d[2] + ... j_0 j_{d-2}d[d-1]
		return index;
	}

	/// indexToDomain
	/// Given an index (i.e. the indexing for axis_) return the domain
	std::vector<int> indexToDomain ( int index ) const {
		int D = network_ -> numVariables;
		std::vector<int> domain ( D );
		for ( int i = 0; i < D; ++ i ) {
			size_t j = network_ -> threshold_count [ i ] + 1;
			domain [ i ] = index % j;
			index /= j;
		}
		return domain;
	}
	/// closestFace
	/// given a domain, return the closest face 
	/// closest face is output in the following form:
	/// There are d entries in an std::vector<int>
	///  0 means lower bound, 1 means between, 2 means upper bound
	std::vector<int> closestFace ( const std::vector<int> & domain ) const {
		int D = network_ -> numVariables;
		std::vector<int> result ( D );
		if ( D != domain . size () ) { 
			std::cout << "error. Parameter::closestFace. Inappropriate input domain size.\n";
			//throw std::logic_error;
		}


		for ( int d = 0; d < D; ++ d ) {
			int code = 0;
			// Determine bit-code for d.
			// Loop through indices for inputs to node d in network
			// We need to determine if the in-edges are "on" or "off"
			// For an up-regulator in-edge:
			//     An in-edge is "on" if the order of the edge is
			int num_in_edges = network_ -> inputs [ d ] . size ();
			for ( int in_edge_index = 0; in_edge_index < num_in_edges; ++ in_edge_index ) {
				int in_variable = network_ -> inputs [ d ] [ in_edge_index ];
				bool reg = network_ -> regulation [ d ] [ in_edge_index ];
				// TODO
				// Question: is this input "on" or "off"?
				// domain[d] tells us which threshold interval we are in
				// what is missing is a means of accessing the critical value
				int critical_value = 0;
				int num_out_edges = network_ -> outputs [ in_variable ] . size ();
				for ( int out_edge_index = 0; out_edge_index < num_out_edges; ++ out_edge_index ) {
					if ( network_ -> outputs [ in_variable ] [ out_edge_index ] == d ) {
						critical_value = out_edge_index;
						break;
					}
				}
				// critical_value 
				// domain[d] is which domain we are in; it is numbered according to the index
				// of the threshold we are to the left of.
				// critical_value tells us which out_edge we are from the in_variable
				// if critical_value is above domain[d], this means we have turned on 
				// (or off, if regulation is down)
				if ( reg ) {
					if ( domain[in_variable] > critical_value ) {
						code += (1 << d);
					}
				} else {
					if ( domain[in_variable] <= critical_value ) {
						code += (1 << d);
					}
				}

			}
			int monotonic_function_index = monotonic_function_ [ d ];

			const MonotonicMap & monotonic_function = 
				network_ -> factors_ [ d ] . vertices [ monotonic_function_index ];

			int bin = monotonic_function . data_ [ code ];
			if ( bin < domain [ d ] ) result [ d ] = 0;
			else if ( bin == domain [ d ] ) result [ d ] = 1;
			else if ( bin > domain [ d ] ) result [ d ] = 2;
		}
		return result;
	}

	/// closestAxis
	/// Given a domain, return which region the associated fixed point is in
	int closestAxis ( const std::vector<int> & domain ) const {
		return axis_ [ domainIndex ( domain ) ];
	}

	/// realizable
	/// Determine if the parameter is "realizable"
	/// In particular, there is a compatibility condition that must be satisified
	/// between the closestFace and closestAxis for each domain.
	bool realizable ( void ) const {

	// Step 1. Loop through domains.
		int N = network_ -> numVariables;
		std::vector<int> domain ( N, 0 );
		while ( 1 ) {
			// check compatibility between closest face and closest axis
			std::vector<int> face = closestFace ( domain );
			int axis = closestAxis ( domain );

			int d = abs(axis) - 1;
			bool special_case = false;
			if ( axis < 0 ) {
				if ( domain [ d ] == network_ -> threshold_count [ d ] ) {
					special_case = true;
				}
				if ( not special_case && face [ d ] != 0 ) { 
					//std::cout << "Not realizable.\n";
					return false;
				}

			}
			if ( axis > 0 ) {
				if ( domain [ d ] == 0 ) {
					special_case = true;
				}
				if ( not special_case && face [ d ] != 2 ) { 
					//std::cout << "Not realizable.\n";
					return false;
				}
			}
			if ( special_case ) {
				bool flag = false;
				for ( int i = -N; i <= N; ++ i ) {
					if ( i == 0 ) continue;
					int other_d = abs(i) - 1;
					if ( i < 0 ) {
						if ( domain [ other_d ] == network_ -> threshold_count [ other_d ] ) {
							if ( face [ other_d ] == 0 ) flag = true;
						}
					}
					if ( i > 0 ) {
						if ( domain [ other_d ] == 0 ) {
							if ( face [ other_d ] == 2 ) flag = true;
						}
					}						
				}
				if ( not flag ) return false;
			}

			bool done = true;
			for ( int i = 0; i < N; ++ i ) {
				++ domain [ i ];
				if ( domain [ i ] == network_ -> threshold_count [ i ] + 1 ) {
					domain [ i ] = 0;
				} else {
					done = false;
					break;
				}
			}
			if ( done ) break;
		}
		return true;
	}

};

/// class ParameterGraph
/// a specialization of ConnectedSmartGraph for the smart vertex class
/// "Parameter"
typedef ConnectedSmartGraph<Parameter> ParameterGraph;


int main ( void ) {

	// Construction of Network

/*
	FactorGraph CLN;
	MonotonicMap map_CLN (1, 2);
	CLN . construct ( map_CLN );
	CLN . compute_adjacencies ();

	FactorGraph YOX;
	MonotonicMap map_YOX (2, 1);
	YOX . construct ( map_YOX );
	YOX . compute_adjacencies ();

	FactorGraph CDK;
	MonotonicMap map_CDK (2, 4);
	CDK . construct ( map_CDK );
	CDK . compute_adjacencies ();

	FactorGraph APC;
	MonotonicMap map_APC (2, 2);
	APC . construct ( map_APC );
	APC . compute_adjacencies ();

	FactorGraph SBF;
	MonotonicMap map_SBF (5, 5);
	SBF . construct ( map_SBF );
	SBF . compute_adjacencies ();

	FactorGraph HCM;
	MonotonicMap map_HCM (1, 1);
	HCM . construct ( map_HCM );
	HCM . compute_adjacencies ();

	FactorGraph SFF;
	MonotonicMap map_SFF (3, 2);
	SFF . construct ( map_SFF );
	SFF . compute_adjacencies ();

	FactorGraph ACE;
	MonotonicMap map_ACE (2, 1);
	ACE . construct ( map_ACE );
	ACE . compute_adjacencies ();	

	boost::shared_ptr<Network> network_ptr ( new Network );

	Network & network = * network_ptr;

	network . numVariables = 8;
	network . inputs . resize ( 8 );
	network . regulation . resize ( 8 );
	network . outputs . resize ( 8 );
	network . threshold_count . resize ( 8 );

#define ADDEDGE(Y,X,Z)  \
	network . inputs [ (X) ] . push_back ( (Y) );     \
	network . regulation [ (X) ] . push_back ( (Z) ); \
	network . outputs [ (Y) ] . push_back ( (X) ); \
	network . threshold_count [ (Y) ] ++;

	ADDEDGE ( 0, 4, true );
	ADDEDGE ( 0, 3, false );
	ADDEDGE ( 1, 4, false );
	ADDEDGE ( 2, 3, true );
	ADDEDGE ( 2, 4, false );
	ADDEDGE ( 2, 6, true );
	ADDEDGE ( 2, 7, false );
	ADDEDGE ( 3, 1, false );
	ADDEDGE ( 3, 2, false );
	ADDEDGE ( 4, 0, true );
	ADDEDGE ( 4, 1, true );
	ADDEDGE ( 4, 4, true );
	ADDEDGE ( 4, 5, true );
	ADDEDGE ( 4, 6, true );
	ADDEDGE ( 5, 6, true );
	ADDEDGE ( 6, 2, true );
	ADDEDGE ( 6, 7, true );
	ADDEDGE ( 7, 4, true );


	network . factors_ . push_back ( CLN );
	network . factors_ . push_back ( YOX );
	network . factors_ . push_back ( CDK );
	network . factors_ . push_back ( APC );
	network . factors_ . push_back ( SBF );
	network . factors_ . push_back ( HCM );
	network . factors_ . push_back ( SFF );
	network . factors_ . push_back ( ACE );

	abort ();

	*/
	/*  small, two node network

	FactorGraph fg_A;
	MonotonicMap A (2, 2);
	fg_A . construct ( A );
	fg_A . compute_adjacencies ();

	FactorGraph fg_B;
	MonotonicMap B (1, 1);
	fg_B . construct ( B );
	fg_B . compute_adjacencies ();

	boost::shared_ptr<Network> network_ptr ( new Network );

	Network & network = * network_ptr;

	network . numVariables = 2;
	network . inputs . resize ( 2 );
	network . regulation . resize ( 2 );
	network . outputs . resize ( 2 );

	network . inputs [ 0 ] . push_back ( 0 );
	network . regulation [ 0 ] . push_back ( false );
	network . outputs [ 0 ] . push_back ( 0 );

	network . inputs [ 0 ] . push_back ( 1 );
	network . regulation [ 0 ] . push_back ( true );
	network . outputs [ 1 ] . push_back ( 0 );

	network . inputs [ 1 ] . push_back ( 0 );
	network . regulation [ 1 ] . push_back ( true );
	network . outputs [ 0 ] . push_back ( 1 );

	network . threshold_count . resize ( 2 );
	network . threshold_count [ 0 ] = 2;
	network . threshold_count [ 1 ] = 1;
	

	network . factors_ . push_back ( fg_A );
	network . factors_ . push_back ( fg_B );
	
	*/


  // 4 node example

// 0 <-> 1
	// | \
	// v |
	// 2 |
	// | |
	// v /  
	// 3	
	FactorGraph fg_A;
	MonotonicMap A (3, 3);
	fg_A . construct ( A );
	fg_A . compute_adjacencies ();

	FactorGraph fg_B;
	MonotonicMap B (1, 1);
	fg_B . construct ( B );
	fg_B . compute_adjacencies ();

	FactorGraph fg_C;
	MonotonicMap C (1, 1);
	fg_C . construct ( C );
	fg_C . compute_adjacencies ();

	FactorGraph fg_D;
	MonotonicMap D (1, 1);
	fg_D . construct ( D );
	fg_D . compute_adjacencies ();

	boost::shared_ptr<Network> network_ptr ( new Network );

	Network & network = * network_ptr;

	network . numVariables = 4;
	network . inputs . resize ( 4 );
	network . regulation . resize ( 4 );
	network . outputs . resize ( 4 );

	network . inputs [ 0 ] . push_back ( 0 );
	network . regulation [ 0 ] . push_back ( true );
	network . outputs [ 0 ] . push_back ( 0 );

	network . inputs [ 0 ] . push_back ( 1 );
	network . regulation [ 0 ] . push_back ( false );
	network . outputs [ 1 ] . push_back ( 0 );

	network . inputs [ 0 ] . push_back ( 3 );
	network . regulation [ 0 ] . push_back ( true );
	network . outputs [ 3 ] . push_back ( 0 );

	network . inputs [ 1 ] . push_back ( 0 );
	network . regulation [ 1 ] . push_back ( true );
	network . outputs [ 0 ] . push_back ( 1 );

	network . inputs [ 2 ] . push_back ( 0 );
	network . regulation [ 2 ] . push_back ( true );
	network . outputs [ 0 ] . push_back ( 2 );

	network . inputs [ 3 ] . push_back ( 2 );
	network . regulation [ 3 ] . push_back ( true );
	network . outputs [ 2 ] . push_back ( 3 );

	network . threshold_count . resize ( 4 );
	network . threshold_count [ 0 ] = 3;
	network . threshold_count [ 1 ] = 1;
	network . threshold_count [ 2 ] = 1;
	network . threshold_count [ 3 ] = 1;

	network . factors_ . push_back ( fg_A );
	network . factors_ . push_back ( fg_B );
	network . factors_ . push_back ( fg_C );
	network . factors_ . push_back ( fg_D );


	//fg . saveToFile ();

	// We need to build a network.
	//
	std::cout << "Constructing Parameter Graph.\n";
	ParameterGraph pg;
	Parameter p_start ( network_ptr );
	pg . construct ( p_start );
	std::cout << "Saving parameter graph to file.\n";
	pg . saveToFile ();

  return 0;
}


/* Accommodating reordering of thresholds

In addition to considering monotonic maps from {0,1...2^n-1} to {0,1,2,...m}
we must also worry about the ordering of the m thresholds.
If some value is not obtained by the monotonic map, this allows us to switch 
thresholds. This needs to be incorporated into the neighbors relation

so we introduce std::vector<int> order_

Given data_, there can be many order_ possibilities. Many of these possibilities
are equivalent. In particular, if a number k is not found in data_, then this implies
the kth out-edge and the k+1-th out-edge are "turned on" simultaneously. For this reason,
a reordering of order_[k] and order_[k+1] is equivalent. Thus we should prefer a
canonical representation, so we demand order_[k] < order_[k+1] whenever k is not in
data_. If consecutive values k, k+1, ..., k+l-1 are missing from data_, then we prefer
a choice of order_ so order_[k],...,order_[k+l-1] are sorted in increasing order.

Some confusion remains. What is the difference between data_ == 0 everywhere and 
data_ == m everywhere? What is the effect on equivalent classes of ordering?

Or do we want these different orderings to actually be distinct points?

More confusion. How does continuation work if we reorder thresholds? This alters space
in a way so that intersection is hard.
*/

/*

Suppose there are N vertices
and M domains.

Parameter graph has vertices which are
N+M-tuples.
The first N coordinates represent a choice of vertex in the factor graph
(i.e. a monotonic function)

The next M coordinates represent a choice of "region" for each domain. Regions
correspond to which "axis" the fixed point for the domain is closest to (the axes passing
through the center of the domain.)

In some cases, we union regions together since the difference does not matter.
We do this for any two adjacent semi-axes if there does not exist a chart opposite of
either semi-axis.

Not every combination of monotonic functions and region choices is realizable;
in particular the location of the fixed point with respect to the domain constrains
the possible regions in a rather obvious way.

We want to provide a more convenient interface to the client (i.e. the code which constructs
the wall graph) than simple access to the N+M-tuple. To this end we provide a
2M tuple, where the first M coordinates give the face of the domain closest to its 
associated fixed point, and the second M coordinates give the semi-axis of the domain
the associated fixed point is closest to. 

When the fixed point is actually within the domain, this is a special case for which the
"closest semi-axis" question becomes unimportant. These special cases require special
attention when determining adjacency in the Parameter Graph.

Adjacency in the parameter graph by and large should be determined by the
"one-coordinate-changed-by-one" rule. That is, all adjacent N+M-tuples are those
which are different only in one coordinate, and that difference is that that coordinate
has moved to an adjacent vertex in the appropriate factor graph. However there are
exceptions having to do with when fixed points move within their own domain; in this case
we are allowed to have two coordinates change.

How do we implement the equivalence classes for adjacency?

First note that if you have a non-trivial equivalence class, it neighbors every other
equivalence class.

What are possible equivalence classes? Clearly two can be together. 
To check this we can see that both of their opposite faces correspond to missing charts.

Is the characterization this simple: all axes with an existing opposite chart are singletons
and all the rest are in an equivalence class? Well, we may assume at least one threshold
in each direction. This means the missing charts are all touching one another.

I think this is the case.






*/

