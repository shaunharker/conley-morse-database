
bool Check_If_Intersect(int x, int y)
{
  return x==y;
}

#include "program/jobs/Clutching_Graph_Job.hpp"
#include <boost/iterator_adaptors.hpp>

/** A dummy Patch class for testing */
template<class CMGraph>
class DummyPatch {
 public:
  /* identifier of a parameter box, should be comparable */
  typedef int ParamBoxDescriptor;
  /* parameter box */
  typedef int ParameterBox;

  typedef boost::counting_iterator<size_t> ParamBoxIterator;
  typedef std::pair<ParamBoxIterator, ParamBoxIterator> ParamBoxIteratorPair;

  struct AdjParamBoxIterator:
      boost::iterator_adaptor<AdjParamBoxIterator,
                              int, std::pair<int, int>, 
                              boost::forward_traversal_tag, 
                              std::pair<int, int>,
                              int> {
    AdjParamBoxIterator(int n) 
        : boost::iterator_adaptor<AdjParamBoxIterator,
                                  int, std::pair<int, int>, 
                                  boost::forward_traversal_tag, 
                                  std::pair<int, int>, int>(n){};
    void increment(){ this->base_reference() += 1; }
    std::pair<int, int> dereference() const {
      return std::pair<int,int>(this->base(), this->base()+1);
    }
  };
  
  typedef std::pair<AdjParamBoxIterator, AdjParamBoxIterator> AdjParamBoxIteratorPair;

  template<class Iterator>
  DummyPatch(Iterator begin, Iterator end) : graphs_(begin, end) {}
  
  /** Return a pointer to C-M Graph related to that paramter */
  CMGraph* GetCMGraph(ParamBoxDescriptor d) const {
    return graphs_[d];
  }
  
  /** Return a pair of iterators for all parameter boxes */
  ParamBoxIteratorPair ParamBoxes() const {
    return ParamBoxIteratorPair(boost::make_counting_iterator((size_t)0),
                                boost::make_counting_iterator(graphs_.size()));
  }

  /** Return a pair of iterators for all pairs of adjacent parameter boxes */
  AdjParamBoxIteratorPair AdjecentBoxPairs() const {
    return AdjParamBoxIteratorPair(AdjParamBoxIterator((size_t)0),
                                   AdjParamBoxIterator(graphs_.size()-1));
  }

 private:
  std::vector<CMGraph*> graphs_;
};

typedef ConleyMorseGraph<int, int> CMGraph;

void SetupCMGraph(CMGraph *graph, std::vector<CMGraph::Vertex> *vertices,
                  int num_vertices, int* toplexes, int* indices) {
  for (int i=0; i<num_vertices; i++) {
    CMGraph::Vertex v = graph->AddVertex();
    vertices->push_back(v);
    graph->CubeSet(v) = toplexes[i];
    graph->ConleyIndex(v) = indices[i];
  }
}


void ShowVertexPairs(const CMGraph &graph1, const int *ptr1,
                     const CMGraph &graph2, const int *ptr2,
                     const VertexPairs<CMGraph> &pairs)
{
  BOOST_FOREACH(VertexPairs<CMGraph>::Pair pair, pairs.Pairs()) {
    std::cout << "(" << graph1.GetVertexIndex(pair.first) 
              << "," << graph2.GetVertexIndex(pair.second)
              << ")" << " ";
  }
  std::cout << std::endl;
}

int main(int argc, char *argv[])
{
  /* test for ClutchingTwoGraphs */
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

  /* true is assumed, because cmgraph4 has shuffled data of cmgraph1 */
  VertexPairs<CMGraph> pairs14;
  std::cout << ClutchingTwoGraphs(cmgraph1, cmgraph4, &pairs14) << std::endl;
  ShowVertexPairs(cmgraph1, toplexes1, cmgraph4, toplexes4, pairs14);

  /* false is assumed */
  VertexPairs<CMGraph> pairs24;
  std::cout << ClutchingTwoGraphs(cmgraph2, cmgraph4, &pairs24) << std::endl;
  ShowVertexPairs(cmgraph2, toplexes2, cmgraph4, toplexes4, pairs24);

  /* test for Union-Find */
  std::cout << "--" << std::endl;
  
  int data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 11};
  UnionFind<int> u(data, data+12);
  int data2[6] = {1, 8, 12, 13, 14, 15};
  UnionFind<int> v(data2, data2+6);
  std::vector<std::vector<int> > vect;
  
  u.Union(1, 2);
  u.Union(3, 4);
  std::cout << u.Find(1, 2) << std::endl;
  std::cout << u.Find(1, 3) << std::endl;
  u.Union(1, 4);
  std::cout << u.Find(1, 3) << std::endl;
  std::cout << u.Find(5, 6) << std::endl;

  v.Union(1, 8);
  v.Union(15, 12);

  u.Merge(v);
  std::cout << u.Find(2, 8) << std::endl;
  std::cout << u.Find(12, 15) << std::endl;
  std::cout << u.Find(12, 14) << std::endl;
  u.FillToVector(&vect);
  
  BOOST_FOREACH (std::vector<int> &t, vect) {
    std::cout << "(";
    BOOST_FOREACH (int& s, t) {
      std::cout << s << ",";
    }
    std::cout << ")";
  }
  std::cout << std::endl;
  
  /* test for ClutchingGraphs */
  std::cout << "--" << std::endl;
  CMGraph *graphs[8] = {
    &cmgraph1, &cmgraph1, &cmgraph4, &cmgraph2,
    &cmgraph2, &cmgraph1, &cmgraph3, &cmgraph3,
  };
  DummyPatch<CMGraph> dummy_patch(graphs, graphs+8);
  std::vector<std::vector<int> > ret;
  ClutchingGraph<CMGraph, DummyPatch<CMGraph> >(dummy_patch, &ret);

  BOOST_FOREACH (std::vector<int> &t, ret) {
    std::cout << "(";
    BOOST_FOREACH (int& s, t) {
      std::cout << s << ",";
    }
    std::cout << ")";
  }
  std::cout << std::endl;
  
  /* test for class Patch */
  std::cout << "--" << std::endl;
  int neighbour_array[8][9] = {
    { 1, 4, -1 },
    { 2, 5, -1 },
    { 3, 6, -1},
    { 7, -1},
    { 5, -1},
    { 6, -1},
    { 7, -1},
    { -1, },
  };

  std::vector<std::vector<size_t> > neighbours;
  
  for (int i=0; i<8; i++) {
    neighbours.push_back(std::vector<size_t>());
    for (int j=0; neighbour_array[i][j] != -1; j++) {
      neighbours.back().push_back(neighbour_array[i][j]);
    }
  }
  std::cout << "--" << std::endl;
  
  return 0;
}
