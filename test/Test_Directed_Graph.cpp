#include "data_structures/Directed_Graph.h"
#include <set>
#include <iostream>

struct Toplex 
{
  typedef int Top_Cell;
  typedef std::set<Top_Cell> Subset;
};

int main ( int argc, char * argv [] ) 
{
  typedef DirectedGraph <Toplex> MyGraph;

  int nVertices = 3;
  Toplex::Subset E[nVertices];

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

  std::vector< Toplex::Subset > Components;

  G.getStronglyConnectedComponents(Components);
  G.getConnectingPathLength(0, 0);
  G.getStablePathLength(0);

  std::cout << "The number of SCCs: " << Components.size() << std::endl;

  return 0;
}
