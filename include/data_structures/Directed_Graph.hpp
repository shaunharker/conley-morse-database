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
void DirectedGraph<Toplex>::removeVertex (typename Toplex::Top_Cell v)
{
  (*this).erase(v);
  typename DirectedGraph<Toplex>::iterator graph_it;

  graph_it = (*this).begin();
  while (graph_it != (*this).end()) {
    ((*graph_it).second).erase(v);
    ++graph_it;
  }
}

template < class Toplex >
DirectedGraph<Toplex> collapseComponents (
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Components & Components,
    std::vector<typename DirectedGraph<Toplex>::Vertex> & Representatives)
{
  typedef DirectedGraph <Toplex> Graph;
  typedef typename DirectedGraph<Toplex>::Vertex Vertex;
  std::map < Vertex, Vertex > table;

  Graph result(G);
  Vertex compRep; // The representative of a component

  // Iterators
  typename Graph::iterator graph_it;
  typename Graph::Components::iterator comp_it;
  typename Graph::Component::iterator vert_it;
  typename Toplex::Subset::iterator edge_it;
  
  // For each component, remove the vertices of the component
  // except the first one, which will be the representative.
  // The representative collects edges of removed vertices.
  // The 'table' variable, which will be used to replace the
  // destinations of edges, is constructed at the same time.
  Representatives.clear();
  comp_it = Components.begin();
  while (comp_it != Components.end()) {
    vert_it = (*comp_it).begin();
    compRep = (*vert_it);
    ++vert_it;
    while (vert_it != (*comp_it).end()) {
      edge_it = G[*(vert_it)].begin();
      while (edge_it != G[*(vert_it)].end()) {
        result[compRep].insert(*edge_it);
        ++edge_it;
      }
      //result[compRep].insert(G[*(vert_it)].begin(), G[*(vert_it)].end());
      result.erase(*vert_it);
      table.insert(typename std::map< Vertex, Vertex >::value_type((*vert_it), compRep));
      ++vert_it;
    }
    ++comp_it;
    Representatives.push_back(compRep);
  }

  // Replace the destinations of edges according to 'table' 
  std::vector<Vertex> toBeRemoved;
  graph_it = result.begin();
  while (graph_it != result.end()) {
    edge_it = ((*graph_it).second).begin();
    toBeRemoved.clear();
    while (edge_it != ((*graph_it).second).end()) {
      if (table.count(*edge_it)) {
        ((*graph_it).second).insert(table[*edge_it]);
        toBeRemoved.push_back(*edge_it);
      }
      ++edge_it;
    }
    for (size_t i = 0; i < toBeRemoved.size(); i++) {
      ((*graph_it).second).erase(toBeRemoved[i]);
    }
    ++graph_it;
  }
  
  // Remove self loops at the representative of components
  for (size_t i = 0; i < Representatives.size(); i++) {
    result[Representatives[i]].erase(Representatives[i]);
  }

  // Return 
  return result;
}

template < class Toplex >
typename Toplex::Subset DirectedGraph<Toplex>::operator () ( const typename Toplex::Top_Cell & cell ) {
  return operator [] ( cell );
} /* DirectedGraph<Toplex>::operator () for Top Cells */

// typename Toplex::Subset DirectedGraph<Toplex>::operator () ( const typename Toplex::Subset & subset ) {
//   typename Toplex::Subset return_value;
//   /* Loop through subset */
//   for ( typename Toplex::Subset::const_iterator it = subset . begin ();
//         it != subset . end (); ++ it ) {
//     const typename Toplex::Subset & value = operator [] ( * it );
//     for ( typename Toplex::Subset::const_iterator range_it = value . begin ();
//          range_it != value . end (); ++ range_it ) {
//       return_value . insert ( * range_it );
//     } /* for */
//   } /* for */
//   return return_value;
// } /* DirectedGraph<Toplex>::operator () for Subsets */

/* Global functions */

