/*
 *  Conley_Morse_Graph.hpp
 */

#include <utility>
#include "boost/foreach.hpp"

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
  return property_[vertex].cell_set_;
}

/** accessor method for grid assigned to vertex */
inline
boost::shared_ptr<const Grid> MorseGraph::grid(Vertex vertex) const {
  return property_ . find ( vertex ) -> second . cell_set_;
}



