#include <cppcutter.h>

#include <chomp/Toplex.h>

namespace TestToplex {
using namespace chomp;

void test_ctor() {
  static const Rect whole_box = Rect(3, -1.0, 1.0);
  Toplex toplex(whole_box);
  
  cppcut_assert_equal(1u, toplex.size());
  cppcut_assert_equal(1u, toplex.tree_size());
  cppcut_assert_equal(3, toplex.dimension());

  GridElement toplevel = *toplex.begin();
  
  Toplex::const_iterator iter = toplex.begin();
  ++iter;
  cut_assert(iter == toplex.end());
  
  cppcut_assert_equal(whole_box, toplex.bounds());
  cppcut_assert_equal(whole_box, toplex.geometry(toplex.begin()));
  cppcut_assert_equal(whole_box, toplex.geometry(toplevel));
  
  cppcut_assert_equal(0, toplex.getDepth(*toplex.begin()));
  
  cut_assert(toplex.begin() == toplex.find(toplevel));
  
  std::vector<unsigned char> prefix = toplex.prefix(toplevel);
  cut_assert(prefix.empty());
  
  std::vector<GridElement> children;
  toplex.children(std::back_inserter(children), toplevel);
  cut_assert(children.empty());
}

void test_EraseTopCell() {
  Toplex toplex(Rect(3, -1.0, 1.0));
  toplex.erase(toplex.begin());
  cppcut_assert_equal(0u, toplex.size());
  cut_assert(toplex.begin() == toplex.end());
}

void test_SubdivideTopCell() {
  static const Rect whole_box = Rect(3, -1.0, 1.0);
  
  Toplex toplex(whole_box);
  std::vector<GridElement> new_elements;
  GridElement top_element = *toplex.begin();
  toplex.subdivide(std::back_inserter(new_elements), toplex.begin());
  cppcut_assert_equal(8u, toplex.size());
  cppcut_assert_equal(8lu, new_elements.size());
  cppcut_assert_equal(0, toplex.getDepth(top_element));
  cppcut_assert_equal(15u, toplex.tree_size()); // 15 = 1 + 2 + 2*2 + 2*2*2
  BOOST_FOREACH (GridElement element,  new_elements) {
    cppcut_assert_equal(1, toplex.getDepth(element));
    std::vector<unsigned char> prefix = toplex.prefix(element);
    cppcut_assert_equal(3lu, prefix.size());
  }
  std::vector<unsigned char> first_element_prefix = toplex.prefix(new_elements.front());
  cppcut_assert_equal(0, static_cast<int>(first_element_prefix[0]));
  cppcut_assert_equal(0, static_cast<int>(first_element_prefix[1]));
  cppcut_assert_equal(0, static_cast<int>(first_element_prefix[2]));

  std::vector<unsigned char> last_element_prefix = toplex.prefix(new_elements.back());
  cppcut_assert_equal(1, static_cast<int>(last_element_prefix[0]));
  cppcut_assert_equal(1, static_cast<int>(last_element_prefix[1]));
  cppcut_assert_equal(1, static_cast<int>(last_element_prefix[2]));

  std::vector<GridElement> children;
  toplex.children(std::back_inserter(children), top_element);
  cppcut_assert_equal(2lu, children.size());
  for (int i=0; i<2; i++) {
    std::vector<unsigned char> prefix = toplex.prefix(children[i]);
    cppcut_assert_equal(1lu, prefix.size());
    cppcut_assert_equal(i, static_cast<int>(prefix[0]));
  }

  std::vector<GridElement> leaves;
  std::vector<GridElement> only_toplevel(1, top_element);
  toplex.leaves(std::back_inserter(leaves), only_toplevel);
  cppcut_assert_equal(8lu, leaves.size());

  {
    //           A
    //     *           *
    //  *     *     *     * 
    // * *   * *   * *   * *
    // ambrella of {A} is {A}
    std::vector<GridElement> umbrella;
    toplex.umbrella(std::back_inserter(umbrella), only_toplevel);
    cppcut_assert_equal(1lu, umbrella.size());
    cppcut_assert_equal(top_element, umbrella[0]);
  }
  {
    //           A
    //     B           *
    //  C     *     *     * 
    // D *   * *   * *   * *
    // abmrbrella of {D} is {D, C, B, A}
    std::vector<GridElement> umbrella;
    std::vector<GridElement> elements = { new_elements[0] };
    toplex.umbrella(std::back_inserter(umbrella), elements);
    cppcut_assert_equal(4lu, umbrella.size());
    cppcut_assert_equal(new_elements[0], umbrella[0]);
    cppcut_assert_equal(top_element, umbrella[3]);
  }
  {
    //           A
    //     B           *
    //  C     *     *     * 
    // D E   * *   * *   * *
    // abmrbrella of {D,E} is {D, E, C, B, A}
    std::vector<GridElement> umbrella;
    std::vector<GridElement> elements = { new_elements[0], new_elements[1] };
    toplex.umbrella(std::back_inserter(umbrella), elements);
    cppcut_assert_equal(5lu, umbrella.size());
    cppcut_assert_equal(new_elements[0], umbrella[0]);
    cppcut_assert_equal(new_elements[1], umbrella[1]);
    cppcut_assert_equal(top_element, umbrella[4]);
  }
  {
    //           A
    //     B           *
    //  C     E     *     * 
    // D *   F *   * *   * *
    // abmrbrella of {D,F} is {D, F, C, E, B, A}
    std::vector<GridElement> umbrella;
    std::vector<GridElement> elements = { new_elements[0], new_elements[2] };
    toplex.umbrella(std::back_inserter(umbrella), elements);
    cppcut_assert_equal(6lu, umbrella.size());
    cppcut_assert_equal(new_elements[0], umbrella[0]);
    cppcut_assert_equal(new_elements[2], umbrella[1]);
    cppcut_assert_equal(top_element, umbrella[5]);
  }
  {
    //           A
    //     B           E
    //  C     *     F     * 
    // D *   * *   G *   * *
    // abmrbrella of {D,G} is {D, G, C, F, B, E, A}
    std::vector<GridElement> umbrella;
    std::vector<GridElement> elements = { new_elements[0], new_elements[5] };
    toplex.umbrella(std::back_inserter(umbrella), elements);
    cppcut_assert_equal(7lu, umbrella.size());
  }
  
  // In this case, Toplex::cover and Toplex::coarseCover return
  // the same result.
  {
    std::vector<GridElement> cover;
    typedef std::vector<double> v;
    toplex.cover(std::back_inserter(cover),
                 Rect(3, v{-0.8,-0.2,0.7}, v{-0.6,0.2,0.9}));
    cppcut_assert_equal(2lu, cover.size());
    cppcut_assert_equal(Rect(3, v{-1.0, -1.0, 0.0}, v{0.0, 0.0, 1.0}),
                        toplex.geometry(cover[0]));
    cppcut_assert_equal(Rect(3, v{-1.0, 0.0, 0.0}, v{0.0, 1.0, 1.0}),
                        toplex.geometry(cover[1]));
    
    std::vector<GridElement> coarse_cover;
    typedef std::vector<double> v;
    toplex.coarseCover(std::back_inserter(coarse_cover),
                       Rect(3, v{-0.8,-0.2,0.7}, v{-0.6,0.2,0.9}));
    cppcut_assert_equal(2lu, coarse_cover.size());
    cppcut_assert_equal(Rect(3, v{-1.0, -1.0, 0.0}, v{0.0, 0.0, 1.0}),
                        toplex.geometry(coarse_cover[0]));
    cppcut_assert_equal(Rect(3, v{-1.0, 0.0, 0.0}, v{0.0, 1.0, 1.0}),
                        toplex.geometry(coarse_cover[1]));
  }

  // In this case, Toplex::cover and Toplex::coarseCover return
  // different results.
  {
    std::vector<GridElement> cover;
    typedef std::vector<double> v;
    toplex.cover(std::back_inserter(cover), whole_box);
    cppcut_assert_equal(8lu, cover.size());
    
    std::vector<GridElement> coarse_cover;
    typedef std::vector<double> v;
    toplex.coarseCover(std::back_inserter(coarse_cover), whole_box);
    cppcut_assert_equal(1lu, coarse_cover.size());
    cppcut_assert_equal(top_element, coarse_cover[0]);
  }

  {
    std::vector<std::vector<uint32_t> > cubes;
    toplex.GridElementToCubes(&cubes, top_element, 1);
    cppcut_assert_equal(8lu, cubes.size());
    BOOST_FOREACH (const std::vector<uint32_t> & cube, cubes) {
      cppcut_assert_equal(3lu, cube.size());
      BOOST_FOREACH (uint32_t n, cube) {
        cut_assert(n == 0 || n == 1);
      }
    }
  }

  // Toplex::GridElementToCubes and Toplex::prefix
  {
    std::vector<std::vector<uint32_t> > cubes;
    toplex.GridElementToCubes(&cubes, top_element, 1);
    for (int i=0; i<8; ++i) {
      std::vector<unsigned char> prefix = toplex.prefix(new_elements[i]);
      for (int j=0; j<3; j++)
        cut_assert(cubes[i][j] == prefix[2-j]);
    }
  }

  {
    std::vector<GridElement> elements = { new_elements[0] };
    toplex.coarsen(elements);
    cppcut_assert_equal(8u, toplex.size());
  }
  {
    std::vector<GridElement> elements = { new_elements[0], new_elements[1] };
    toplex.coarsen(elements);
    cppcut_assert_equal(8u, toplex.size());
  }
  {
    std::vector<GridElement> elements = { top_element };
    toplex.coarsen(elements);
    cppcut_assert_equal(1u, toplex.size());
  }
}

void test_subdivide_subdivide()
{
  Toplex toplex(Rect(3, -1.0, 1.0));
  GridElement top_element = *toplex.begin();

  std::vector<GridElement> level1_elements;
  toplex.subdivide(std::back_inserter(level1_elements), top_element);
  std::vector<GridElement> level2_elements;
  toplex.subdivide(std::back_inserter(level2_elements), level1_elements[7]);
  cppcut_assert_equal(15u, toplex.size()); // 15 = (2*2*2 - 1) + 2*2*2
  cppcut_assert_equal(29u, toplex.tree_size()); // 29 = 1 + 2 + 2*2 + 2*2*2 + 2 + 2*2 + 2*2*2
  BOOST_FOREACH (GridElement e, level2_elements) {
    std::vector<unsigned char> prefix = toplex.prefix(e);
    cppcut_assert_equal(6lu, prefix.size());
    BOOST_FOREACH (unsigned char p, prefix) {
      cut_assert(p == 0 || p == 1);
    }
  }

  BOOST_FOREACH (GridElement e, level2_elements) {
    std::vector<std::vector<uint32_t> > cubes;
    toplex.GridElementToCubes(&cubes, e, 2);
    cppcut_assert_equal(1lu, cubes.size());
    cppcut_assert_equal(3lu, cubes[0].size());
  }

  {
    std::vector<std::vector<uint32_t> > cubes;
    toplex.GridElementToCubes(&cubes, level2_elements[1], 2);
    cppcut_assert_equal(2u, cubes[0][0]);
    cppcut_assert_equal(2u, cubes[0][1]);
    cppcut_assert_equal(3u, cubes[0][2]);
  }
}
}
