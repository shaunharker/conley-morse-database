
bool Check_If_Intersect(int x, int y)
{
  return x==y;
}

#include "program/jobs/Clutching_Graph_Job.hpp"
typedef ConleyMorseGraph<int, int> CMGraph;

void SetupCMGraph(CMGraph *graph, std::vector<CMGraph::Vertex> *vertices,
                  int num_vertices, int* toplexes, int* indices) {
  for (int i=0; i<num_vertices; i++) {
    CMGraph::Vertex v = graph->AddVertex();
    vertices->push_back(v);
    graph->SetCubeSet(v, toplexes+i);
    graph->SetConleyIndex(v, indices+i);
  }
}


void ShowVertexPairs(const CMGraph &graph1, const int *ptr1,
                     const CMGraph &graph2, const int *ptr2,
                     const VertexPairs<CMGraph> &pairs)
{
  BOOST_FOREACH(VertexPairs<CMGraph>::Pair pair, pairs.Pairs()) {
    std::cout << "(" << graph1.GetCubeSet(pair.first) - ptr1
              << "," << graph2.GetCubeSet(pair.second) - ptr2
              << ")" << " ";
  }
  std::cout << std::endl;
}

int main(int argc, char *argv[])
{
  CMGraph cmgraph1;
  typedef VertexPairs<CMGraph> Pairs;
  
  int toplexes1[4] = { 0, 1, 2, 3 };
  int indices1[4] = { 0, 1, 2, 3};
  
  std::vector<CMGraph::Vertex> vs1;
  SetupCMGraph(&cmgraph1, &vs1, 4, toplexes1, indices1);

  CMGraph cmgraph2;
  int toplexes2[4] = { 0, 1, 2, 1};
  int indices2[4] = { 0, 1, 2, 1};
  std::vector<CMGraph::Vertex> vs2;
  SetupCMGraph(&cmgraph2, &vs2, 4, toplexes2, indices2);

  CMGraph cmgraph3;
  int toplexes3[5] = { 0, 1, 2, 3, 4};
  int indices3[5] = { 0, 1, 2, 3, 4};
  std::vector<CMGraph::Vertex> vs3;
  SetupCMGraph(&cmgraph3, &vs3, 5, toplexes3, indices3);

  CMGraph cmgraph4;
  int toplexes4[4] = { 3, 0, 1, 2};
  int indices4[4] = { 3, 0, 1, 2};
  std::vector<CMGraph::Vertex> vs4;
  SetupCMGraph(&cmgraph4, &vs4, 4, toplexes4, indices4);

  /* true is assumed, because two graph is the same  */
  VertexPairs<CMGraph> pairs11;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph1, &pairs11) << std::endl;
  ShowVertexPairs(cmgraph1, toplexes1, cmgraph1, toplexes1, pairs11);

  /* false is assumed */
  VertexPairs<CMGraph> pairs12;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph2, &pairs12) << std::endl;
  ShowVertexPairs(cmgraph1, toplexes1, cmgraph2, toplexes2, pairs12);

  /* false is assumed, because cmgraph3 is larger than cmgraph1 */
  VertexPairs<CMGraph> pairs13;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph3, &pairs13) << std::endl;
  ShowVertexPairs(cmgraph1, toplexes1, cmgraph3, toplexes3, pairs13);

  /* true is assumed, because cmgraph4 is shuffle of  cmgraph1 */
  VertexPairs<CMGraph> pairs14;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph4, &pairs14) << std::endl;
  ShowVertexPairs(cmgraph1, toplexes1, cmgraph4, toplexes4, pairs14);

  /* false is assumed */
  VertexPairs<CMGraph> pairs24;
  std::cout << ClutchingTwoGraphs(cmgraph2, cmgraph4, &pairs24) << std::endl;
  ShowVertexPairs(cmgraph2, toplexes2, cmgraph4, toplexes4, pairs24);
  
  std::cout << "--" << std::endl;
  
  int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  UnionFind<int> u(data, data+10);
  int data2[6] = {1, 8, 12, 13, 14, 15};
  UnionFind<int> v(data2, data2+6);
  
  u.Union(1, 2);
  u.Union(3, 4);
  std::cout << u.Find(1, 2) << std::endl;
  std::cout << u.Find(1, 3) << std::endl;
  u.Union(1, 4);
  std::cout << u.Find(1, 3) << std::endl;
  std::cout << u.Find(5, 6) << std::endl;

  v.Union(1, 8);
  v.Union(12, 15);

  u.Merge(v);
  std::cout << u.Find(2, 8) << std::endl;
  std::cout << u.Find(12, 15) << std::endl;
  std::cout << u.Find(12, 14) << std::endl;
  
  return 0;
}
