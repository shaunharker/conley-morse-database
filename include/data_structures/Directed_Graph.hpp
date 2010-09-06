/*
 *  Directed_Graph.hpp
 */

#include <functional>

/* BOOST includes */
//#include <boost/config.hpp>
//#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp> 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/foreach.hpp>

/* External Adaptation of DirectedGraph so it is a BGL graph as well */

namespace DirectedGraph_detail {
  
  template < class Toplex >
  class DirectedGraph_out_edge_iterator : public Toplex::Subset::const_iterator {
  public:
    /* Typedefs */
    typedef typename Toplex::Top_Cell vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    /* Default Constructor */
    DirectedGraph_out_edge_iterator ( void ) {}
    /* Constructor */
    DirectedGraph_out_edge_iterator (const vertex_descriptor & v, 
                                     const typename Toplex::Subset::const_iterator & it ) :
    Toplex::Subset::const_iterator ( it ), domain_cell_ ( v ) {}
    /* Dereference */
    const edge_descriptor operator * ( void ) {
      return std::pair < vertex_descriptor, vertex_descriptor> 
        ( domain_cell_, Toplex::Subset::const_iterator::operator * () );
    }
    /* Preincrement */
    DirectedGraph_out_edge_iterator & operator ++ ( void ) {
      Toplex::Subset::const_iterator::operator ++ ();
      return *this;
    } /* operator ++ */
    /* Postincrement */
    DirectedGraph_out_edge_iterator operator ++ ( int ) {
      return DirectedGraph_out_edge_iterator ( domain_cell_, 
             Toplex::Subset::const_iterator::operator ++ ( int () ) );
    } /* operator ++ */
  private:
    typename Toplex::Top_Cell domain_cell_;
  };

  template < class Toplex >
  class DirectedGraph_vertex_iterator : public DirectedGraph<Toplex>::const_iterator {
  public:
    /* Typedefs */
    typedef typename Toplex::Top_Cell vertex_descriptor;
    /* Default Constructor */
    DirectedGraph_vertex_iterator ( void ) {}
    /* Constructor */
    DirectedGraph_vertex_iterator ( const typename DirectedGraph<Toplex>::const_iterator & it ) :
    DirectedGraph<Toplex>::const_iterator ( it ) {}
    /* Dereference */
    const vertex_descriptor & operator * ( void ) {
      return DirectedGraph<Toplex>::const_iterator::operator * () . first;
    } /* operator * */
    /* Preincrement */
    DirectedGraph_vertex_iterator & operator ++ ( void ) {
      DirectedGraph<Toplex>::const_iterator::operator ++ ();
      return *this;
    } /* operator ++ */
    /* Postincrement */
    DirectedGraph_vertex_iterator operator ++ ( int ) {
      return DirectedGraph_vertex_iterator 
        ( DirectedGraph<Toplex>::const_iterator::operator ++ ( int () ) );
    } /* operator ++ */
    const vertex_descriptor * operator -> ( void ) {
      return & DirectedGraph<Toplex>::const_iterator::operator * () . first;
    } /* operator -> */
  };

  struct graph_traversal_category : 
  public virtual boost::incidence_graph_tag,
  public virtual boost::adjacency_graph_tag,
  public virtual boost::vertex_list_graph_tag { };
  
} /* namespace DirectedGraph_detail */

namespace boost {
  template < class Toplex >
  struct graph_traits< DirectedGraph<Toplex> > {    
    /* Vertex and Edge descriptor */
    typedef typename Toplex::Top_Cell vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    /* Graph categories */
    typedef boost::directed_tag directed_category;
    typedef boost::disallow_parallel_edge_tag edge_parallel_category;
    typedef DirectedGraph_detail::graph_traversal_category traversal_category;
    /* Size Types */
    typedef typename Toplex::size_type degree_size_type;
    typedef typename Toplex::size_type vertices_size_type;
    /* Iterators */
    typedef typename Toplex::Subset::const_iterator adjacency_iterator;
    typedef DirectedGraph_detail::DirectedGraph_out_edge_iterator<Toplex> out_edge_iterator;
    typedef DirectedGraph_detail::DirectedGraph_vertex_iterator<Toplex> vertex_iterator;
    /* Member functions */
    vertex_descriptor null_vertex ( void ) {
      return ( typename Toplex::Top_Cell ) ( -1 ); 
    } /* boost::graph_traits<>::null_vertex */
  };
  template < class Toplex >
  std::pair< typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator, 
  typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator > 
  adjacent_vertices ( typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor v, 
                     const DirectedGraph<Toplex> & g ) {
    return std::pair< typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator, 
    typename graph_traits< DirectedGraph<Toplex> >::adjacency_iterator > 
    ( g . find ( v ) -> second . begin (), g . find ( v ) -> second . end () );
  } /* boost::adjacent_vertices<> */
  
