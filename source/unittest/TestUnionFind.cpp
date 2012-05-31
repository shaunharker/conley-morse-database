#include <cppcutter.h>
#include <database/structures/UnionFind.hpp>
#include <vector>
#include <algorithm>
#include <functional>

namespace test_UnionFind
{
void test_Representative()
{
  int input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  UnionFind<int> union_find(input, input+10);

  cppcut_assert_equal(3, union_find.Representative(3));
}

void test_Union_and_Find()
{
  int input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  UnionFind<int> union_find(input, input+10);

  cut_assert_false(union_find.Find(2,3));
  union_find.Union(2, 3);
  cut_assert(union_find.Find(2,3));

  cut_assert_false(union_find.Find(2,4));
  union_find.Union(3,4);
  cut_assert(union_find.Find(2,4));
}

void test_Add_Union_Find()
{
  int input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  UnionFind<int> union_find(input, input+10);

  union_find.Union(9, 2);
  union_find.Add(10);
  union_find.Add(11);
  union_find.Add(12);
  union_find.Union(11, 10);
  union_find.Union(11, 9);
  cut_assert(union_find.Find(9, 10));
  cut_assert(union_find.Find(2, 10));
  cut_assert_false(union_find.Find(10, 12));
}

void test_Merge()
{
  int input1[] = { 0, 1, 2, 3, 4};
  int input2[] = { 0, 1, 2, 6, 7};
  UnionFind<int> union_find1(input1, input1+5);
  UnionFind<int> union_find2(input2, input2+5);

  union_find1.Union(1, 3);
  union_find2.Union(1, 2);
  union_find2.Union(2, 6);
  union_find1.Merge(union_find2);
  cut_assert(union_find1.Find(3, 6));
  cut_assert_false(union_find1.Find(3, 7));
}

void test_FillToVector()
{
  int input[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  UnionFind<int> union_find(input, input+10);

  union_find.Union(0, 1);
  union_find.Union(1, 2);
  union_find.Union(4, 5);
  union_find.Union(8, 9);
  union_find.Union(6, 7);
  union_find.Union(6, 9);

  std::vector<std::vector<int> > output;
  union_find.FillToVector(&output);

  cppcut_assert_equal(4, (int)output.size());

  std::vector<int> sizes;
  for (int i=0; i<4; i++)
    sizes.push_back(output[i].size());
  std::sort(sizes.begin(), sizes.end());
  
  cppcut_assert_equal(1, sizes[0]);
  cppcut_assert_equal(2, sizes[1]);
  cppcut_assert_equal(3, sizes[2]);
  cppcut_assert_equal(4, sizes[3]);
}

}
