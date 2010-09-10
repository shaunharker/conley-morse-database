
#include "data_structures/Conley_Morse_Graph.h"
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

int main ( int argc, char * argv [] ) {
  typedef ConleyMorseGraph<int, int> CMGraph;
  int n[4] = { 0, 1, 2, 3 };
  CMGraph::Vertex v[4];
  CMGraph cmgraph;

  for (int i=0; i<4; i++) {
    v[i] = cmgraph.AddVertex();
    cmgraph.CubeSet(v[i]) = n[i];
  }

  cmgraph.AddEdge(v[0], v[1]);
  cmgraph.AddEdge(v[0], v[2]);
  cmgraph.AddEdge(v[1], v[2]);
  cmgraph.AddEdge(v[2], v[3]);
  BOOST_FOREACH (CMGraph::Vertex x, cmgraph.OutEdges(v[0])) {
    std::cout << cmgraph.CubeSet(x) << std::endl;
  }
  BOOST_FOREACH (CMGraph::Vertex x, cmgraph.InEdges(v[2])) {
    std::cout << cmgraph.CubeSet(x) << std::endl;
  }
  BOOST_FOREACH (CMGraph::Edge e, cmgraph.Edges()) {
    std::cout << "(" << cmgraph.CubeSet(cmgraph.Source(e)) << " "
              << cmgraph.CubeSet(cmgraph.Target(e)) << ") " ;
  }
  std::cout << std::endl;
  
  std::cout << cmgraph.PathExists(v[0], v[1]) << std::endl;
  std::cout << cmgraph.PathExists(v[1], v[0]) << std::endl;
  std::cout << cmgraph.PathExists(v[0], v[3]) << std::endl;
  std::cout << cmgraph.PathExists(v[3], v[0]) << std::endl;

#if 0
  std::ofstream dummy_out("/dev/null");
  boost::archive::text_oarchive oa(dummy_out);
  //boost::shared_ptr<std::string> x = boost::shared_ptr<std::string>(new std::string());
  //oa << x;
  oa << cmgraph;
  dummy_out.close();
#endif
  
  return 0;
}
