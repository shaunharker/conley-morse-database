
#include <chomp/Toplex.h>
#include <database/structures/MapGraph.h>
#include <database/structures/Conley_Morse_Graph.h>
#include <chomp/ConleyIndex.h>
#include <database/algorithms/GraphTheory.h>

#include <cppcutter.h>

#include <boost/foreach.hpp>

#include "LinearMaps.h"

namespace TestGraphTheory {
using namespace chomp;

typedef std::vector<GridElement> CellContainer;
typedef MapGraph<Toplex, HyperbolicLinearMap, CellContainer> Graph;
typedef ConleyMorseGraph<CellContainer, ConleyIndex_t> CMGraph;

void TestCellsOfMorseComponent(const Toplex& toplex, const CellContainer& cells) {
  cppcut_assert_equal(4lu, cells.size());
  BOOST_FOREACH (const GridElement element, cells) {
    Rect g = toplex.geometry(element);
    cut_assert((g.upper_bounds[0] == 0.0 && g.upper_bounds[1] == 0.0) ||
               (g.lower_bounds[0] == 0.0 && g.lower_bounds[1] == 0.0) ||
               (g.upper_bounds[0] == 0.0 && g.lower_bounds[1] == 0.0) ||
               (g.lower_bounds[0] == 0.0 && g.upper_bounds[1] == 0.0));
  }
}

void test_compute_morse_sets() {
  Toplex toplex(Rect(2, -1, 1));
  for (int i=0; i<4; i++)
    toplex.subdivide();
  std::vector<CellContainer> all_cells(1, CellContainer());
  toplex.cover(std::back_inserter(all_cells[0]), toplex.bounds());

  HyperbolicLinearMap map;
  Graph graph(all_cells, toplex, map);
  
  std::vector<CellContainer> morse_sets;
  CMGraph cmgraph;
  compute_morse_sets<CMGraph, Graph, CellContainer>(&morse_sets, graph, &cmgraph);

  cppcut_assert_equal(1lu, morse_sets.size());
  TestCellsOfMorseComponent(toplex, morse_sets[0]);

  cppcut_assert_equal(1u, cmgraph.NumVertices());
  CMGraph::Vertex morse_component = *cmgraph.Vertices().first;
  TestCellsOfMorseComponent(toplex, cmgraph.CellSet(morse_component));

  // Conley index is empty. If you want Conley index, you need to call
  // ConleyIndex(&cmgraph.ConleyIndex(morse_component), toplex,
  //             cmgraph.CellSet(morse_component), map);
  cppcut_assert_equal(0lu, cmgraph.ConleyIndex(morse_component).data().size());
}
    
}
