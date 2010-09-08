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
  
/* For some reason the BGL has already implemented the following and we get ambiguity.
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
typename Toplex::Subset DirectedGraph<Toplex>::operator () ( const typename Toplex::Top_Cell & cell ) {
  return operator [] ( cell );
} /* DirectedGraph<Toplex>::operator () for Top Cells */

typename Toplex::Subset DirectedGraph<Toplex>::operator () ( const typename Toplex::Subset & subset ) {
  typename Toplex::Subset return_value;
  /* Loop through subset */
  for ( typename Toplex::Subset::const_iterator it = subset . begin ();
        it != subset . end (); ++ it ) {
    const typename Toplex::Subset & value = operator [] ( * it );
    for ( typename Toplex::Subset::const_iterator range_it = value . begin ();
         range_it != value . end (); ++ range_it ) {
      return_value . insert ( * range_it );
    } /* for */
  } /* for */
  return return_value;
} /* DirectedGraph<Toplex>::operator () for Subsets */

/* A wrapper for the boost's strongly connected components algorithm */
#include <boost/graph/strong_components.hpp>
template < class Toplex >
typename DirectedGraph<Toplex>::comp_index_t 
DirectedGraph< Toplex >::computeStronglyConnectedComponents(
    DirectedGraph< Toplex >::Components & C)
{
  using namespace boost;

  typedef DirectedGraph<Toplex> Graph;
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

  /* Pair Associative Container -- Vertex to Vertex or Integer or Color */
  typedef std::map< Vertex, Vertex > PAC_VV_t; 
  typedef std::map< Vertex, int > PAC_VI_t; 
  typedef std::map< Vertex, default_color_type > PAC_VC_t; 
  /* Associative Property Map -- Vertex to Integer or Color */
  typedef boost::associative_property_map< PAC_VV_t > APM_VV_t;
  typedef boost::associative_property_map< PAC_VI_t > APM_VI_t;
  typedef boost::associative_property_map< PAC_VC_t > APM_VC_t;

  PAC_VI_t pac_component;
  PAC_VI_t pac_discover_time;
  PAC_VV_t pac_root;
  PAC_VC_t pac_color;
  
  APM_VI_t component_number ( pac_component );
  APM_VI_t discover_time ( pac_discover_time );
  APM_VV_t root ( pac_root );
  APM_VC_t color ( pac_color );
  
  int num_scc = strong_components(*this, component_number, 
                                  root_map(root).
                                  color_map(color).
                                  discover_time_map(discover_time));

  std::vector < std::vector < Vertex > > components;
  build_component_lists(*this, num_scc, component_number, components);

  /* Erase top cells not in attractor sets */
  Vertex v;
  typename std::vector < std::vector < Vertex > >::iterator it = components.begin();
  while (it != components.end ()) {
    if ((*it).size() == 1) {
      v = (*it)[0];
      if ((*this)[v].count(v) == 0) {
        components.erase(it);
        --it;
      }
    }
    ++it;
  }

  /* Convert 'components (std::vec)' into C (Toplex::Components) */
  typename DirectedGraph< Toplex >::Component comp;
  typename std::vector < Vertex >::iterator vect_it;
  it = components.begin();
  while (it != components.end()) {
    // std::cout << "comp" << std::endl;
    vect_it = (*it).begin();
    comp.clear();
    while(vect_it != (*it).end()) {
      // std::cout << *vect_it << std::endl;
      comp.insert(*vect_it);
      ++vect_it;
    }
    components.erase(it);
    C.push_back(comp);
  }

  /* Exit the function */
  return C.size();
}

template < class Toplex >
typename DirectedGraph< Toplex >::vert_index_t 
DirectedGraph< Toplex >::computeLongestPathLength(const Vertex v, const Vertex w)
{
  // dummy
  return 0;
}

template < class Toplex >
typename DirectedGraph< Toplex >::vert_index_t 
DirectedGraph< Toplex >::computeConnectingPathBound(const comp_index_t c1, 
                                                    const comp_index_t c2)
{
  // dummy
  return 0;
}

template < class Toplex >
typename DirectedGraph< Toplex >::vert_index_t 
DirectedGraph< Toplex >::computeEntrancePathBound(const comp_index_t c, 
                                                  const Component Entrance)
{
  // dummy
  return 0;
}

template < class Toplex >
typename DirectedGraph< Toplex >::vert_index_t 
DirectedGraph< Toplex >::computeExitPathBound(const comp_index_t c, 
                                              const Component Entrance)
{
  // dummy
  return 0;
}

template < class Toplex >
typename DirectedGraph< Toplex >::vert_index_t 
DirectedGraph< Toplex >::computeThroughPathBound(const Component Entrance, 
                                                 const Component Exit)
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

template < class Toplex, class Map >
void subdivide_toplex_and_directed_graph (Toplex * my_toplex, 
                                          DirectedGraph<Toplex> * my_directed_graph,
                                          const typename Toplex::Subset & subdivide_me,
                                          const Map & f) {
  /* TO BE IMPLEMENTED */
} /* subdivide_toplex_and_directed_graph */

template < class Toplex >
DirectedGraph<Toplex> collapseVertices (
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Components & oldComponents,
    typename DirectedGraph<Toplex>::Components & newComponents)
{
  typedef DirectedGraph <Toplex> Graph;
  typedef typename DirectedGraph<Toplex>::Vertex Vertex;
  std::map < Vertex, Vertex > table;

  Graph result(G);
  Vertex componentRepresentative;
  
  // Initialize the table
  typename Graph::iterator graph_it = G.begin();
  while (graph_it != G.end()) {
    table.insert(typename std::map< Vertex, Vertex >::value_type((*graph_it).first,
                                                                 (*graph_it).first));
    ++graph_it;
  }
  
  // The first vertex of a component will be the representative
  // of the component
  typename DirectedGraph<Toplex>::Components::iterator comp_it = oldComponents.begin();
  while (comp_it != oldComponents.end()) {
    typename DirectedGraph<Toplex>::Component::iterator vert_it = (*comp_it).begin();
    componentRepresentative = (*vert_it);
    ++vert_it;
    while (vert_it != (*comp_it).end()) {
      result.erase(*vert_it);
      ++vert_it;
    }
    ++comp_it;
  }

  // Apply the table
  
  // dummy
  return result;
}
