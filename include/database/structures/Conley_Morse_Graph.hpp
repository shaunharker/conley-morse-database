/*
 *  Conley_Morse_Graph.hpp
 */

#include <utility>
#include "boost/foreach.hpp"

/** Create an empty graph */
template<class CellSet_t, class ConleyIndex_t>
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::ConleyMorseGraph() {
  num_vertices_ = 0;
}

/** Create a new vertex and return the descriptor of the vertex.
 *  The vertex is not connected to anywhere.
 *  An empty CellSet and ConleyIndex are allocated and
 *  assigned to the vertex just after this function is called.
 *  ("empty" objects mean the objects created by default-constructor.)
 */
template<class CellSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CellSet_t,ConleyIndex_t>::Vertex 
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
 AddVertex() {
  int v = num_vertices_ ++;
   property_ . insert ( std::make_pair ( v, VertexData () ) );
  return v;
}

/** Add a edge from the "from" vertex to the "to" vertex.
 */
template<class CellSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CellSet_t,ConleyIndex_t>::Edge
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
AddEdge(Vertex from, Vertex to) {
  return * edges_ . insert ( Edge (from, to) ) . first;
}

/** Remove a "from"-"to" edge
 *
 *  NOTE: some edge iterators become invalid when you call this function 
 */
template<class CellSet_t, class ConleyIndex_t>
void ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
 RemoveEdge(Vertex from, Vertex to) {
  edges_ . erase ( Edge (from, to) );
}

/** return a number of vertices */
template<class CellSet_t, class ConleyIndex_t>
unsigned int ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
NumVertices() const {
  return num_vertices_;
}

/** return a iterator pair to all vertices */
template<class CellSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CellSet_t,ConleyIndex_t>::VertexIteratorPair 
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
Vertices() const {
  return VertexIteratorPair ( 0, num_vertices_ );
}


/** return a iterator pair to all edges */

template<class CellSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CellSet_t,ConleyIndex_t>::EdgeIteratorPair 
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
Edges() const {
  return EdgeIteratorPair(edges_.begin(), edges_.end());
}

template<class CellSet_t, class ConleyIndex_t>
ConleyMorseGraph<CellSet_t,ConleyIndex_t>
ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
Subgraph ( const std::set < Vertex > & vertices ) const {
  ConleyMorseGraph result;
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

/** Get a Cellset of the vertex. */
template<class CellSet_t, class ConleyIndex_t>
CellSet_t & ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
CellSet(Vertex vertex) {
  return property_[vertex].cell_set_;
}
template<class CellSet_t, class ConleyIndex_t>
const CellSet_t & ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
CellSet(Vertex vertex) const {
  return property_ . find ( vertex ) -> second . cell_set_;
}

/** Get a Conley-Index of the vertex. */
template<class CellSet_t, class ConleyIndex_t>
ConleyIndex_t & ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
ConleyIndex(Vertex vertex) {
  return property_[vertex].conley_index_;
}

template<class CellSet_t, class ConleyIndex_t>
const ConleyIndex_t & ConleyMorseGraph<CellSet_t,ConleyIndex_t>::
ConleyIndex(Vertex vertex) const {
  return property_ . find ( vertex ) -> second . conley_index_;
}


