#ifndef BOOLEANSWITCHINGNETWORK_H
#define BOOLEANSWITCHINGNETWORK_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>
#include <vector>
#include <exception>
#include <functional>
#include <unordered_map>
#include <boost/tokenizer.hpp>
#include "boost/iterator/counting_iterator.hpp"
#include "boost/iterator/transform_iterator.hpp"

namespace BooleanSwitching {

struct Node {
  int64_t index; // indexing starts at 1
  std::vector<std::vector<int64_t> > logic; // negative indices represent down-regulation
  std::vector<int64_t> out_order;
  // Constraint information
  std::vector<std::pair<int64_t,std::pair<int64_t,int64_t>>> constraints;
  // How "constraints" are encoded:
  // Given a constraint (m,(x,y)) in constraints
  // Given two bit codes, A and B, 
  // we test if A matches B off the mask m.
  // If so,
  // we test if A matches x on the mask
  // whether B matches y on the mask
  // bool matches == ((x & mask) == A);
  // Whenever we have two matches we must enforce the constraint.
};

class Network {
public:
  /// iterator
  ///   iterators dereference to class Node
  typedef boost::transform_iterator < std::function < Node const& ( int64_t ) >, 
    boost::counting_iterator<int64_t> > iterator;

  /// begin
  iterator 
  begin ( void ) const;

  /// end
  iterator 
  end ( void ) const;
  
  /// size
  int64_t
  size ( void ) const;

  /// load
  /// load network from file
  void 
  load ( char const* filename );

  /// index
  ///   Return index of node given name string
  int64_t 
  index ( std::string const& name ) const;

  /// name
  ///   Return name of node given index
  std::string
  name ( int64_t index ) const;

  /// node
  ///   Return node information given index
  Node const&
  node ( int64_t index ) const;
private:
  /// addConstraint
  ///   add a constraint
  void
  addConstraint ( std::string const& line );

