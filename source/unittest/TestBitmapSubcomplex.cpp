#include <cppcutter.h>

#include <boost/scoped_ptr.hpp>

#include <chomp/CubicalComplex.h>
#include <chomp/BitmapSubcomplex.h>
#include <chomp/Generators.h>


namespace TestBitmapSubcomplex {
using namespace chomp;

typedef std::vector<uint32_t> c;

void SetupCubicalComplex(CubicalComplex *complex)
{
  complex->initialize(std::vector<uint32_t>{16, 16});
  // xxx
  // x x
  // xxx
  complex->addFullCube(c{0, 0});
  complex->addFullCube(c{0, 1});
  complex->addFullCube(c{0, 2});
  complex->addFullCube(c{1, 0});
  complex->addFullCube(c{1, 2});
  complex->addFullCube(c{2, 0});
  complex->addFullCube(c{2, 1});
  complex->addFullCube(c{2, 2});
  complex->finalize();
}

void InsertCube(BitmapSubcomplex *subcomplex,
                const CubicalComplex &base,
                const std::vector<uint32_t> &cube)
{
  int dim = cube.size();

  std::vector<std::vector<Index> > indexes = base.fullCubeIndexes(cube);
  for (int d=0; d<=dim; ++d)
    BOOST_FOREACH (const Index index, indexes[d]) {
      subcomplex->insert(index, d);
    }
}

BitmapSubcomplex *SetupBitmapSubcomplex(CubicalComplex *base)
{
  BitmapSubcomplex *subcomplex = new BitmapSubcomplex(*base, false);
  InsertCube(subcomplex, *base, c{2,2});
  InsertCube(subcomplex, *base, c{2,1});
  InsertCube(subcomplex, *base, c{1,2});
  subcomplex->finalize();
  return subcomplex;
}

void test_MorseGenerator()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);
  boost::scoped_ptr<BitmapSubcomplex> subcomplex(SetupBitmapSubcomplex(&complex));

  Generators_t subcomplex_generators = SmithGenerators(*subcomplex);

  cppcut_assert_equal(static_cast<size_t>(3), subcomplex_generators.size());
  cppcut_assert_equal(static_cast<size_t>(1), subcomplex_generators[0].size());
  cppcut_assert_equal(static_cast<size_t>(0), subcomplex_generators[1].size());
  cppcut_assert_equal(static_cast<size_t>(0), subcomplex_generators[2].size());
}
}
