#include <cppcutter.h>

#include <chomp/Rect.h>

namespace TestRect
{
using namespace chomp;

void test_ctor()
{
  {
    Rect rect(3, -1.0, 1.0);
    cppcut_assert_equal((size_t)3, rect.lower_bounds.size());
    cppcut_assert_equal((size_t)3, rect.upper_bounds.size());
    for (int i=0; i<3; i++) {
      cppcut_assert_equal(-1.0, rect.lower_bounds[i]);
      cppcut_assert_equal(1.0, rect.upper_bounds[i]);
    }
  }
  {
    Rect rect(3,
              std::vector<double>({ -1.0, -2.0, -3.0}),
              std::vector<double>({ 1.0, 2.0, 3.0}));
    cppcut_assert_equal(-1.0, rect.lower_bounds[0]);
    cppcut_assert_equal(2.0, rect.upper_bounds[1]);
  }
}

void test_intersect()
{
  typedef std::vector<double> v;
  Rect rect1(2, v{-2, 2}, v{2, 3});
  Rect rect2(2, v{-1, -2}, v{1, 0});
  Rect rect3(2, v{0, -1}, v{3, 1});
  cut_assert(!rect1.intersects(rect2));
  cut_assert(!rect1.intersects(rect3));
  cut_assert(rect2.intersects(rect3));
}
}
