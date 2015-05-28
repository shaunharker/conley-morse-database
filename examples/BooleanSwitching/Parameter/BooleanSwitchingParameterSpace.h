#ifndef BOOLEANSWITCHINGPARAMETERSPACE_H
#define BOOLEANSWITCHINGPARAMETERSPACE_H

#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <memory>
#include "boost/functional/hash.hpp"
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
  virtual std::shared_ptr<Parameter> parameter ( ParameterIndex v ) const;
  
  /// search
  ///    Given a parameter, find the vertex associated with it
  ///    (This can be used to find a parameter which might contain the other)
  virtual uint64_t search ( std::shared_ptr<Parameter> parameter ) const;
  
  /// closestFace
  ///   given a domain, return the closest face 
  ///   closest face is output in the following form:
  ///   There are d entries in an std::vector<int64_t>
  ///     0 means lower bound, 1 means between, 2 means upper bound
  std::vector<int64_t> closestFace ( std::shared_ptr<Parameter> parameter, 
                                 std::vector<size_t> const& domain ) const;
  /// domainLimits
  ///    Return a vector containing the number of thresholds plus one in each dimension
  ///    This gives us the number of bins in each dimension, which is needed
  ///    for multidimensional iteration through the "domains" (i.e. regions between
  ///    thresholds)
  std::vector<size_t> domainLimits ( void ) const;

  /// dimension
  ///    Return dimension
  int64_t dimension ( void ) const;

  /// factorGraph
  ///    Return factor graph
  const FactorGraph &
  factorGraph ( int64_t i ) const;

  /// prettyPrint64_t
  ///    Print64_t out parameter in human readable format
  std::string
  prettyPrint ( std::shared_ptr<Parameter> parameter ) const;

private:

  /// dimension_
  ///   dimension of phase space. This is equal to the number of network nodes.
  int64_t dimension_; 

  /// factors_
  ///    The stored FactorGraphs
  std::vector<FactorGraph> factors_;

  /// pinned_
  ///    If we want to fix a parameter from one of the factor graphs,
  ///    we set pinned_[d] to be the index of the monotonic map we want to fix it
  ///    at. Otherwise, we set pinned_[d]=-1
  std::vector<int64_t> pinned_;

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
    network_ . load ( loadstring.c_str() );
    // Get dimension
  dimension_ = network_ . size ();
  std::cout << "BooleanSwitchingParameterSpace::initialize." << 
    "dimension_ = " << dimension_ << "\n"; // DEBUG

  std::cout << network_ << "\n";
  // Loop through nodes and create FactorGraphs
  factors_ . resize ( dimension_ );
  pinned_ . resize ( dimension_ );
  for ( BooleanSwitching::Node const& node : network_ ) {
    int64_t d = node . index - 1; // Index of node (minus one to start indexing at 0)
    int64_t n = 0; // Number of in edges for node d
    std::vector<int64_t> logic; // Logic for node d inputs
    for ( int64_t i = 0; i < node . logic . size (); ++ i ) {
      int64_t k = node . logic [ i ] . size ();
      logic . push_back ( k );
      n += k;
    }
    int64_t m = node . out_order . size ();
    for ( int64_t x : logic ) std::cout << x << " "; std::cout << "\n";
    factors_ [ d ] . construct ( MonotonicMap ( n, m, logic, node . constraints ) );
    pinned_ [ d ] = node . choice;
    std::cout << "\n BooleanSwitchingParameterSpace::initialize. Constructing factors_[" << d << "] with n = " << n << " and m = " << m << "\n";
    std::cout << "This should correspond to " << network_ . name ( node . index ) << "\n";
    std::cout << "factors_[" << d << "].size() = " << factors_[d].size() << "\n"; // DEBUG
    if ( pinned_[d] != -1 ) std::cout << "Pinned to monotonic map " << pinned_[d] << "\n"; //DEBUG
  }
}

inline std::vector<BooleanSwitchingParameterSpace::ParameterIndex> 
BooleanSwitchingParameterSpace::adjacencies ( ParameterIndex v ) const {
  std::vector<ParameterIndex> result;
  std::shared_ptr<BooleanSwitchingParameter> p = 
      std::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter ( v ) );
  if ( not p ) {
    std::stringstream ss;
    ss << "BooleanSwitchingParameterSpace::adjacencies. ";
    ss << "Invalid ParameterIndex v = " << v << "\n";
    throw std::domain_error ( ss . str () );
  }
  // Loop through coordinates and change monotonic functions by one
  uint64_t multiplier = 1;
  for ( int64_t d = 0; d < dimension_; ++ d ) {
    if ( pinned_[d] != -1 ) continue;
    int64_t digit = p -> monotonic_function_ [ d ];
    std::vector<int64_t> const& neighbors = factors_ [ d ] . adjacencies ( digit );
    for ( int64_t neighbor : neighbors ) {
        result . push_back ( v + multiplier * ( neighbor - digit ) );
    }
    multiplier *= factors_ [ d ] . size ();
  }
  return result;
}
    
inline uint64_t 
BooleanSwitchingParameterSpace::size ( void ) const {
  uint64_t result = 1;
  for ( int64_t d = 0; d < dimension_; ++ d ) {
    if ( pinned_[d] == -1 ) result *= factors_ [ d ] . size ();
  }
  return result;
}

