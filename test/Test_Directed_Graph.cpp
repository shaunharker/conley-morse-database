#include "data_structures/Directed_Graph.h"
#include <set>
#include <iostream>

struct Toplex 
{
  typedef unsigned int size_type;
  typedef size_type Top_Cell;
  typedef std::set<Top_Cell> Subset;
};

void outputGraph(DirectedGraph <Toplex> G) {
  std::cout << "The size of the Graph: " << G.size() << std::endl; 
  DirectedGraph <Toplex>::iterator graph_it;
  DirectedGraph <Toplex>::Component::iterator edge_it;
  graph_it = G.begin();
  while (graph_it != G.end()) {
    std::cout << (*graph_it).first << " --> ";
    edge_it = ((*graph_it).second).begin();
    while (edge_it != ((*graph_it).second).end()) {
      std::cout << *edge_it << " ";
          ++edge_it;
    }
    ++graph_it;
    std::cout << std::endl;
  }
}

int main ( int argc, char * argv [] ) 
{
  typedef DirectedGraph <Toplex> Graph;

  int nVertices = 12;
  Toplex::Subset E[nVertices];

  /*
    The sample graph G has 12 vertices with edges defined by
    0  |--> 1
    1  |--> 2
    2  |--> 3, 5
    3  |--> 4, 7
    4  |--> 1
    5  |--> 5, 6
    6  |-->
    7  |--> 8, 9
    8  |--> 9
    9  |--> 7, 10
    10 |--> 11
    11 |--> 11
    Therefore, there should be four compones: {1234}, {5}, {789} and {11}
  */

  E[0].insert(1);
  E[1].insert(2);
  E[2].insert(3);
  E[2].insert(5);
  E[3].insert(4);
  E[3].insert(7);
  E[4].insert(1);
  E[5].insert(5);
  E[5].insert(6);
  E[7].insert(8);
  E[7].insert(9);
  E[8].insert(9);
  E[9].insert(7);
  E[9].insert(10);
  E[10].insert(11);
  E[11].insert(11);

  Graph G;
  for (int i = 0; i < nVertices; i++) {
    G.insert(Graph::value_type(i, E[i]));
  }
  outputGraph(G);
  
  Graph::Components SCC;
  SCC = computeSCC(G);
  std::cout << "The number of SCCs: " << SCC.size() << std::endl;

  std::vector< Toplex::Subset >::iterator it = SCC.begin();
  int i = 0;
  while (it != SCC.end()) {
    std::cout << "component(" << i << "): ";
    Toplex::Subset::iterator comp_it = (*it).begin();
    while (comp_it != (*it).end()) {
      std::cout << *comp_it << " ";;
      ++comp_it;
    }
    std::cout << std::endl;
    ++it;
    ++i;
  }

  Toplex::Subset Entrance;
  Toplex::Subset Exit;
  std::vector<size_t> ConnectingPathBounds(SCC.size() * SCC.size());
  std::vector<size_t> EntrancePathBounds(SCC.size());
  std::vector<size_t> ExitPathBounds(SCC.size());
  size_t ThruPathBound;

  Entrance.insert(0);
  Exit.insert(6);

  computePathBounds(G, SCC, Entrance, Exit,
                      ConnectingPathBounds,
                      EntrancePathBounds,
                      ExitPathBounds,
                      ThruPathBound);

  for (size_t i = 0; i < SCC.size(); i++) {
    for (size_t j = 0; j < SCC.size(); j++) {
      std::cout << "ConnectingPathBound(" << i << ", " << j << "): "
                << ConnectingPathBounds[i * SCC.size() + j] << std::endl;
    }
  }

  for (size_t i = 0; i < SCC.size(); i++) {
    std::cout << "EntrancePathBound(" << i << "): " << EntrancePathBounds[i] << std::endl;
  }

  for (size_t i = 0; i < SCC.size(); i++) {
    std::cout << "ExitPathBound(" << i << "): " << ExitPathBounds[i] << std::endl;
  }
  std::cout << "ThruPathBound: " << ThruPathBound << std::endl;

  return 0;
}
