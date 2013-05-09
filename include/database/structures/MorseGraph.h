/* TODO: Rename this file
 *  Morse_Graph.h
 */

#ifndef _CMDP_MORSE_GRAPH_
#define _CMDP_MORSE_GRAPH_

#include <utility>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/foreach.hpp"

// For now we use the old hash_set and hash_map 
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>

#include "database/structures/Grid.h"


/** Conley-Morse Graph.
 *  It is the directed acyclic graph whose vertices may
 *  be annotated with "Grid"s.
 *
 *  NOTE: This class doesn't validate acyclic condition.
 *  NOTE: Cellset and Graph must be default-constructable.
 *  TODO: Add a field for a phase space toplex.
 */
class MorseGraph {

 public:
   
  // Vertex types
  typedef int Vertex;  
  typedef boost::counting_iterator<Vertex> VertexIterator;
  typedef std::pair<VertexIterator, VertexIterator> VertexIteratorPair;

  // Edge types
  typedef std::pair<Vertex, Vertex> Edge;
  typedef boost::unordered_set<Edge>::const_iterator EdgeIterator;
  typedef std::pair<EdgeIterator, EdgeIterator> EdgeIteratorPair;

  /** Create an empty graph */
  MorseGraph();

  // WRITE FUNCTIONS
  
  /** Create a new vertex and return the descriptor of the vertex.
   *  The vertex is not connected to anywhere.
   *  An empty grid and ConleyIndex are allocated and
   *  assigned to the vertex just after this function is called.
   *  ("empty" objects mean the objects created by default-constructor.)
   */
  Vertex AddVertex();
  
  /** Add a edge from the "from" vertex to the "to" vertex.
   */
  Edge AddEdge(Vertex from, Vertex to);

  /** Remove a "from"-"to" edge
   *
   *  NOTE: some edge iterators become invalid when you call this function 
   */
  void RemoveEdge(Vertex from, Vertex to);
  
  //// READ FUNCTIONS
  
  /** return a number of vertices */
  unsigned int NumVertices() const;
  
  /** return a iterator pair to all vertices */
  VertexIteratorPair Vertices() const;
  
  /** return a iterator pair to all edges */
  EdgeIteratorPair Edges() const;
  
  MorseGraph Subgraph ( const std::set < Vertex > & vertices ) const;
  
  //// PROPERTY ACCESS
  
  /** Get the grid associated with the vertex. */
  boost::shared_ptr<Grid> & grid(Vertex vertex);
  boost::shared_ptr<const Grid> grid(Vertex vertex) const;
  
  /** Remove the grids associated with the vertices */
  void clearProperties ( void );
  
private:
  // DATA
  struct VertexData;
  int num_vertices_;
  boost::unordered_set < Edge > edges_;
  boost::unordered_map < Vertex, VertexData > property_;

  // PROPERTY
  struct VertexData {
    boost::shared_ptr<Grid> grid_;
    
    VertexData() {}
    virtual ~VertexData() {}
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & grid_;
    }
  };
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & num_vertices_;
    ar & edges_;
    ar & property_;
  }
};


/******************************/
/*  DEFINITIONS */
/****************/

/** Create an empty graph */
inline
MorseGraph::MorseGraph() {
  num_vertices_ = 0;
}

/** Create a new vertex and return the descriptor of the vertex.
 *  The vertex is not connected to anywhere.
 *  An empty gridPtr and ConleyIndex are allocated and
 *  assigned to the vertex just after this function is called.
 *  ("empty" objects mean the objects created by default-constructor.)
 */

inline
MorseGraph::Vertex
MorseGraph::AddVertex() {
  int v = num_vertices_ ++;
  property_ . insert ( std::make_pair ( v, VertexData () ) );
  return v;
}

/** Add a edge from the "from" vertex to the "to" vertex.
 */

inline
MorseGraph::Edge
MorseGraph::AddEdge(Vertex from, Vertex to) {
  return * edges_ . insert ( Edge (from, to) ) . first;
}

/** Remove a "from"-"to" edge
 *
 *  NOTE: some edge iterators become invalid when you call this function
 */

inline
void MorseGraph::RemoveEdge(Vertex from, Vertex to) {
  edges_ . erase ( Edge (from, to) );
}

/** return a number of vertices */
inline
unsigned int MorseGraph::NumVertices() const {
  return num_vertices_;
}

/** return a iterator pair to all vertices */
inline
MorseGraph::VertexIteratorPair
MorseGraph::Vertices() const {
  return VertexIteratorPair ( 0, num_vertices_ );
}


/** return a iterator pair to all edges */

inline
MorseGraph::EdgeIteratorPair
MorseGraph::Edges() const {
  return EdgeIteratorPair(edges_.begin(), edges_.end());
}

inline
MorseGraph
MorseGraph::Subgraph ( const std::set < Vertex > & vertices ) const {
  MorseGraph result;
  // Give proper number of vertices
  result . num_vertices_ = vertices . size ();
  std::map < Vertex, Vertex > lookup;
  // reindex vertices
  int i = 0;
  BOOST_FOREACH ( Vertex v, vertices ) {
    lookup [ v ] = i;
    ++ i;
  }
  // Loop through edges and insert the ones with both endpoints in vertices
  BOOST_FOREACH ( Edge e, edges_ ) {
    if ( vertices . count ( e . first ) != 0 && vertices . count ( e . second ) != 0 ) {
      result . AddEdge ( lookup [ e . first ], lookup [ e . second ] );
    }
  }
  // Loop through vertices and copy properties
  BOOST_FOREACH ( Vertex v, vertices ) {
    result . property_ [ lookup [ v ] ] = property_ . find ( v ) -> second;
  }
  return result;
}


// PROPERTY ACCESS

/** accessor method for grid assigned to vertex */
inline
boost::shared_ptr<Grid> & MorseGraph::grid(Vertex vertex) {
  return property_[vertex].grid_;
}

/** accessor method for grid assigned to vertex */
inline
boost::shared_ptr<const Grid> MorseGraph::grid(Vertex vertex) const {
  return property_ . find ( vertex ) -> second . grid_;
}

/** method to clear grids associated with all vertices */

inline void MorseGraph::clearProperties ( void ) {
  VertexIteratorPair vip = Vertices ();
  for ( VertexIterator vi = vip . first; vi != vip . second; ++ vi ) {
    grid ( *vi ) . reset ();
  }
}


#endif
