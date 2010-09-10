/*
 *  Conley_Morse_Graph.hpp
 */

/*
 *  Simple DFS is used.
 *  TODO: mark reached vertices.
 */
template<class CubeSet_t, class ConleyIndex_t>
bool ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::PathExists(
    ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::Vertex from,
    ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::Vertex to) const {
  if (from == to)
    return true;
  BOOST_FOREACH (Edge edge, boost::out_edges(from, graph_)) {
    Vertex next = boost::target(edge, graph_);
    if (PathExists(next, to))
      return true;
  }
  return false;
}

template<class CubeSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::OutEdgeIteratorPair
ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::OutEdges(
    ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::Vertex vertex) const {
  typedef typename boost::graph_traits<Graph>::out_edge_iterator Iter;
  boost::function<Vertex (Edge)> f =
      boost::bind(&ConleyMorseGraph::Target, boost::ref(*this), _1);
  Iter b, e;
  tie(b, e) = boost::out_edges(vertex, graph_);
  
  return OutEdgeIteratorPair(boost::make_transform_iterator(b, f),
                             boost::make_transform_iterator(e, f));
}

template<class CubeSet_t, class ConleyIndex_t>
typename ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::InEdgeIteratorPair
ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::InEdges(
    ConleyMorseGraph<CubeSet_t,ConleyIndex_t>::Vertex vertex) const {
  typedef typename boost::graph_traits<Graph>::in_edge_iterator Iter;
  boost::function<Vertex (Edge)> f =
      boost::bind(&ConleyMorseGraph::Source, boost::ref(*this), _1);
  Iter b, e;
  tie(b, e) = boost::in_edges(vertex, graph_);
  
  return InEdgeIteratorPair(boost::make_transform_iterator(b, f),
                            boost::make_transform_iterator(e, f));
}