/* A wrapper for the boost's strongly connected components algorithm */
#include <boost/graph/strong_components.hpp>
template < class Toplex >
typename DirectedGraph<Toplex>::Components computeSCC(DirectedGraph<Toplex> & G)
{
  using namespace boost;

  typedef DirectedGraph<Toplex> Graph;
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

  typename Graph::Components result;
  
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
  
  int num_scc = strong_components(G, component_number, 
                                  root_map(root).
                                  color_map(color).
                                  discover_time_map(discover_time));

  std::vector < std::vector < Vertex > > components;
  build_component_lists(G, num_scc, component_number, components);

  /* Erase top cells not in attractor sets */
  Vertex v;
  typename std::vector < std::vector < Vertex > >::iterator it = components.begin();
  while (it != components.end ()) {
    if ((*it).size() == 1) {
      v = (*it)[0];
      if (G[v].count(v) == 0) {
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
    result.push_back(comp);
  }

  /* Exit the function */
  return result;
}

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

// Compute the length of the longest path from v to w
// assuming the graph is acyclic
template < class Toplex >
size_t computeLongestPathLength(
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Vertex v,
    typename DirectedGraph<Toplex>::Vertex w)
{

  typedef DirectedGraph<Toplex> Graph;
  typedef typename DirectedGraph<Toplex>::Vertex Vertex;
  std::map <Vertex, size_t> distance;

  // Iterators
  typename Graph::iterator graph_it;
  typename Graph::Components::iterator comp_it;
  typename Graph::Component::iterator vert_it;
  typename Toplex::Subset::iterator edge_it;

  // Initialize 'distance'
  graph_it = G.begin();
  while (graph_it != G.end()) {
    distance[(*graph_it).first] = 0;
    ++graph_it;
  }
  distance[v] = 1;

  // Main loop
  for (size_t i = 0; i < G.size() - 1; i++) {
    graph_it = G.begin();
    while (graph_it != G.end()) {
      if (distance[(*graph_it).first] >= 0) {
        edge_it = ((*graph_it).second).begin();
        while (edge_it != ((*graph_it).second).end()) {
          if (distance[(*graph_it).first] + 1 > distance[*edge_it]) {
            distance[*edge_it] = distance[(*graph_it).first] + 1;
          }
          ++edge_it;
        }
      }
      ++graph_it;
    }
  }

  if (distance[w] > 0) {
    return distance[w] - 1;
  } else {
    return 0;
  }
}

// The main loop of 
template < class Toplex >
void BFLoop(DirectedGraph<Toplex> & G,
            std::map <typename DirectedGraph<Toplex>::Vertex, size_t> & distance)
{
  typedef DirectedGraph<Toplex> Graph;
  typedef typename DirectedGraph<Toplex>::Vertex Vertex;

  // Iterators
  typename Graph::iterator graph_it;
  typename Graph::Components::iterator comp_it;
  typename Graph::Component::iterator vert_it;
  typename Toplex::Subset::iterator edge_it;

  // Main loop
  for (size_t i = 0; i < G.size() - 1; i++) {
    graph_it = G.begin();
    while (graph_it != G.end()) {
      if (distance[(*graph_it).first] >= 0) {
        edge_it = ((*graph_it).second).begin();
        while (edge_it != ((*graph_it).second).end()) {
          if (distance[(*graph_it).first] + 1 > distance[*edge_it]) {
            distance[*edge_it] = distance[(*graph_it).first] + 1;
          }
          ++edge_it;
        }
      }
      ++graph_it;
    }
  }
  return;
}

template < class Toplex >
void computePathBounds(DirectedGraph<Toplex> G,
                       typename DirectedGraph<Toplex>::Components SCC,
                       typename Toplex::Subset Entrance,
                       typename Toplex::Subset Exit,
                       std::vector<size_t> & ConnectingPathBounds,
                       std::vector<size_t> & EntrancePathBounds,
                       std::vector<size_t> & ExitPathBounds,
                       size_t & ThruPathBound)
{

  typedef DirectedGraph<Toplex> Graph;
  typedef typename DirectedGraph<Toplex>::Vertex Vertex;
  typedef typename DirectedGraph<Toplex>::Components Components;
  size_t n = G.size();
  size_t nComponents = SCC.size();
  typename Graph::iterator graph_it;
  typename Toplex::Subset::iterator comp_it;

  Graph H;
  //std::vector<typename DirectedGraph<Toplex>::Vertex> V;
  std::vector<Vertex> V;

  // Compute the collapsed graph
  // V is a vector containing vertices of the collapsed graph
  // corresponding to strongly connected components
  H = collapseComponents(G, SCC, V);
  
  // Components to Components
  ConnectingPathBounds.clear();
  for (size_t i = 0; i < nComponents; i++) {
    for (size_t j = 0; j < nComponents; j++) {
      if (i == j) {
        ConnectingPathBounds.push_back(0);
      } else {
        Graph K(H);
        for (size_t k = 0; k < nComponents; k++) {
          if ((k != i) & (k != j)) {
            K.removeVertex(V[k]);
          }
        }
        ConnectingPathBounds.push_back(computeLongestPathLength(K, V[i], V[j]));
      }
    }
  }
  
  std::map <Vertex, size_t> distance;

  // Entrance to Components
  EntrancePathBounds.clear();
  for (size_t i = 0; i < nComponents; i++) {
    Graph K(H);
    for (size_t j = 0; j < nComponents; j++) {
      if (j != i) {
        K.removeVertex(V[j]);
      }
    }
    distance.clear();
    graph_it = K.begin();
    while(graph_it != K.end()) {
      if (Entrance.count((*graph_it).first)) {
        distance[(*graph_it).first] = 1;
      } else {
        distance[(*graph_it).first] = 0;
      }
      ++graph_it;
    }
    BFLoop(K, distance);
    EntrancePathBounds.push_back(distance[V[i]]);
  }

  // Components to Exit
  ExitPathBounds.clear();
  for (size_t i = 0; i < nComponents; i++) {
    distance.clear();
    graph_it = H.begin();
    while(graph_it != H.end()) {
      distance[(*graph_it).first] = 0;
      ++graph_it;
    }
    distance[V[i]] = 1;
    BFLoop(H, distance);
    size_t maxLength = 0;
    comp_it = Exit.begin();
    while (comp_it != Exit.end()) {
      if (maxLength < distance[*comp_it]) {
        maxLength = distance[*comp_it];
      }
      ++comp_it;
    }
    ExitPathBounds.push_back(maxLength);
  }

  // Entrance to Exit
  Graph K(H);
  for (size_t i = 0; i < nComponents; i++) {
    K.removeVertex(V[i]);
  }
  distance.clear();
  graph_it = K.begin();
  while(graph_it != K.end()) {
    if (Entrance.count((*graph_it).first)) {
      distance[(*graph_it).first] = 1;
    } else {
      distance[(*graph_it).first] = 0;
    }
    ++graph_it;
  }
  BFLoop(K, distance);
  size_t maxLength = 0;
  comp_it = Exit.begin();
  while (comp_it != Exit.end()) {
    if (maxLength < distance[*comp_it]) {
      maxLength = distance[*comp_it];
    }
    ++comp_it;
  }
  ThruPathBound = maxLength;
  
  return;
}
