#include <cppcutter.h>

#include <chomp/ConleyIndex.h>
#include <chomp/Toplex.h>

#include "LinearMaps.h"

namespace TestConleyIndex
{
using namespace chomp;

void test_ConleyIndex()
{
  Toplex toplex(Rect(2, -1.0, 1.0));
  toplex.subdivide();
  toplex.subdivide();
  toplex.subdivide();
  toplex.subdivide();
  
  std::vector<GridElement> center_of_four_cubes;
  toplex.cover(std::back_inserter(center_of_four_cubes), Rect(2, -0.0001, 0.0001));

  cppcut_assert_equal(4lu, center_of_four_cubes.size());

  {
    HyperbolicLinearMap F;
    ConleyIndex_t conley_index;
    
    ConleyIndex(&conley_index, toplex, center_of_four_cubes, F);
    
    cppcut_assert_equal(3lu, conley_index.data().size());
    cppcut_assert_equal(0, conley_index.data()[0].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[0].number_of_columns());
    cppcut_assert_equal(1, conley_index.data()[1].number_of_rows());
    cppcut_assert_equal(1, conley_index.data()[1].number_of_columns());
    cppcut_assert_equal(0, conley_index.data()[2].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[2].number_of_columns());
    cppcut_assert_equal(Zp<2>(1), conley_index.data()[1].read(0,0));
  }
  {
    ContractingLinearMap F;
    ConleyIndex_t conley_index;
    
    ConleyIndex(&conley_index, toplex, center_of_four_cubes, F);
    
    cppcut_assert_equal(3lu, conley_index.data().size());
    cppcut_assert_equal(1, conley_index.data()[0].number_of_rows());
    cppcut_assert_equal(1, conley_index.data()[0].number_of_columns());
    cppcut_assert_equal(0, conley_index.data()[1].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[1].number_of_columns());
    cppcut_assert_equal(0, conley_index.data()[2].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[2].number_of_columns());

    cppcut_assert_equal(Zp<2>(1), conley_index.data()[0].read(0,0));
  }

  {
    ExpandingLinearMap F;
    ConleyIndex_t conley_index;
    
    ConleyIndex(&conley_index, toplex, center_of_four_cubes, F);
    
    cppcut_assert_equal(3lu, conley_index.data().size());
    cppcut_assert_equal(0, conley_index.data()[0].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[0].number_of_columns());
    cppcut_assert_equal(0, conley_index.data()[1].number_of_rows());
    cppcut_assert_equal(0, conley_index.data()[1].number_of_columns());
    cppcut_assert_equal(1, conley_index.data()[2].number_of_rows());
    cppcut_assert_equal(1, conley_index.data()[2].number_of_columns());

    cppcut_assert_equal(Zp<2>(1), conley_index.data()[2].read(0,0));
  }
  
}

}
