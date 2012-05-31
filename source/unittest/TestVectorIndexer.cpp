#include <cppcutter.h>
#include <chomp/VectorIndexer.h>
#include <vector>
#include <algorithm>
#include <functional>

namespace TestVectorIndexer
{

chomp::VectorIndexer<uint32_t, uint32_t> *indexer = NULL;
chomp::VectorIndexer<uint32_t, uint32_t> *rindexer = NULL;

void setup()
{
  uint32_t data[] = { 4, 1, 2, 0, 3 };
  std::vector<uint> input(data, data+5);
  indexer = new chomp::VectorIndexer<uint32_t, uint32_t>();
  rindexer = new chomp::VectorIndexer<uint32_t, uint32_t>();
  indexer->initialize(input);
  rindexer->initialize(input);
  rindexer->reindex(input);
}

void teardown()
{
  delete indexer;
  delete rindexer;
}

void test_size()
{
  cppcut_assert_equal((uint32_t)5, indexer->size());
  cppcut_assert_equal((uint32_t)5, rindexer->size());
}

void test_clear()
{
  indexer->clear();
  cppcut_assert_equal((uint32_t)0, indexer->size());
}

void test_rank()
{
  cppcut_assert_equal((uint32_t)3, indexer->rank(0));
  cppcut_assert_equal((uint32_t)1, indexer->rank(1));
  cppcut_assert_equal((uint32_t)0, indexer->rank(4));
}

void test_key()
{
  cppcut_assert_equal((uint32_t)0, indexer->key(3));
  cppcut_assert_equal((uint32_t)1, indexer->key(1));
  cppcut_assert_equal((uint32_t)4, indexer->key(0));
}

void test_rindex_rank()
{
  cppcut_assert_equal((uint32_t)0, rindexer->rank(0));
  cppcut_assert_equal((uint32_t)1, rindexer->rank(1));
  cppcut_assert_equal((uint32_t)4, rindexer->rank(4));
}

void test_rindex_key()
{
  cppcut_assert_equal((uint32_t)3, rindexer->key(3));
  cppcut_assert_equal((uint32_t)1, rindexer->key(1));
  cppcut_assert_equal((uint32_t)0, rindexer->key(0));
}

}
