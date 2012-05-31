#include <cppcutter.h>
#include <database/structures/UnionFind.hpp>
#include <chomp/Field.h>

namespace TestField
{
using chomp::Zp;

void test_equal()
{
  cut_assert(Zp<7>(0) == Zp<7>(0));
  cut_assert(Zp<7>(-1) == Zp<7>(6));
  cut_assert(Zp<7>(2) == 2);
}

void test_arith()
{
  cppcut_assert_equal(Zp<7>(3), Zp<7>(6) + Zp<7>(4));
  cppcut_assert_equal(Zp<7>(2), Zp<7>(6) - Zp<7>(4));
  cppcut_assert_equal(Zp<7>(6), Zp<7>(5) * Zp<7>(4));
  cppcut_assert_equal(Zp<7>(5), Zp<7>(3) / Zp<7>(2));
  cppcut_assert_equal(Zp<7>(5), inverse(Zp<7>(3)));
  cppcut_assert_equal(Zp<7>(3), -Zp<7>(4));
}

void test_outputstream()
{
  std::stringstream out("");
  out << Zp<7>(3);
  cppcut_assert_equal(std::string("3"), out.str());
}
}
