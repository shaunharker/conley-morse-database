/* TODO: Rename this file
 *  Conley_Morse_Graph.h
 */

#ifndef _CMDP_CONLEY_MORSE_GRAPH_
#define _CMDP_CONLEY_MORSE_GRAPH_

#include <boost/iterator/counting_iterator.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>

#include "database/structures/Grid.h"

// For now we use the old hash_set and hash_map 
#if 0
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>
#else
#include <set>
#include <map>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#endif

/** Conley-Morse Graph.
 *  It is the directed acyclic graph whose vertices have a
 *  Cellset and a ConleyIndex.
 *
 *  NOTE: This class doesn't validate acyclic condition.
 *  NOTE: Cellset and Graph must be default-constructable.
 *  NOTE: ConleyIndex must be serializable.
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
#if 0
  typedef boost::unordered_set<Edge>::const_iterator EdgeIterator;
#else
  typedef std::set<Edge>::const_iterator EdgeIterator;
#endif
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
  
private:
  // DATA
  struct VertexData;
  int num_vertices_;
#if 0
  boost::unordered_set < Edge > edges_;
  boost::unordered_map < Vertex, VertexData > property_;
#else
  
  std::set < Edge > edges_;
  std::map < Vertex, VertexData > property_;
#endif
  // PROPERTY
  struct VertexData {
    boost::shared_ptr<Grid> cell_set_;
    
    VertexData() {}
    virtual ~VertexData() {}
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
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

#include "database/structures/Conley_Morse_Graph.hpp"

#endif
