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
  typedef std::vector<Component *> Components;

  // Constructor
  DirectedGraph() { };

  // Destructor
  ~DirectedGraph() { };

  // Remove Vertex v from the graph
  void removeVertex(Vertex);
      
  // Functional Notation when Directed Graph is considered as a Combinatorial Map
  typename Toplex::Subset operator () ( const typename Toplex::Top_Cell & );
  typename Toplex::Subset operator () ( const typename Toplex::Subset & );

private:
};

template < class Toplex, class Map >
DirectedGraph<Toplex> compute_directed_graph (const Toplex & my_toplex, 
                                              const Map & f);

template < class Toplex, class Map >
DirectedGraph<Toplex> compute_directed_graph (const typename Toplex::Subset & my_subset,
                                              const Toplex & my_toplex, 
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
size_t computeLongestPathLength(
    DirectedGraph<Toplex> & G,
    typename DirectedGraph<Toplex>::Vertex v,
    typename DirectedGraph<Toplex>::Vertex w);

// Function to compute the longest path lengths (of a fixed graph).
// Call with 'SCC', which stores the strongly connected components,
// and 'Entrance' set and 'Exit' set.
//
// This function computes the length of the following paths:
//
//   1. The longest direct path between components.
//      The resule will be stored in 'ConnectingPathBounds', which is
//      assumed to be a vector of length n * n (n = number of SCC).
//
//   2. The longest path from 'Entrance' to components.
//      The result will be stored in 'EntrancePathBounds', a vector
//      of length n.
//
//   3. The longest path from components to 'Exit'
//      The result will be stored in 'ExitPathBounds', a vector of
//      length n.
//
//   4. The longest path
//
// Remark: the 'length' is defined to be the number of edges inside
// the graph
template < class Toplex >
void computePathBounds(/* inputs */
                       const DirectedGraph<Toplex> & G,
                       const typename DirectedGraph<Toplex>::Components & SCC,
                       const typename Toplex::Subset & Entrance,
                       const typename Toplex::Subset & Exit,
                       /* outputs */
                       std::vector<size_t> & ConnectingPathBounds,
                       std::vector<size_t> & EntrancePathBounds,
                       std::vector<size_t> & ExitPathBounds,
                       size_t & ThruPathBound);

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Directed_Graph.hpp"
#endif

#endif