inline std::shared_ptr<Parameter> 
BooleanSwitchingParameterSpace::parameter ( ParameterIndex v ) const {
  std::shared_ptr<BooleanSwitchingParameter> 
    p ( new BooleanSwitchingParameter(dimension_) );
  for ( int64_t d = 0; d < dimension_; ++ d ) {
    if ( pinned_[d] != -1 ) {
      p -> monotonic_function_ [ d ] = pinned_ [ d ];
      continue;
    }
    size_t factor_size = factors_ [ d ] . size ();
    p -> monotonic_function_ [ d ] = v % factor_size;
    v /= factor_size;
  }
  return std::dynamic_pointer_cast<Parameter> ( p );
}
    
inline uint64_t 
BooleanSwitchingParameterSpace::search ( std::shared_ptr<Parameter> parameter ) const {
  const BooleanSwitchingParameter & p = 
    * std::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter );
  uint64_t result = 0;
  uint64_t multiplier = 1;
  for ( int64_t d = 0; d < dimension_; ++ d ) {
    result += multiplier * p . monotonic_function_ [ d ];
    multiplier *= factors_ [ d ] . size ();
  }
  return result;
}


inline std::vector<int64_t> 
BooleanSwitchingParameterSpace::closestFace 
                ( std::shared_ptr<Parameter> p, 
                  std::vector<size_t> const& domain ) const {
  std::shared_ptr<BooleanSwitchingParameter> parameter =
  std::dynamic_pointer_cast<BooleanSwitchingParameter> ( p ); 
  std::vector<int64_t> result ( dimension_ );
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
  std::unordered_map < std::pair<int64_t, int64_t>, bool, boost::hash<std::pair<int64_t, int64_t>> > state;
  for ( BooleanSwitching::Node const& node : network_ ) {
    int64_t critical_value = domain [ node . index - 1 ];
    int64_t count = 0;
    for ( int64_t out_node : node . out_order ) {
      state [ std::make_pair ( node . index, out_node ) ] = 
        ( count ++ < critical_value );
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
  for ( BooleanSwitching::Node const& node : network_ ) {
    uint64_t code = 0;
    uint64_t sweep_bit = 1;
    for ( std::vector<int64_t> const& factor : node . logic ) {
      for ( int64_t in_node : factor ) {
        bool bit = state [ std::make_pair ( std::abs(in_node), 
                           node . index ) ];
        if ( in_node < 0 ) bit = not bit; // Take into account down-regulation
        if ( bit ) code |= sweep_bit;
        sweep_bit <<= 1LL;
      }
    }
    // Note. Input code for node (node . index) has been established
    int64_t d = node . index - 1; // Index of node (minus one to start indexing at 0)
    int64_t monotonic_function_index = parameter -> monotonic_function_ [ d ];
    const MonotonicMap & monotonic_function = 
      factors_ [ d ] . vertices [ monotonic_function_index ];
    int64_t bin = monotonic_function . data_ [ code ];
    if ( bin < domain [ d ] ) result [ d ] = 0;
    else if ( bin == domain [ d ] ) result [ d ] = 1;
    else if ( bin > domain [ d ] ) result [ d ] = 2;    
  }

  /// DEBUG
  //std::cout << "domain:\n";
  //for ( auto x : domain ) std::cout << x << " ";
  //std::cout << "\nclosestFace result:\n";
  //for ( auto x : result ) std::cout << x << " ";
  //std::cout << "\n\n";
  /// END DEBUG
  return result;
}

inline std::vector<size_t> 
BooleanSwitchingParameterSpace::domainLimits ( void ) const {
  std::vector<size_t> result ( dimension_ );
  for ( BooleanSwitching::Node const& node : network_ ) {
    result [ node . index - 1 ] = node . out_order . size () + 1;
  }
  return result;
}

inline int64_t
BooleanSwitchingParameterSpace::dimension ( void ) const {
  return dimension_;
}

inline const FactorGraph &
BooleanSwitchingParameterSpace::factorGraph ( int64_t i ) const {
  return factors_ [ i ];
}

inline std::string BooleanSwitchingParameterSpace::
prettyPrint ( std::shared_ptr<Parameter> parameter ) const {
  std::stringstream result;
  const BooleanSwitchingParameter & p = 
      * std::dynamic_pointer_cast<BooleanSwitchingParameter> ( parameter );
  for ( int64_t d = 0; d < dimension_; ++ d ) {
    BooleanSwitching::Node node = network_ . node ( d + 1 );
    std::string symbol = network_ . name ( node . index );
    int64_t fd = node . index - 1;
    std::vector<std::string> input_symbols, output_symbols;
    for ( std::vector<int64_t> const& factor : node . logic ) {
      for ( int64_t input : factor ) {
        input_symbols . push_back ( network_ . name ( std::abs(input) ) );
      }
    }
    for ( int64_t output : node . out_order ) {
      output_symbols . push_back ( network_ . name ( output ) );
    }
    MonotonicMap mono = factors_ [ fd ] . vertices [ p . monotonic_function_ [ fd ] ];
    result << mono . prettyPrint ( symbol, input_symbols, output_symbols );
  }
  return result . str ();  
}


#endif
