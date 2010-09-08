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
  
  // Return the number of SCC and store the components to C
  comp_index_t computeStronglyConnectedComponents(Components & C);

  // Return the length (number of edges inside the Graph) of
  // the longest path connecting v to w
  // The result is 0 iff there is no such path
  // The result is 1 iff there is a shortest possible path
  vert_index_t computeLongestPathLength(const Vertex v,
                                        const Vertex w);

  // Return the length (number of edges inside the Graph) of 
  // the longest path connecting components c1 and c2
  // The result is 0 iff there is no such path
  // The result is 1 iff there is a shortest possible path
  vert_index_t computeConnectingPathBound(comp_index_t c1, comp_index_t c2);

  // Compute the length of the longest path
  // connecting 'Entrance' to the component c
  // The result is 0 iff there is no such path
  // The result is 1 iff there is a shortest possible path
  vert_index_t computeEntrancePathBound(comp_index_t c,
                                        const Component Entrance);

  // Compute the length of the longest path
  // connecting the component c to 'Exit'
  // The result is 0 iff there is no such path
  // The result is 1 iff there is a shortest possible path
  vert_index_t computeExitPathBound(comp_index_t c,
                                    const Component Exit);

  // Compute the length of the longest path
  // connecting 'Entrance' to 'Exit'
  // The result is 0 iff there is no such path
  // The result is 1 iff there is a shortest possible path
  vert_index_t computeThroughPathBound(const Component Entrance,
                                       const Component Exit);

private:

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
