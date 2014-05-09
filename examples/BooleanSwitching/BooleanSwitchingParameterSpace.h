#ifndef BOOLEANSWITCHINGPARAMETERSPACE_H
#define BOOLEANSWITCHINGPARAMETERSPACE_H

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <exception>
#include <vector>
#include <stack>
#include <fstream>

#include "Network.h"
#include "FactorGraph.h"

/// class BooleanSwitchingParameter
///    A smart vertex class representing parameters of interest
///    for a Boolean Switching network, ignoring closest axis information
class BooleanSwitchingParameter : public Parameter {
public:
	std::vector<int> monotonic_function_; // given by preorder

	/// virtual deconstructor
	~BooleanSwitchingParameter ( void ) {}
	/// constructor
	BooleanSwitchingParameter ( int dimension_ ) : dimension_(dimension_) {
		monotonic_function_ . resize ( dimension_, 0 );
	}

	/// operator ==
	///   check equality
	bool operator == ( const BooleanSwitchingParameter & rhs ) const {
		if ( dimension_ != rhs . dimension_ ) return false;
		if ( rhs . monotonic_function_ . size () != monotonic_function_ . size () ) {
			return false;
		}
		for ( int d = 0; d < dimension_; ++ d ) {
			if ( monotonic_function_ [ d ] != rhs . monotonic_function_ [ d ] ) return false;
		}
		return true;
	}

	/// hash_value
	///   hash_value for class BooleanSwitchingParameter
	friend std::size_t hash_value ( const BooleanSwitchingParameter & p ) {
    std::size_t seed = 0;
		for ( int d = 0; d < dimension_; ++ d ) {
			boost::hash_combine(seed, p.monotonic_function_[d] );
		}
    return seed;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version ) {
  	ar & monotonic_function_;
  }
};

/// class BooleanSwitchingParameterSpace
class BooleanSwitchingParameterSpace : public ParameterSpace {
public:
	// typedef
	typedef uint64_t ParameterIndex;
	typedef boost::counting_iterator < ParameterIndex > iterator;
	typedef iterator const_iterator;
	// Constructor/Deconstructor
	BooleanSwitchingParameterSpace ( void ) {}
	virtual ~BooleanSwitchingParameterSpace ( void ) {}

	/// initialize
	///    Create the ParameterSpace given the configuration specified
	virtual void initialize ( int argc, char * argv [] );

	/// adjacencies
	///    Return a vector of adjacent vertices.
	virtual std::vector<ParameterIndex> adjacencies ( ParameterIndex v ) const;
	
	/// size
	///    Return the number of vertices
	virtual uint64_t size ( void ) const;

	/// parameter
	///    Return the parameter object associated with a vertex
	virtual boost::shared_ptr<Parameter> parameter ( ParameterIndex v ) const;
	
	/// search
	///    Given a parameter, find the vertex associated with it
	///    (This can be used to find a parameter which might contain the other)
	virtual uint64_t search ( boost::shared_ptr<Parameter> parameter ) const;
	
	/// closestFace
	///   given a domain, return the closest face 
	///   closest face is output in the following form:
	///   There are d entries in an std::vector<int>
	///     0 means lower bound, 1 means between, 2 means upper bound
	std::vector<int> closestFace ( boost::shared_ptr<Parameter> parameter, 
																 const std::vector<size_t> & domain ) const;
	/// domainLimits
	///    Return a vector containing the number of thresholds plus one in each dimension
	///    This gives us the number of bins in each dimension, which is needed
	///    for multidimensional iteration through the "domains" (i.e. regions between
	///    thresholds)
	std::vector<size_t> domainLimits ( void ) const;

	/// dimension
	///    Return dimension
	int dimension ( void ) const;
private:

	/// dimension_
	///   dimension of phase space. This is equal to the number of network nodes.
	int dimension_; 

	/// factors_
	///    The stored FactorGraphs
	std::vector<FactorGraph> factors_;

	/// network_
	BooleanSwitching::Network network_;

	// Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
  }
};

inline void 
BooleanSwitchingParameterSpace::initialize ( int argc, char * argv [] ) {
	// Load the network file
	std::string filestring ( argv[1] );
	std::string appendstring ( "/network.txt" );
	std::string loadstring = filestring + appendstring;
	network_ = BooleanSwitching::loadNetwork ( loadstring.c_str() );
	// Get dimension
  dimension_ = network_ . nodes_ . size ();
  // Loop through nodes and create FactorGraphs
  factors_ . resize ( dimension_ );
  BOOST_FOREACH ( const BooleanSwitching::NodeData & data, network_ . node_data_ ) {
  	int d = data . index - 1; // Index of node (minus one to start indexing at 0)
  	int n = 0; // Number of in edges for node d
  	std::vector<int> logic; // Logic for node d inputs
  	for ( int i = 0; i < data . logic . size (); ++ i ) {
  		int k = data . logic [ i ] . size ();
  		logic . push_back ( k );
			n += k;
		}
  	int m = data . out_order . size ();
  	factors_ [ d ] . construct ( MonotonicMap ( n, m, logic ) );
  }

}

