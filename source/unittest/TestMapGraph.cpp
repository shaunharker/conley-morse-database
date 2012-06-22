#include <cppcutter.h>

#include <database/structures/MapGraph.h>
#include <chomp/Toplex.h>

#include <boost/scoped_ptr.hpp>
#include "LinearMaps.h"

namespace TestMapGraph {
using namespace chomp;

typedef MapGraph<Toplex, HyperbolicLinearMap, std::vector<GridElement> >
  HyperbolicMapGraph;

HyperbolicMapGraph* mapgraph;
Toplex* toplex;
HyperbolicLinearMap* map;

void AllCells(std::vector<std::vector<GridElement> > *cells, const Toplex& toplex) {
  cells->push_back(std::vector<GridElement>());
  toplex.cover(std::back_inserter(cells->at(0)), toplex.bounds());
}

void setup() {
  toplex = new Toplex(Rect(2, -1.0, 1.0));
  
  for (int i=0; i<3; i++)
    toplex->subdivide();
  
  map = new HyperbolicLinearMap();
  mapgraph = new HyperbolicMapGraph(*toplex, *map);
}

GridElement CubeAtOriginInFirstOctant(const Toplex& toplex) {
  std::vector<GridElement> cubes;
  toplex.cover(std::back_inserter(cubes), Rect(2, 0.00000001, 0.00000002));
  return cubes.front();
}

GridElement RootCube(const Toplex& toplex) {
  std::vector<GridElement> cubes;
  toplex.coarseCover(std::back_inserter(cubes), toplex.bounds());
  cppcut_assert_equal(1lu, cubes.size());
  return cubes.front();
}

void test_ctor() {
  cppcut_assert_equal(127u, mapgraph->sentinel());
  cppcut_assert_equal(127u, mapgraph->num_vertices());
}

void test_lookup_index() {
  GridElement a_grid = CubeAtOriginInFirstOctant(*toplex);
  cppcut_assert_equal(a_grid, mapgraph->lookup(mapgraph->index(a_grid)));
}


void test_adjacenies()
{
  GridElement root = RootCube(*toplex);
  std::vector<uint32_t> adjacencies_of_root_cube;
  std::vector<GridElement> children_of_root_cube;
  std::vector<uint32_t> children_indices_of_root_cube;
  
  adjacencies_of_root_cube = mapgraph->adjacencies(mapgraph->index(root));
  toplex->children(std::back_inserter(children_of_root_cube), root);
  mapgraph->index(&children_indices_of_root_cube, children_of_root_cube);
  cppcut_assert_equal(2lu, adjacencies_of_root_cube.size());
  cppcut_assert_equal(children_indices_of_root_cube[0],
                      adjacencies_of_root_cube[0]);
  cppcut_assert_equal(children_indices_of_root_cube[1],
                      adjacencies_of_root_cube[1]);

  std::vector<uint32_t> adjacencies;
  HyperbolicMapGraph::size_type a_grid =
      mapgraph->index(CubeAtOriginInFirstOctant(*toplex));
  adjacencies = mapgraph->adjacencies(a_grid);
  cppcut_assert_equal(10lu, adjacencies.size());
  cut_assert(1lu, std::count(adjacencies.begin(), adjacencies.end(), a_grid));
}

void test_leaves() {
  std::vector<GridElement> leaves_of_deepest_cube;
  std::vector<uint32_t> deepest_cube = {
    mapgraph->index(CubeAtOriginInFirstOctant(*toplex)),
  };
  
  mapgraph->leaves(&leaves_of_deepest_cube, deepest_cube);
  cppcut_assert_equal(1lu, leaves_of_deepest_cube.size());
  cppcut_assert_equal(CubeAtOriginInFirstOctant(*toplex),
                      leaves_of_deepest_cube[0]);

  std::vector<GridElement> leaves_of_root_cube;
  std::vector<uint32_t> root_cube = {
    mapgraph->index(RootCube(*toplex))
  };
  mapgraph->leaves(&leaves_of_root_cube, root_cube);
  cppcut_assert_equal(0lu, leaves_of_root_cube.size());
}

void teardown() {
  delete mapgraph;
  delete map;
  delete toplex;
}

}
