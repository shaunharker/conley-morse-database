
#include "data_structures/Conley_Morse_Graph.h"

int main ( int argc, char * argv [] ) {
  typedef ConleyMorseGraph<int, int> CMGraph;
  int n[4] = { 0, 1, 2, 3 };
  CMGraph::Vertex v[4];
  CMGraph cmgraph;

  for (int i=0; i<4; i++) {
    v[i] = cmgraph.AddVertex();
    cmgraph.SetCubeSet(v[i], n+i);
  }

  cmgraph.AddEdge(v[0], v[1]);
  cmgraph.AddEdge(v[0], v[2]);
  cmgraph.AddEdge(v[1], v[2]);
  cmgraph.AddEdge(v[2], v[3]);
  BOOST_FOREACH (CMGraph::Vertex x, cmgraph.OutEdges(v[0])) {
    std::cout << *cmgraph.GetCubeSet(x) << std::endl;
  }
  BOOST_FOREACH (CMGraph::Vertex x, cmgraph.InEdges(v[2])) {
    std::cout << *cmgraph.GetCubeSet(x) << std::endl;
  }
  BOOST_FOREACH (CMGraph::Edge e, cmgraph.Edges()) {
    std::cout << "(" << *cmgraph.GetCubeSet(cmgraph.Source(e)) << " "
              << *cmgraph.GetCubeSet(cmgraph.Target(e)) << ") " ;
  }
  std::cout << std::endl;
  
  std::cout << cmgraph.PathExist(v[0], v[1]) << std::endl;
  std::cout << cmgraph.PathExist(v[1], v[0]) << std::endl;
  std::cout << cmgraph.PathExist(v[0], v[3]) << std::endl;
  std::cout << cmgraph.PathExist(v[3], v[0]) << std::endl;

  return 0;
}
