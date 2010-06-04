/*
 *  Conley_Morse_Graph.hpp
 */

/*
 *  Simple DFS is used.
 *  TODO: mark reached vertices.
 */
template<class CubeSet, class ConleyIndex>
bool ConleyMorseGraph<CubeSet,ConleyIndex>::PathExist(
    ConleyMorseGraph<CubeSet,ConleyIndex>::Vertex from,
    ConleyMorseGraph<CubeSet,ConleyIndex>::Vertex to) {
  if (from == to)
    return true;
  BOOST_FOREACH (Edge edge, boost::out_edges(from, graph_)) {
    Vertex next = boost::target(edge, graph_);
    if (PathExist(next, to))
      return true;
  }
  return false;
}

template<class CubeSet, class ConleyIndex>
typename ConleyMorseGraph<CubeSet,ConleyIndex>::OutEdgeIteratorPair
ConleyMorseGraph<CubeSet,ConleyIndex>::OutEdges(
    ConleyMorseGraph<CubeSet,ConleyIndex>::Vertex vertex) {
  typedef typename boost::graph_traits<Graph>::out_edge_iterator Iter;
  boost::function<Vertex (Edge)> f =
      boost::bind(&ConleyMorseGraph::Target, boost::ref(*this), _1);
  Iter b, e;
  tie(b, e) = boost::out_edges(vertex, graph_);
  
  return OutEdgeIteratorPair(boost::make_transform_iterator(b, f),
                             boost::make_transform_iterator(e, f));
}

template<class CubeSet, class ConleyIndex>
typename ConleyMorseGraph<CubeSet,ConleyIndex>::InEdgeIteratorPair
ConleyMorseGraph<CubeSet,ConleyIndex>::InEdges(
    ConleyMorseGraph<CubeSet,ConleyIndex>::Vertex vertex) {
  typedef typename boost::graph_traits<Graph>::in_edge_iterator Iter;
  boost::function<Vertex (Edge)> f =
      boost::bind(&ConleyMorseGraph::Source, boost::ref(*this), _1);
  Iter b, e;
  tie(b, e) = boost::in_edges(vertex, graph_);
  
  return InEdgeIteratorPair(boost::make_transform_iterator(b, f),
                            boost::make_transform_iterator(e, f));
}