inline std::vector<ParameterIndex> 
BooleanSwitchingParameterSpace::adjacencies ( ParameterIndex v ) const {
	std::vector<ParameterIndex> result;
	boost::shared_ptr<BooleanSwitchingParameter> p = 
		boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter ( v ) );
	// Loop through coordinates and change monotonic functions by one
	uint64_t multiplier = 1;
	for ( int d = 0; d < dimension_; ++ d ) {
		int digit = p -> monotonic_function_ [ d ];
		const std::vector<int> & neighbors = factors_ [ d ] . adjacencies ( digit );
		BOOST_FOREACH ( int neighbor, neighbors ) {
			result . push_back ( v + multiplier * ( neighbor - digit ) );
		}
		multiplier *= factors_ [ d ] . size ();;
	}
	return result;
}
	
inline uint64_t 
BooleanSwitchingParameterSpace::size ( void ) const {
	uint64_t result = 1;
	for ( int d = 0; d < dimension_; ++ d ) {
		result *= factors_ [ d ] . size ();
	}
	return result;
}

inline boost::shared_ptr<Parameter> 
BooleanSwitchingParameterSpace::parameter ( ParameterIndex v ) const {
	boost::shared_ptr<BooleanSwitchingParameter> 
		p ( new BooleanSwitchingParameter(dimension_) );
	for ( int d = 0; d < dimension_; ++ d ) {
		size_t factor_size = factors_ [ d ] . size ();
		p . monotonic_function_ [ d ] = v % factor_size;
		v /= factor_size;
	}
	return boost::dynamic_pointer_cast<Parameter> ( p );
}
	
inline uint64_t 
BooleanSwitchingParameterSpace::search ( boost::shared_ptr<Parameter> parameter ) const {
	const Parameter & p = 
		* boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter );
	uint64_t result = 0;
	uint64_t multiplier = 1;
	for ( int d = 0; d < dimension_; ++ d ) {
		result += multiplier * p . monotonic_function_ [ d ];
		multiplier *= factors_ [ d ] . size ();;
	}
	return result;
}


inline std::vector<int> 
BooleanSwitchingParameterSpace::closestFace 
				( boost::shared_ptr<Parameter> p, 
					const std::vector<size_t> & domain ) const {
	boost::shared_ptr<BooleanSwitchingParameter> parameter =
	  boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( p ); 
	std::vector<int> result ( dimension_ );
	if ( dimension_ != domain . size () ) { 
		std::cout << "error. BooleanSwitchingParameter::closestFace. Inappropriate input domain size.\n";
		throw std::logic_error ( "BooleanSwitchingParameter::closestFace. Inappropriate input domain size.\n");
	}

	boost::unordered_map < std::pair<int, int>, bool > state;
	BOOST_FOREACH ( const BooleanSwitching::NodeData & data, network_ . node_data_ ) {
  	int critical_value = domain [ data . index - 1 ];
  	int count = 0;
  	BOOST_FOREACH ( int out_node, data . out_order ) {
  		state [ std::make_pair ( data . index, out_node ) ] = 
  			( count ++ > critical_value );
  	}
  }
  BOOST_FOREACH ( const BooleanSwitching::NodeData & data, network_ . node_data_ ) {
  	uint64_t code = 0;
  	BOOST_FOREACH ( const std::vector<int> & factor, data . logic ) {
  		BOOST_FOREACH ( int in_node, factor ) {
  			code <<= 1;
  			bool bit = state [ std::make_pair ( std::abs(in_node), data . index ) ];
  			if ( in_node < 0 ) bit = not bit; // Take into account down-regulation
  			if ( bit ) ++ code;
  		}
  	}
  	// Note. Input code for node (data . index) has been established
  	int d = data . index - 1; // Index of node (minus one to start indexing at 0)
		int monotonic_function_index = parameter -> monotonic_function_ [ d ];

		const MonotonicMap & monotonic_function = 
			factors_ [ d ] . vertices [ monotonic_function_index ];

		int bin = monotonic_function . data_ [ code ];
		if ( bin < domain [ d ] ) result [ d ] = 0;
		else if ( bin == domain [ d ] ) result [ d ] = 1;
		else if ( bin > domain [ d ] ) result [ d ] = 2;	
	}

	return result;
}

inline std::vector<size_t> 
BooleanSwitchingParameterSpace::domainLimits ( void ) const {
	std::vector<size_t> result ( dimension_ );
  BOOST_FOREACH ( const BooleanSwitching::NodeData & data, network_ . node_data_ ) {
  	int d = data . index - 1;
  	result [ data . index - 1 ] = data . out_order . size () + 1;
	}
	return result;
}

inline int
BooleanSwitchingParameterSpace::dimension ( void ) const {
	return dimension_;
}

#endif
