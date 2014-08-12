

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