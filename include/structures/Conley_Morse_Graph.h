/*
 *  Conley_Morse_Graph.h
 */

#ifndef _CMDP_CONLEY_MORSE_GRAPH_
#define _CMDP_CONLEY_MORSE_GRAPH_

//#include <boost/archive/text_oarchive.hpp>  don't need?

// Note. The usage of map and set could be replaced by unordered_set and unordered_map,
//       however serialization support does not yet exist. It exists for hash_set and
//       hash_map, but the chosen solution is to just use set and map, and unordered can
//       be introduced on the next version (1.47) of Boost, when this issue is resolved.

#include <boost/iterator/counting_iterator.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/version.hpp>

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
template<class CellSet_t, class ConleyIndex_t>
class ConleyMorseGraph {

 public:
  
  typedef CellSet_t CellContainer;
 
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
  ConleyMorseGraph();

  // WRITE FUNCTIONS
  
  /** Create a new vertex and return the descriptor of the vertex.
   *  The vertex is not connected to anywhere.
   *  An empty CellSet and ConleyIndex are allocated and
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
  
  ConleyMorseGraph Subgraph ( const std::set < Vertex > & vertices ) const;
  
  //// PROPERTY ACCESS
  
  /** Get a Cellset of the vertex. */
  CellSet_t &CellSet(Vertex vertex);
  const CellSet_t &CellSet(Vertex vertex) const ;
  
  /** Get a Conley-Index of the vertex. */
  const ConleyIndex_t &ConleyIndex(Vertex vertex) const;
  ConleyIndex_t &ConleyIndex(Vertex vertex);
  
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
    CellSet_t cell_set_;
    ConleyIndex_t conley_index_;
    
    VertexData() {}
    virtual ~VertexData() {}
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & conley_index_;
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

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Conley_Morse_Graph.hpp"
#endif

#endif
