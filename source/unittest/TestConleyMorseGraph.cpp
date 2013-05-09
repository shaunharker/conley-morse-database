#include <cppcutter.h>

#include <database/structures/MorseGraph.h>

namespace TestConleyMorseGraph {

typedef ConleyMorseGraph<std::string, char> CMG;

void test_NumVertices() {
  CMG cmgraph;

  cppcut_assert_equal(0u, cmgraph.NumVertices());
  cmgraph.AddVertex();
  cmgraph.AddVertex();
  cppcut_assert_equal(2u, cmgraph.NumVertices());
}

unsigned int NumEdges(const CMG &cmg) {
  unsigned int n=0;
  BOOST_FOREACH (const CMG::Edge& e, cmg.Edges()) {
    (void)(&e);
    n++;
  }
  return n;
}

void test_AddEdeges() {
  CMG cmgraph;
  CMG::Vertex v1, v2;
  
  cppcut_assert_equal(0u, NumEdges(cmgraph));
  v1 = cmgraph.AddVertex();
  v2 = cmgraph.AddVertex();
  cppcut_assert_equal(0u, NumEdges(cmgraph));
  cmgraph.AddEdge(v1, v2);
  cppcut_assert_equal(1u, NumEdges(cmgraph));
  cmgraph.AddEdge(v1, v2);
  cppcut_assert_equal(1u, NumEdges(cmgraph));

}

void test_CellSet() {
  CMG cmgraph;
  CMG::Vertex v1, v2;
  
  v1 = cmgraph.AddVertex();
  v2 = cmgraph.AddVertex();
  
  cmgraph.CellSet(v1) = "abc";
  cmgraph.CellSet(v2) = "def";
  cppcut_assert_equal(std::string("abc"), cmgraph.CellSet(v1));
  cppcut_assert_equal(std::string("def"), cmgraph.CellSet(v2));
}

void test_ConleyIndex() {
  CMG cmgraph;
  CMG::Vertex v1, v2;
  
  v1 = cmgraph.AddVertex();
  v2 = cmgraph.AddVertex();
  
  cmgraph.ConleyIndex(v1) = 'd';
  cmgraph.ConleyIndex(v2) = 'k';
  cppcut_assert_equal('d', cmgraph.ConleyIndex(v1));
  cppcut_assert_equal('k', cmgraph.ConleyIndex(v2));
}

}
