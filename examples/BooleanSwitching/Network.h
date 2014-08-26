#ifndef BOOLEANSWITCHINGNETWORK_H
#define BOOLEANSWITCHINGNETWORK_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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
  return names_[index];
}

inline Node const& Network::
node ( int64_t index ) const {
  return nodes_[index];
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

inline void Network::
load ( const char * filename ) {
  std::ifstream infile ( filename );
  if ( not infile . good () ) {
    throw std::logic_error ( "Problem loading network file.\n");
  }
  std::string line;
  while ( std::getline ( infile, line ) ) {
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
  infile . close ();
  names_ . resize ( name_to_index_ . size () );
  for ( auto name_index_pair : name_to_index_ ) {
    names_ [ name_index_pair.second ] = name_index_pair.first;
  }
}

}

#endif
