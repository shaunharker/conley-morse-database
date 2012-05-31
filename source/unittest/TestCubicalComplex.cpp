#include <cppcutter.h>

#include <chomp/CubicalComplex.h>
#include <chomp/Generators.h>
#include <chomp/Closure.h>

#include <vector>

namespace TestCubicalComplex
{
using namespace chomp;

typedef std::vector<uint32_t> c;
typedef std::vector<double> v;

void SetupCubicalComplex(CubicalComplex *complex)
{
  complex->initialize(std::vector<uint32_t>{16, 16});
  complex->bounds() = Rect(2, -1.0, 1.0);
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

void SetupCubicalComplexWithTwoHoles(CubicalComplex *complex)
{
  complex->initialize(std::vector<uint32_t>{16, 16});
  complex->bounds() = Rect(2, -1.0, 1.0);
  // xxxx
  // x x x
  // xxxx
  complex->addFullCube(c{0, 0});
  complex->addFullCube(c{0, 1});
  complex->addFullCube(c{0, 2});
  complex->addFullCube(c{1, 0});
  complex->addFullCube(c{1, 2});
  complex->addFullCube(c{2, 0});
  complex->addFullCube(c{2, 1});
  complex->addFullCube(c{2, 2});
  complex->addFullCube(c{3, 0});
  complex->addFullCube(c{3, 2});
  complex->addFullCube(c{4, 1});
  complex->finalize();
}

void test_bounds()
{
  CubicalComplex complex;
  complex.bounds() = Rect(3, -1.0, 1.0); // set whole box as [-1.0,1.0]^3
  cppcut_assert_equal(-1.0, complex.bounds().lower_bounds[1]);
  cppcut_assert_equal(1.0, complex.bounds().upper_bounds[1]);
}

void test_cubeIndex_indexCube()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);

  c cube = complex.indexToCube(complex.cubeIndex(c{2,1}));
  cppcut_assert_equal(2lu, cube.size());
  cppcut_assert_equal(2u, cube[0]);
  cppcut_assert_equal(1u, cube[1]);
}

void test_geometry()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);
  // crush: cppcut_assert_equal(0u, complex.cubeIndex(c{3,3}));
  // 0:-1.0 1:-0.875 2:-0.75  4:-0.5  6:-0.25 8:0.0
  cppcut_assert_equal(Rect(2, v{-0.75, -0.875}, v{-0.625, -0.75}),
                      complex.geometryOfCube(c{2, 1}));
  cppcut_assert_equal(Rect(2, v{-0.75, -0.875}, v{-0.625, -0.75}),
                      complex.geometry(complex.cubeIndex(c{2, 1}), 2));
  /*
  cppcut_assert_equal(Rect(2, v{-0.75, -0.75}, v{-0.75, -0.625}),
                      complex.geometry(complex.cubeIndex(c{2, 1}), 1));
  cppcut_assert_equal(Rect(2, v{-0.625, -0.875}, v{-0.625, -0.875}),
                      complex.geometry(complex.cubeIndex(c{2, 1}), 0));
  */
}

void test_cubeIndex()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);

  cppcut_assert_not_equal(complex.cubeIndex(c{2,1}),
                          complex.cubeIndex(c{0,1}));
  cppcut_assert_equal(complex.cubeIndex(c{2,1}),
                      complex.cubeIndex(c{2,1}));
}

void test_indexToCell_cellToIndx()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);

  Index index = complex.cubeIndex(c{2, 2});
  cppcut_assert_equal(index,
                      complex.cellToIndex(complex.indexToCell(index, 2), 2));
}

template<typename GeneratorsComputer>
void TestGeneratorComputation(GeneratorsComputer compute)
{
  {
    CubicalComplex complex;
    SetupCubicalComplex(&complex);
    Generators_t generators = compute(complex);
    cppcut_assert_equal(static_cast<size_t>(3), generators.size());

    cppcut_assert_equal(static_cast<size_t>(1), generators[0].size());
    cppcut_assert_equal(0, generators[0][0].first.dimension());
    cppcut_assert_equal(Zp<2>(0), generators[0][0].second); // What this mean?
    
    cppcut_assert_equal(static_cast<size_t>(1), generators[1].size());
    cppcut_assert_equal(1, generators[1][0].first.dimension());
    cppcut_assert_equal(Zp<2>(0), generators[1][0].second); // What this mean?
    
    cppcut_assert_equal(static_cast<size_t>(0), generators[2].size());
  }
  {
    CubicalComplex complex;
    complex.initialize(std::vector<uint32_t>{16, 16});
    complex.addFullCube(c{1,1});
    complex.finalize();

    Generators_t generators = compute(complex);
    cppcut_assert_equal(static_cast<size_t>(3), generators.size());
    cppcut_assert_equal(static_cast<size_t>(1), generators[0].size());
    cppcut_assert_equal(static_cast<size_t>(0), generators[1].size());
    cppcut_assert_equal(static_cast<size_t>(0), generators[2].size());
  }

  {
    CubicalComplex complex;
    SetupCubicalComplexWithTwoHoles(&complex);
    Generators_t generators = compute(complex);
    cppcut_assert_equal(static_cast<size_t>(3), generators.size());
    cppcut_assert_equal(static_cast<size_t>(1), generators[0].size());
    cppcut_assert_equal(static_cast<size_t>(2), generators[1].size());
    cppcut_assert_equal(static_cast<size_t>(0), generators[2].size());
  }
}

void test_fullCubeIndexes()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);
  std::vector<std::vector<Index> > indexes = complex.fullCubeIndexes(c{2,1});
  cppcut_assert_equal(static_cast<size_t>(3), indexes.size());
  cppcut_assert_equal(static_cast<size_t>(4), indexes[0].size());
  cppcut_assert_equal(static_cast<size_t>(4), indexes[1].size());
  cppcut_assert_equal(static_cast<size_t>(1), indexes[2].size());
}

void test_SmithGenerators()
{
  TestGeneratorComputation<Generators_t (*)(const Complex &)>(SmithGenerators);
}

void test_MorseGenerators()
{
  TestGeneratorComputation<Generators_t (*)(Complex &)>(MorseGenerators);
}

void test_closure()
{
  CubicalComplex complex;
  SetupCubicalComplex(&complex);
  {
    std::vector<boost::unordered_set<Index> > cells(3);
    cells[2].insert(complex.cubeIndex(c{2,1}));
    closure(cells, complex);
    cppcut_assert_equal(static_cast<size_t>(4), cells[0].size());
    cppcut_assert_equal(static_cast<size_t>(4), cells[1].size());
    cppcut_assert_equal(static_cast<size_t>(1), cells[2].size());
  }

  {
    Chain chain;
    complex.boundary(&chain, complex.cubeIndex(c{2,1}), 2);
    std::vector<boost::unordered_set<Index> > cells(3);
    cells[1].insert(chain()[0].index());
    closure(cells, complex);
    cppcut_assert_equal(static_cast<size_t>(2), cells[0].size());
    cppcut_assert_equal(static_cast<size_t>(1), cells[1].size());
    cppcut_assert_equal(static_cast<size_t>(0), cells[2].size());
  }
}

}
