#include <cppcutter.h>
#include <chomp/HashIndexer.h>
#include <string>

namespace TestHashIndexer
{

chomp::HashIndexer<std::string, uint32_t> *indexer = NULL;

void setup()
{
  const std::vector<std::string> data = { "a", "b", "c", "d", "e" };
  indexer = new chomp::HashIndexer<std::string, uint32_t>();
  indexer->initialize(data);
}

void teardown()
{
  delete indexer;
}

void test_size()
{
  cppcut_assert_equal((uint32_t)5, indexer->size());
}

void test_rank()
{
  cppcut_assert_equal((uint32_t)1, indexer->rank("b"));
  cppcut_assert_equal((uint32_t)5, indexer->rank("f"));
}

void test_key()
{
  cppcut_assert_equal(std::string("c"), indexer->key(2));
}

void test_reindex()
{
  const std::vector<uint32_t> permute = {0, 4, 1, 2, 3};
  indexer->reindex(permute);
  cppcut_assert_equal((uint32_t)0, indexer->rank("a"));
  cppcut_assert_equal((uint32_t)4, indexer->rank("b"));
  cppcut_assert_equal((uint32_t)2, indexer->rank("d"));
}

}
