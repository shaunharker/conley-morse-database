#ifndef BOOLEANSWITCHINGPARAMETERSPACE_H
#define BOOLEANSWITCHINGPARAMETERSPACE_H

#include <iostream>
#include <sstream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <exception>
#include <vector>
#include <stack>
#include <fstream>

#include "database/structures/ParameterSpace.h"
#include "database/structures/AbstractParameterSpace.h"

#include "Network.h"
#include "Parameter/FactorGraph.h"
#include "Parameter/BooleanSwitchingParameter.h"
#include "Parameter/Polytope.h"

/// class BooleanSwitchingParameterSpace
class BooleanSwitchingParameterSpace : public AbstractParameterSpace {
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
                                 std::vector<size_t> const& domain ) const;
  /// domainLimits
  ///    Return a vector containing the number of thresholds plus one in each dimension
  ///    This gives us the number of bins in each dimension, which is needed
  ///    for multidimensional iteration through the "domains" (i.e. regions between
  ///    thresholds)
  std::vector<size_t> domainLimits ( void ) const;

  /// dimension
  ///    Return dimension
  int dimension ( void ) const;

  /// factorGraph
  ///    Return factor graph
  const FactorGraph &
  factorGraph ( int i ) const;

/*
  /// polytope
  ///   Return a polytope corresponding to a parameter
  Polytope 
  polytope ( boost::shared_ptr<Parameter> parameter ) const;
*/
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
  std::cout << "BooleanSwitchingParameterSpace::initialize\n";
    // Load the network file
    std::string filestring ( argv[1] );
    std::string appendstring ( argv[2] );
    std::string loadstring = filestring + appendstring;

  std::cout << "BooleanSwitchingParameterSpace::initialize." << 
    " Loading network file " << loadstring << "\n";
    network_ = BooleanSwitching::loadNetwork ( loadstring.c_str() );
    // Get dimension
  dimension_ = network_ . nodes_ . size ();
  std::cout << "BooleanSwitchingParameterSpace::initialize." << 
    "dimension_ = " << dimension_ << "\n"; // DEBUG

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
    std::cout << "BooleanSwitchingParameterSpace::initialize." << 
      "factors_[" << d << "].size() = " << factors_[d].size() << "\n"; // DEBUG
  }

}

inline std::vector<BooleanSwitchingParameterSpace::ParameterIndex> 
BooleanSwitchingParameterSpace::adjacencies ( ParameterIndex v ) const {
    std::vector<ParameterIndex> result;
    boost::shared_ptr<BooleanSwitchingParameter> p = 
        boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter ( v ) );
  if ( not p ) {
    std::stringstream ss;
    ss << "BooleanSwitchingParameterSpace::adjacencies. ";
    ss << "Invalid ParameterIndex v = " << v << "\n";
    throw std::domain_error ( ss . str () );
  }
  
  // DEBUG BEGIN
  //if ( factors_ . size () != dimension_ ) {
  //  std::cout << "Bug.\n";
  //  throw std::logic_error ( "BooleanSwitchingParameterSpace::adjacencies. Dimension/Factor mismatch.\n");
  //}
  // DEBUG END

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
        p -> monotonic_function_ [ d ] = v % factor_size;
        v /= factor_size;
    }
    return boost::dynamic_pointer_cast<Parameter> ( p );
}
    
inline uint64_t 
BooleanSwitchingParameterSpace::search ( boost::shared_ptr<Parameter> parameter ) const {
    const BooleanSwitchingParameter & p = 
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
                  const std::vector<size_t> const& domain ) const {
    boost::shared_ptr<BooleanSwitchingParameter> parameter =
      boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( p ); 
    std::vector<int> result ( dimension_ );
    if ( dimension_ != domain . size () ) { 
        std::cout << "error. BooleanSwitchingParameter::closestFace. "
                 "Inappropriate input domain size.\n";
        throw std::logic_error ( "BooleanSwitchingParameter::closestFace. "
                             "Inappropriate input domain size.\n");
    }

  // Determination of out-states.
  //    Loop through each node of network.
  //    For each node, examine the out-edges in the order they are listed
  //    Depending on the "critical_value", which is the domain bin
  //    of the associated phase space variable, we determine whether this
  //    output should be "on" or "off". We store this data in the 
  //    "state" data structure, which maps (source,target) to "off,on"
  //  Notes:  The indexing of nodes starts at 1, whereas the indexing
  //          of dimension starts at 0. This requires a translation
  //          from network indexing to dimension indexing, which is
  //          just to subtract 1.
  //          We store the "state" via network indexing.
    boost::unordered_map < std::pair<int, int>, bool > state;
    BOOST_FOREACH ( const BooleanSwitching::NodeData & data, 
                  network_ . node_data_ ) {
    int critical_value = domain [ data . index - 1 ];
    //std::cout << "closestFace. variable = " << data . index - 1 << "\n";
    //std::cout << "closestFace.   critical_val = " << critical_value << "\n";

    int count = 0;
    BOOST_FOREACH ( int out_node, data . out_order ) {
        state [ std::make_pair ( data . index, out_node ) ] = 
            ( count ++ < critical_value );
      //std::cout << "closestFace.   state (" << data.index<<", "<<out_node<<") = " <<
      //             (int) state [ std::make_pair ( data . index, out_node ) ] << "\n";
    }

  }

  // Loop through each node of network
  //  Loop through each in-edge (i.e. each factor of logic, 
  //                                  each summand of factor)
  //     Access state corresponding to in-edge.
  //     If this is a down-regulator, flip the state
  //     Append the state as the last bit of a growing code-word
  //  End
  //  Determine monotonic function for node
  //  Call the monotonic function with the code look-up
  //  Write the result component
  // End
  BOOST_FOREACH ( const BooleanSwitching::NodeData & data, 
                  network_ . node_data_ ) {
    uint64_t code = 0;
    BOOST_FOREACH ( const std::vector<int> & factor, data . logic ) {
        BOOST_FOREACH ( int in_node, factor ) {
            code <<= 1;
            bool bit = state [ std::make_pair ( std::abs(in_node), 
                                            data . index ) ];
            if ( in_node < 0 ) bit = not bit; // Take into account down-regulation
            if ( bit ) ++ code;
        }
    }
    // Note. Input code for node (data . index) has been established
    int d = data . index - 1; // Index of node (minus one to start indexing at 0)

    //std::cout << "Variable " << d + 1 << " received code " << code << "\n";

        int monotonic_function_index = parameter -> monotonic_function_ [ d ];

        const MonotonicMap & monotonic_function = 
            factors_ [ d ] . vertices [ monotonic_function_index ];

        int bin = monotonic_function . data_ [ code ];
    //std::cout << "Consulting monotonic function " << monotonic_function_index << " for bin value.\n";
    //std::cout << "This results in bin value " << bin << "\n";
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
    result [ data . index - 1 ] = data . out_order . size () + 1;
    }
    return result;
}

inline int
BooleanSwitchingParameterSpace::dimension ( void ) const {
    return dimension_;
}

inline const FactorGraph &
BooleanSwitchingParameterSpace::factorGraph ( int i ) const {
  return factors_ [ i ];
}

/*
inline Polytope BooleanSwitchingParameterSpace::
polytope ( boost::shared_ptr<Parameter> parameter ) const {
  Polytope result;
  const BooleanSwitchingParameter & p = 
      * boost::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter );
  for ( int d = 0; d < dimension_; ++ d ) {
    MonotonicMap mono = factors_ [ d ] . vertices [ p . monotonic_function_ [ d ] ];
    result << mono . polytope ();
  }
  return result;
}
*/

#endif
