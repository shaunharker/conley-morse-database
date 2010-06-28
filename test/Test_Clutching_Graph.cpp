
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

int main(int argc, char *argv[])
{
  CMGraph cmgraph1;
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
  int toplexes4[4] = { 0, 1, 2, 1};
  int indices4[4] = { 0, 1, 2, 1};
  std::vector<CMGraph::Vertex> vs4;
  SetupCMGraph(&cmgraph4, &vs4, 4, toplexes4, indices4);
  
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph1) << std::endl;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph2) << std::endl;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph3) << std::endl;
  std::cout << ClutchingTwoGraphs(cmgraph2, cmgraph4) << std::endl;
  
  return 0;
}
