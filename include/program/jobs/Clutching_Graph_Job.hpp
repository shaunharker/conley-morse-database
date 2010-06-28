/*
 *  Clutching_Graph_Job.hpp
 */

#include "data_structures/Conley_Morse_Graph.h"
#include "distributed/Message.h"
#include <boost/foreach.hpp>


/** Compute a relation between two C-M graph
 *
 *  Return true if given two graphs has a same structure
 *  on the level of C-M graph.
 *
 *  NOTE: now graph connection structures are not used
 */
template<class CMGraph>
bool ClutchingTwoGraphs(
    const CMGraph &graph1,
    const CMGraph &graph2) {
  typedef typename CMGraph::Vertex Vertex;
  bool result = true;

  if (graph1.NumVertices() != graph2.NumVertices())
    result = false;
  
  BOOST_FOREACH (Vertex v1, graph1.Vertices()) {
    int n = 0;
    BOOST_FOREACH (Vertex v2, graph2.Vertices()) {
      if (Check_If_Intersect(*graph1.GetCubeSet(v1), *graph2.GetCubeSet(v2))) {
        n++;
        /* if two reccurent sets in one C-M graph intersect
           one reccurent set in other C-M graph, the two graphs does not
           share the "same" structure */
        if (n >= 2)
          result = false;
        result = result &&
                 (*graph1.GetConleyIndex(v1) == *graph2.GetConleyIndex(v2));
      }
    }
    /* if a reccurent set in one C-M graph intersects
       no reccurent set in other C-M graph, the two graphs does not
       share the "same" structure */
    if (n == 0)
      result = false;
  }
  return result;
}
    
template < class Toplex_Template >
const Message Clutching_Graph_Job ( const Message & job ) {
	/* Not implemented */
	return Message ();
}
