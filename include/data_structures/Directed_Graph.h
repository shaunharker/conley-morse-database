/*
 *  Directed_Graph.h
 */

#ifndef _CMDP_DIRECTED_GRAPH_
#define _CMDP_DIRECTED_GRAPH_

#include <map>
#include <vector>
#include <set>
#include <iostream>

template < class Toplex >
class DirectedGraph : public std::map < typename Toplex::Top_Cell, 
                                        typename Toplex::Subset > 
{
public:
  typedef typename Toplex::Top_Cell Vertex;
  typedef typename Toplex::Subset Component;
  typedef std::vector<Component> Components;
  typedef typename Component::size_type vert_index_t;
  typedef typename Components::size_type comp_index_t;

  // Constructor
  DirectedGraph() { };

  // Destructor
  ~DirectedGraph() { };

  // Functional Notation when Directed Graph is considered as a Combinatorial Map
  typename Toplex::Subset operator () ( const typename Toplex::Top_Cell & );
  typename Toplex::Subset operator () ( const typename Toplex::Subset & );
};


template < class Toplex, class Map >
DirectedGraph<Toplex> compute_directed_graph (const Toplex & my_toplex, 
                                              const Map & f);


template < class Toplex, class Map >
void subdivide_toplex_and_directed_graph (Toplex * my_toplex, 
                                          DirectedGraph<Toplex> * my_directed_graph,
                                          const typename Toplex::Subset & subdivide_me,
                                          const Map & f);

template < class Toplex >
DirectedGraph<Toplex> collapseVertices (
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Components & oldComponents,
    typename DirectedGraph<Toplex>::Components & newComponents);

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Directed_Graph.hpp"
#endif

#endif