  /* For some reason the BGL has already implemented these and we get ambiguity.
     The problem is that these are overloads and not specialization. 
     Luckily, the defaults are precisely what we want. 
   */
  /*
  template < class Toplex >
  typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor 
  source(const typename graph_traits< DirectedGraph<Toplex> >::edge_descriptor & e, 
         const DirectedGraph<Toplex> & g) {	
    return e . first;
  } 
  *//* source */
  
  /*
  template < class Toplex >
  typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor
  target(const typename graph_traits< DirectedGraph<Toplex> >::edge_descriptor & e, 
         const DirectedGraph<Toplex> & g) {
    return e . second;
  } 
  *//* target */
  
  template < class Toplex >
  std::pair<typename graph_traits< DirectedGraph<Toplex> >::out_edge_iterator, 
  typename graph_traits< DirectedGraph<Toplex> >::out_edge_iterator > 
  out_edges(const typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor & u, 
            const DirectedGraph<Toplex> & g) {
    const typename Toplex::Subset & subset = g . find ( u ) -> second;
    typedef typename graph_traits< DirectedGraph<Toplex> >::out_edge_iterator edge_iter;
    return std::make_pair (edge_iter ( u, subset . begin () ),
                           edge_iter ( u, subset . end () ) );
  } /* out_edges */
  
  template < class Toplex >
  typename graph_traits< DirectedGraph<Toplex> >::degree_size_type  
  out_degree(const typename graph_traits< DirectedGraph<Toplex> >::vertex_descriptor & u, 
             const DirectedGraph<Toplex> & g) {
    return g . find ( u ) -> second . size ();
  } /* out_degree */
  
  template < class Toplex >
  std::pair<typename graph_traits< DirectedGraph<Toplex> >::vertex_iterator, 
            typename graph_traits< DirectedGraph<Toplex> >::vertex_iterator>
  vertices(const DirectedGraph<Toplex> & g) {
    typedef typename graph_traits< DirectedGraph<Toplex> >::vertex_iterator Iterator;
    return std::make_pair ( Iterator (g.begin()), Iterator (g.end()) );    
  } /* vertices */
  
  template < class Toplex >
  typename graph_traits< DirectedGraph<Toplex> >::vertices_size_type 
  num_vertices(const DirectedGraph<Toplex> & g)	{
    return g . size ();
  } /* num_vertices */
  
} /* namespace boost */

template < class Toplex >
typename DirectedGraph<Toplex>::comp_index_t 
DirectedGraph< Toplex >::getStronglyConnectedComponents(
  DirectedGraph< Toplex >::Components & C)
{
  // dummy: Pretend SCC has only one component (G itself).

  typename Toplex::Subset E;

  typename std::map < typename Toplex::Top_Cell, typename Toplex::Subset >::iterator i;
  i = std::map < typename Toplex::Top_Cell, typename Toplex::Subset >::begin();
  do {
    E.insert(i->first);
    i++;
  } while (i != std::map < typename Toplex::Top_Cell, 
           typename Toplex::Subset >::end());

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

/* Global functions */
template < class Toplex, class Map >
DirectedGraph<Toplex> compute_directed_graph (const Toplex & my_toplex, 
                                              const Map & f) {
  DirectedGraph<Toplex> directed_graph;
  for (typename Toplex::const_iterator cell_iterator = my_toplex . begin ();
       cell_iterator != my_toplex . end (); ++ cell_iterator ) {
    directed_graph[*cell_iterator] = my_toplex.cover(f(my_toplex.geometry(cell_iterator)));
  } /* for_each */
  
  return directed_graph;
} /* compute_directed_graph */
