#include "data_structures/Directed_Graph.h"
#include <set>
#include <iostream>

int main ( int argc, char * argv [] ) 
{
  typedef int Topcell;
  typedef std::set<Topcell> Toplex_Subset;
  typedef DirectedGraph <Topcell, Toplex_Subset> MyGraph;

  int nVertices = 3;
  Toplex_Subset E[nVertices];

  /*
    The sample graph G has 3 vertices with edges defined by
    0 |--> 0, 1
    1 |--> 2
    2 |--> 2
    Therefore, there should be two compones of scc(G), namely, {0} and {2}  
   */

  E[0].insert(0);
  E[0].insert(1);
  E[1].insert(2);
  E[2].insert(2);

  MyGraph G;
  for (int i = 0; i < nVertices; i++) {
    G.insert(MyGraph::value_type(i, E[i]));
  }
  std::cout << "The size of the Graph: " << G.size() << std::endl; 

  std::vector< Toplex_Subset > Components;

  G.getStronglyConnectedComponents(Components);
  G.getConnectingPathLength(0, 0);
  G.getStablePathLength(0);

  std::cout << "The number of SCCs: " << Components.size() << std::endl;

  return 0;
}