  std::unordered_map<std::string, int64_t> name_to_index_;
  std::vector<std::string> names_;
  std::vector<Node> nodes_;
};

inline Network::iterator Network::
begin ( void ) const {
  return iterator ( 1, std::bind ( &Network::node, this, std::placeholders::_1 ) );
}

inline Network::iterator Network::
end ( void ) const {
  return begin() + size();
}

inline int64_t Network::
size ( void ) const {
  return name_to_index_ . size ();
}

inline int64_t Network::
index ( std::string const& name ) const {
  return name_to_index_ . find ( name ) -> second;
}

inline std::string Network::
name ( int64_t index ) const {
  return names_[index-1];
}

inline Node const& Network::
node ( int64_t index ) const {
  return nodes_[index-1];
}

inline std::ostream & 
operator << ( std::ostream & stream, 
              const Node & node ) {
  stream << "Node " << node . index << ":\n";
  for ( const std::vector<int64_t> & factor : node . logic ) {
    stream << "(";
    for ( int64_t i = 0; i < factor . size (); ++ i ) {
      if ( i != 0 ) stream << ", ";
      stream << factor [ i ];
    }
    stream << ")";
  }
  stream << "\n  Out Order:  ";
  for ( int64_t i = 0; i < node . out_order . size (); ++ i ) {
    if ( i != 0 ) stream << ", ";
    stream << node . out_order [ i ];
  }
  stream << "\n";
  return stream;
}

inline std::ostream & 
operator << ( std::ostream & stream, 
              const Network & network ) {
  stream << "There are " << network . size () << " nodes with the following names and indices:\n";
  for ( auto const& node : network ) {
    stream << node . index << " <-> " << network . name ( node . index ) << "\n";
  }
  for ( auto const& node : network ) {
    stream << node << "\n";
  }
  return stream;
}

inline int64_t 
parseNodeName ( const std::string & field, 
                std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  //std::cout << "NODE: " << field << "\n"; // DEBUG
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(" ", "~");
  tokenizer tokens(field, sep);
  tokenizer::iterator tok_iter = tokens . begin ();
  if ( tok_iter == tokens . end () ) throw std::logic_error ( field );
  std::string token = * tok_iter ++;
  std::string nodeName;
  bool negated;
  if ( token == std::string ( "~" ) ) { 
    negated = true;
    if ( tok_iter == tokens . end () ) throw std::logic_error ( field );
    nodeName = * tok_iter ++;
  } else {
    negated = false;
    nodeName = token;
  }
  if ( name_to_index_ . count ( nodeName ) == 0 ) {
    int64_t new_index = name_to_index_ . size () + 1;
    name_to_index_ [ nodeName ] = new_index;
  } 
  if ( tok_iter != tokens . end () ) throw std::logic_error ( field );
  return ( negated ? -1 : 1 ) * name_to_index_ [ nodeName ];
}

inline std::vector<int64_t> 
parseSum( const std::string & field,
          std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  //std::cout << "SUM: " << field << "\n"; // DEBUG
  std::vector<int64_t> result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("+ ");
  tokenizer tokens(field, sep);
  for ( std::string const& summand : tokens ) {
    result . push_back ( parseNodeName ( summand, name_to_index_ ) );
  }
  return result;
}

inline std::vector< std::vector<int64_t> > 
parseProductOfSums( const std::string & field,
                    std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  //std::cout << "POS: " << field << "\n"; // DEBUG
  std::vector< std::vector<int64_t> > result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("()");
  tokenizer tokens(field, sep);
  for ( std::string const& factor : tokens ) {
    std::vector<int64_t> factor_parse = parseSum ( factor, name_to_index_ );
    if ( factor_parse . size () > 0 ) result . push_back ( factor_parse );
  }
  return result;
}

inline std::vector< int64_t > 
parseOutOrder( const std::string & field,
               std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  //std::cout << "OUT: " << field << "\n"; // DEBUG
  std::vector<int64_t> result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(", ");
  tokenizer tokens(field, sep);
  for ( std::string const& outNode : tokens ) {
    result . push_back ( parseNodeName ( outNode, name_to_index_ ) );
  }
  return result;
}

inline Node 
parseLine ( const std::string & line,
            std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  //std::cout << "LINE: " << line << "\n"; // DEBUG
  Node result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(":->");
  tokenizer tokens(line, sep);
  tokenizer::iterator tok_iter = tokens . begin ();
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . index = parseNodeName ( * tok_iter ++, name_to_index_ );
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . logic = parseProductOfSums ( * tok_iter ++, name_to_index_ );
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . out_order = parseOutOrder ( * tok_iter ++, name_to_index_ );
  if ( tok_iter != tokens . end () ) throw std::logic_error ( line );
  return result;
}

inline void
Network::addConstraint ( std::string const& line ) {
  // Parse line. Example ". U(A,B) + L(A,C) <= L(A,B) + U(A,C)"
  // type == "." lhs == "U(A,B) + L(A,C)", ineq=="<=", rhs=="L(A,C) + U(A,C)"
  bool side = false; 
  bool sign = true;
  std::stringstream ss ( line );
  // Each term should have the same first argument
  int64_t node_index = -1;
  // A side is a collection of terms, which gets parsed into a collection of pairs.
  // Negative terms get placed on the opposite side.
  // The integer of the pair is the node index for the second argument of the term.
  // The boolean of the pair is false for L, true for U.
  std::unordered_map<int64_t, bool> leftterms, rightterms;
  while ( ss ) {
    std::string term;
    ss >> term;
    if ( term == "." ) continue;
    if ( term == ">" || term == ">=" ) {
      std::swap ( leftterms, rightterms );
      sign = true;
      continue;
    }
    if ( term == "<" || term == "<=" ) {
      side = true;
      sign = true;
      continue;
    }
    if ( term == "+" ) {
      sign = true;
      continue;
    }
    if ( term == "-" ) {
      sign = false;
      continue;
    }
    std::unordered_map<int64_t, bool> & terms 
      = (side != sign) ? leftterms : rightterms;
    bool U_or_L;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("(),");
    tokenizer tokens(term, sep);
    tokenizer::iterator tok_iter = tokens . begin ();
    std::string type = * tok_iter ++;
    if ( type . size () == 0 ) break;
    if ( type == "U" ) U_or_L = true;
    if ( type == "L" ) U_or_L = false;
    if ( type != "U" && type != "L") {
      std::cout << "term = {" << term << "} and type = {" << type << "}\n";
      throw std::logic_error ( "Problem parsing inequality: terms must be of form U(,) or L(,)\n");
    }
    std::string first_argument = * tok_iter ++;
    if ( name_to_index_ . count ( first_argument ) == 0 ) {
      throw std::logic_error ( "Problem parsing inequality (first argument)\n");
    }
    int64_t first_index = name_to_index_ [ first_argument ];
    if ( node_index == -1 ) { 
      node_index = first_index;
    }
    if ( node_index != first_index ) {
      throw std::logic_error ( "Problem parsing inequality (first arguments don't match)\n");
    }
    std::string second_argment = * tok_iter ++;
    if ( name_to_index_ . count ( second_argment ) == 0 ) {
      throw std::logic_error ( "Problem parsing inequality (second argument)\n");
    }
    int64_t second_index = name_to_index_ [ second_argment ];
    terms [ second_index ] = U_or_L;
    std::cout << "Parsed term " << term << " as (" << first_index << ", " 
              << second_index << ") with " << (U_or_L ? "U" : "L") << " on "
              << ((side != sign) ? "the left\n" : "the right\n");
  }
  if ( leftterms . empty () && rightterms . empty () ) return;
  // leftterms and rightterms need to be turned into bit codes corresponding to
  // node "node_index"
  Node & n = nodes_ [ node_index - 1 ];
  int64_t bit = 1;
  int64_t x = 0;
  int64_t y = 0;
  int64_t mask = 0;
  std::cout << "node_index = " << node_index << ", logic size =" << n . logic . size () << "\n";

  for ( int64_t f = 0; f < n . logic . size (); ++ f ) {
    std::cout << "node_index = " << node_index << ", factor number " << f << "\n";
    for ( int64_t i : n . logic [ f ] ) {
      std::cout << "input " << i << "\n";
      if ( leftterms . count ( std::abs(i) ) ) {
        std::cout << "Found in left terms.\n";
        if ( leftterms[std::abs(i)] ) x |= bit;
        mask |= bit;
      }
      if ( rightterms . count ( std::abs(i) ) ) {
        std::cout << "Found in right terms.\n";
        if ( rightterms[std::abs(i)] ) y |= bit;
        mask |= bit;
      }
      bit <<= 1;
    }
  }
  std::cout << "Adding constraint (" << std::bitset<64> (mask) << ", ("
    << std::bitset<64> (x) << ", " << std::bitset<64> (y) << ")) to node " << node_index << "\n";  
  n . constraints . push_back ( std::make_pair(mask, std::make_pair(x,y) ) );

}


inline void 
initialParse ( const std::string & line,
               std::unordered_map<std::string, int64_t> & name_to_index_ ) {
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(":->");
  tokenizer tokens(line, sep);
  tokenizer::iterator tok_iter = tokens . begin ();
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  parseNodeName ( * tok_iter ++, name_to_index_ );
}


inline void Network::
load ( const char * filename ) {
  std::ifstream infile ( filename );
  if ( not infile . good () ) {
    throw std::logic_error ( "Problem loading network file.\n");
  }
  bool inequalities_present = false;
  std::string line;
  while ( std::getline ( infile, line ) ) {
    if ( line[0] != '.' ) initialParse ( line, name_to_index_ );
    else inequalities_present = true;
  }
  infile.clear();
  infile.seekg(0,std::ios::beg);
  // The following line is for backwards compatibility, so older files
  // will not be indexed according to the new rules 
  if ( not inequalities_present ) name_to_index_ . clear ();
  while ( std::getline ( infile, line ) ) {
    if ( line[0] == '.' ) {
      addConstraint ( line );
    } else {
      try {
        Node node = parseLine ( line, name_to_index_ );
        nodes_ . push_back ( node );
      } catch ( std::logic_error & except ) {
        std::stringstream ss;
        ss << "Failure parsing network file " << filename << "\n";
        ss << "Trouble parsing: " << except . what () << "\n";
        throw std::logic_error ( ss . str () );
      }
    }
  }
  infile . close ();
  names_ . resize ( name_to_index_ . size () );
  for ( auto name_index_pair : name_to_index_ ) {
    names_ [ name_index_pair.second - 1 ] = name_index_pair.first;
  }
}

}

#endif
