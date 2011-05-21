/*
 *  Conley_Morse_Graph.h
 */

#ifndef _CMDP_CONLEY_MORSE_GRAPH_
#define _CMDP_CONLEY_MORSE_GRAPH_

#include <boost/graph/adjacency_list.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/shared_ptr.hpp>

/** Conley-Morse Graph.
 *  It is the directed acyclic graph whose vertices have a
 *  Cubeset and a ConleyIndex.
 *
 *  NOTE: This class doesn't validate acyclic condition.
 *  NOTE: Cubeset and Graph must be default-constructable.
 *  NOTE: ConleyIndex must be serializable.
 *  TODO: Add a field for a phase space toplex.
 */
template<class CubeSet_t, class ConleyIndex_t>
class ConleyMorseGraph {
 private:
  struct Component;

 public:
  /* property of vertices and graph type */
  struct ComponentProperty {
    typedef boost::vertex_property_tag kind;
  };
  typedef CubeSet_t CellContainer;
  typedef boost::property<ComponentProperty, Component> VertexProperty;
  typedef boost::no_property EdgeProperty;
  typedef boost::adjacency_list<boost::listS, boost::listS,
                                boost::bidirectionalS,
                                VertexProperty, EdgeProperty> Graph;

  /* vertex identifier type and edge identifier type */
  typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
  /* iterator types */
  typedef typename boost::graph_traits<Graph>::vertex_iterator VertexIterator;
  typedef std::pair<VertexIterator, VertexIterator> VertexIteratorPair;
  
  typedef boost::transform_iterator<
    boost::function<Vertex (Edge)>,
    typename boost::graph_traits<Graph>::out_edge_iterator> OutEdgeIterator;
  typedef std::pair<OutEdgeIterator, OutEdgeIterator> OutEdgeIteratorPair;
  
  typedef boost::transform_iterator<
    boost::function<Vertex (Edge)>,
    typename boost::graph_traits<Graph>::in_edge_iterator> InEdgeIterator;
  typedef std::pair<InEdgeIterator, InEdgeIterator> InEdgeIteratorPair;
  
  typedef typename boost::graph_traits<Graph>::edge_iterator EdgeIterator;
  typedef std::pair<EdgeIterator, EdgeIterator> EdgeIteratorPair;
  
  /* size types */
  typedef typename boost::graph_traits<Graph>::vertices_size_type
      VerticesSizeType;
  
  /** Create an empty graph */
  ConleyMorseGraph() {
    component_accessor_ = boost::get(ComponentProperty(), graph_);
  }
  
  /** Destruct a graph.
   *  Note that CubeSet and ConleyIndex assigned to each vertex are deleted.
   */
  virtual ~ConleyMorseGraph() {
  }
  
  /** Create a new vertex and return the descriptor of the vertex.
   *  The vertex is not connected to anywhere.
   *  An empty CubeSet and ConleyIndex are allocated and
   *  assigned to the vertex just after this function is called.
   *  ("empty" objects mean the objects created by default-constructor.)
   */
  Vertex AddVertex() {
    return boost::add_vertex(graph_);
  }
  /** Remove a vertex from the graph.
   *  All edges connected to and from the removed vertex are also removed.
   * 
   *  Note that CubeSet and ConleyIndex assigned to the vertex are
   *  also delteted, therefore please be careful not to share one ConleyIndex
   *  from other place.
   */
  void RemoveVertex(Vertex vertex) {
    boost::clear_vertex(vertex, graph_);
    boost::remove_vertex(vertex, graph_);
  }
  /** Add a edge from the "from" vertex to the "to" vertex.
   */
  Edge AddEdge(Vertex from, Vertex to) {
    return boost::add_edge(from, to, graph_) . first;
  }

  /** Remove a "from"-"to" edge
   *
   *  NOTE: some edge iterators become invalid when you call this function 
   */
  void RemoveEdge(Vertex from, Vertex to) {
    boost::remove_edge(from , to, graph_);
  }
  
  /** Return true if there is a path from the "from" vertex
   *  to the "to" vertex.
   *
   */
  bool PathExists(Vertex from, Vertex to) const;
  
  /** Get a cubeset of the vertex. */
  CubeSet_t &CubeSet(Vertex vertex) {
    return component_accessor_[vertex].cube_set_;
  }
  const CubeSet_t &CubeSet(Vertex vertex) const {
    return component_accessor_[vertex].cube_set_;
  }
  
  /** Get a Conley-Index of the vertex. */
  const ConleyIndex_t &ConleyIndex(Vertex vertex) const {
    return component_accessor_[vertex].conley_index_;
  }
  ConleyIndex_t &ConleyIndex(Vertex vertex) {
    return component_accessor_[vertex].conley_index_;
  }

  /** Return a index of the vertex.
   *  The index will be changed if a vertex is removed.
   *  This function is used only for debug. The Implementation is not efficient;
   */
  size_t GetVertexIndex(Vertex v) const {
    size_t n = 0;
    BOOST_FOREACH (Vertex w, Vertices()) {
      if (v == w)
        return n;
      n++;
    }
    return (size_t)-1;
  }
  
  /** return a iterator pair to all vertices */
  VertexIteratorPair Vertices() const {
    return boost::vertices(graph_);
  }
  
  /** return a number of vertices */
  VerticesSizeType NumVertices() const {
    return boost::num_vertices(graph_);
  }
  
  /** return a iterator pair to all out-edges */
  OutEdgeIteratorPair OutEdges(Vertex vertex) const;
  
  /** return a iterator pair to all verteces of in-edges */
  InEdgeIteratorPair InEdges(Vertex vertex) const;
  
  /** return a iterator pair to all edges */
  EdgeIteratorPair Edges() const {
    return boost::edges(graph_);
  }

  /** Return a edge descriptor */
  std::pair<Edge, bool> GetEdge(Vertex u, Vertex v) const {
    return boost::edge(u, v, graph_);
  }

  /** Return a target of given edge.
   *
   *  This function is used in OutEdges() to avoid a compilation error
   *  of imcomplete type inference.
   */
  Vertex Target(Edge e) const {
    return boost::target(e, graph_);
  }
  /** Return a source of given edge.
   *
   *  This function is used in InEdges() for the same reason of Target().
   */
  Vertex Source(Edge e) const {
    return boost::source(e, graph_);
  }
  /** Return Source and Target pair of edge;
   */
  std::pair<Vertex, Vertex> EdgePair(Edge e) const {
    return std::pair<Vertex, Vertex>(Source(e), Target(e));
  }
  /** Purge all ConleyIndex data in this ConleyMorseGraph.
   *  This function is used to avoid the serialization of ConleyIndex.
   *  This function is not implemented yet.
   */
  void PurgeConleyIndex();
  
 private:
  Graph graph_;
  typename boost::property_map<Graph, ComponentProperty>::type component_accessor_;

  /** struct of each component, which has a pointer to cubeset and conley index.
   *  there exist this struct because of serialization problem.
   * 
   *  TODO: use boost::shared_ptr
   */ 
  struct Component {
    CubeSet_t cube_set_;
    ConleyIndex_t conley_index_;
    Component() {}
    
    virtual ~Component() {}
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & conley_index_;
    }
  };
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & graph_;
  }
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Conley_Morse_Graph.hpp"
#endif

#endif
