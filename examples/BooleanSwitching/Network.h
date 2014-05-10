#ifndef BOOLEANSWITCHINGNETWORK_H
#define BOOLEANSWITCHINGNETWORK_H

// char_sep_example_2.cpp
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <exception>
#include "boost/unordered_map.hpp"
#include "boost/foreach.hpp"

namespace BooleanSwitching {

struct NodeData {
  int index;
  std::vector<std::vector<int> > logic;
  std::vector<int> out_order;
};

class Network {
public:
  boost::unordered_map<std::string, int> nodes_;
  std::vector<NodeData> node_data_;
};

inline std::ostream & 
operator << ( std::ostream & stream, 
              const NodeData & data ) {
  stream << "Node " << data . index << ":\n";
  BOOST_FOREACH ( const std::vector<int> & factor, data . logic ) {
    stream << "(";
    for ( int i = 0; i < factor . size (); ++ i ) {
      if ( i != 0 ) stream << ", ";
      stream << factor [ i ];
    }
    stream << ")";
  }
  stream << "\n  Out Order:  ";
  for ( int i = 0; i < data . out_order . size (); ++ i ) {
    if ( i != 0 ) stream << ", ";
    stream << data . out_order [ i ];
  }
  stream << "\n";
  return stream;
}



inline std::ostream & 
operator << ( std::ostream & stream, 
              const Network & network ) {
  stream << "There are " << network . nodes_ . size () << " nodes with the following names and indices:\n";
  typedef std::pair<const std::string, int> value_t;
  BOOST_FOREACH ( const value_t & node, network . nodes_ ) {
    stream << node . first << " <-> " << node . second << "\n";
  }
  BOOST_FOREACH ( const NodeData & data, network . node_data_ ) {
    stream << data << "\n";
  }
  return stream;
}


inline int 
parseNodeName ( const std::string & field, 
                boost::unordered_map<std::string, int> & nodes_ ) {
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
  if ( nodes_ . count ( nodeName ) == 0 ) {
    int new_index = nodes_ . size () + 1;
    nodes_ [ nodeName ] = new_index;
  } 
  if ( tok_iter != tokens . end () ) throw std::logic_error ( field );
  return ( negated ? -1 : 1 ) * nodes_ [ nodeName ];
}

inline std::vector<int> 
parseSum( const std::string & field,
          boost::unordered_map<std::string, int> & nodes_ ) {
  //std::cout << "SUM: " << field << "\n"; // DEBUG
  std::vector<int> result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("+ ");
  tokenizer tokens(field, sep);
  BOOST_FOREACH ( std::string summand, tokens ) {
    result . push_back ( parseNodeName ( summand, nodes_ ) );
  }
  return result;
}

inline std::vector< std::vector<int> > 
parseProductOfSums( const std::string & field,
                    boost::unordered_map<std::string, int> & nodes_ ) {
  //std::cout << "POS: " << field << "\n"; // DEBUG
  std::vector< std::vector<int> > result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep("()");
  tokenizer tokens(field, sep);
  BOOST_FOREACH ( std::string factor, tokens ) {
    std::vector<int> factor_parse = parseSum ( factor, nodes_ );
    if ( factor_parse . size () > 0 ) result . push_back ( factor_parse );
  }
  return result;
}

inline std::vector< int > 
parseOutOrder( const std::string & field,
               boost::unordered_map<std::string, int> & nodes_ ) {
  //std::cout << "OUT: " << field << "\n"; // DEBUG
  std::vector<int> result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(", ");
  tokenizer tokens(field, sep);
  BOOST_FOREACH ( std::string outNode, tokens ) {
    result . push_back ( parseNodeName ( outNode, nodes_ ) );
  }
  return result;
}

inline NodeData 
parseLine ( const std::string & line,
            boost::unordered_map<std::string, int> & nodes_ ) {
  //std::cout << "LINE: " << line << "\n"; // DEBUG
  NodeData result;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(":->");
  tokenizer tokens(line, sep);
  tokenizer::iterator tok_iter = tokens . begin ();
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . index = parseNodeName ( * tok_iter ++, nodes_ );
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . logic = parseProductOfSums ( * tok_iter ++, nodes_ );
  if ( tok_iter == tokens . end () ) throw std::logic_error ( line );
  result . out_order = parseOutOrder ( * tok_iter ++, nodes_ );
  if ( tok_iter != tokens . end () ) throw std::logic_error ( line );
  return result;
}

inline Network
loadNetwork ( const char * filename ) {
  Network result;
  std::ifstream infile ( filename );
  if ( not infile . good () ) {
    throw std::logic_error ( "Problem loading network file.\n");
  }
  std::string line;
  while ( std::getline ( infile, line ) ) {
    try {
      NodeData data = parseLine ( line, result . nodes_ );
      result . node_data_ . push_back ( data );
    } catch ( std::logic_error & except ) {
      std::stringstream ss;
      ss << "Failure parsing network file " << filename << "\n";
      ss << "Trouble parsing: " << except . what () << "\n";
      throw std::logic_error ( ss . str () );
    }
  }
  infile . close ();
  std::cout << "Network.h loadNetwork. " << result << "\n"; // DEBUG
  return result;
}

}

#endif
