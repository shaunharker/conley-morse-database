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
  typedef typename Toplex::Subset Component;
  typedef std::vector<Component> Components;
  typedef typename Components::size_type comp_index_t;

  // Constructor
  DirectedGraph() { };

  // Destructor
  ~DirectedGraph() { };

  // Return the number of SCC and store components to C
  comp_index_t getStronglyConnectedComponents(Components & C);

  // Compute an upper bound on the connecting path length
  // between components c1 and c2. The result is 0 if there 
  // is no such connecting orbit.
  comp_index_t getConnectingPathLength(comp_index_t c1, comp_index_t c2);

  // Compute an upper bound on the length of a path in the 
  // complement of the union of the SCCs and whose last vertex 
  // is in the component c.
  comp_index_t getStablePathLength(comp_index_t c);

private:

};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Directed_Graph.hpp"
#endif

#endif
