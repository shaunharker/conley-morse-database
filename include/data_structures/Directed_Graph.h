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

  // Constructor
  DirectedGraph() { };

  // Destructor
  ~DirectedGraph() { };

  // Functional Notation when Directed Graph is considered as a Combinatorial Map
  typename Toplex::Subset operator () ( const typename Toplex::Top_Cell & );
  typename Toplex::Subset operator () ( const typename Toplex::Subset & );

private:
};

template < class Toplex, class Map >
DirectedGraph<Toplex> compute_directed_graph (const Toplex & my_toplex, 
                                              const Map & f);

// Collapse components
template < class Toplex >
DirectedGraph<Toplex> collapseComponents (
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Components & Components,
    std::vector<typename DirectedGraph<Toplex>::Vertex> & Representatives);

// Compute SCC using boost
template < class Toplex >
typename DirectedGraph<Toplex>::Components computeSCC (
    DirectedGraph<Toplex> & G);

// Return the length (number of edges inside the Graph) of the
// longest path connecting v to w, assuming the graph is acyclic.
// The result is -1 iff there is no such path, 0 iff v = w, etc.
template < class Toplex >
long computeLongestPathLength(
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Vertex v,
    typename DirectedGraph<Toplex>::Vertex w);

// void computePathBounds(Graph G,
//                        const Components SCC,
//                        const typename Toplex::Subset Entrance,
//                        const typename Toplex::Subset Exit,
//                        vector<long> & ConnectingPathBounds,
//                        vector<long> & EntrancePathBounds,
//                        vector<long> & ExitPathBounds,
//                        long ThruPathBounds);

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Directed_Graph.hpp"
#endif

#endif
