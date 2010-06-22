/*
 *  Directed_Graph.hpp
 */

/* External Adaptation of DirectedGraph so it is a BGL graph as well */
namespace boost {
  template < class Toplex >
  struct graph_traits< DirectedGraph<Toplex> > {    
    typedef typename Toplex::Top_Cell vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef typename Toplex::Subset::const_iterator adjacency_iterator;
    typedef boost::directed_tag boost::graph_traits<DirectedGraph<Toplex> >::directed_category;
    typedef boost::disallow_parallel_edge_tag boost::graph_traits<DirectedGraph<Toplex> >::edge_parallel_category;
    typedef boost::adjacency_graph_tag boost::graph_traits<DirectedGraph<Toplex> >::traversal_category;
    vertex_descriptor null_vertex ( void ) {
      return ( typename Toplex::Top_Cell ) ( -1 ); 
    } /* boost::graph_traits<>::null_vertex */
  };
  template < Toplex >
  std::pair< typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator, 
             typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator > 
  adjacent_vertices ( typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor v, 
                      DirectedGraph<Toplex> & g ) {
    return std::pair< typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator, 
                      typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator > 
      ( g [ v ] . begin (), g [ v ] . end () );
  } /* boost::adjacent_vertices<> */
} /* namespace boost */

template < class Toplex >
typename DirectedGraph<Toplex>::comp_index_t 
DirectedGraph< Toplex >::getStronglyConnectedComponents(
  DirectedGraph< Toplex >::Components & C)
{
  // dummy: Pretend SCC has only one component (G itself).

  typename Toplex::Subset E;

  typename std::map< Toplex >::iterator i;
  i = std::map< Toplex >::begin();
  do {
    E.insert(i->first);
    i++;
  } while (i != std::map< Toplex >::end());

  C.push_back(E);

  return C.size();
}

template < class Toplex >
typename DirectedGraph< Toplex >::comp_index_t 
DirectedGraph< Toplex >::getConnectingPathLength(
  typename DirectedGraph< Toplex >::comp_index_t c1,
  typename DirectedGraph< Toplex >::comp_index_t c2)
{
  // dummy
  return 0;
}

template < class Toplex >
typename DirectedGraph< Toplex >::comp_index_t 
DirectedGraph< Toplex >::getStablePathLength(
  typename DirectedGraph< Toplex >::comp_index_t c) 
{
  // dummy
  return 0;
}
