#define RINGDEFINED
#include <chomp/Field.h>
namespace chomp {
typedef Zp<7> Ring;
}
#include <chomp/Chain.h>
#include <cppcutter.h>

namespace TestTerm
{
using namespace chomp;

void test_ctor()
{
  {
    Term term((Index)0, Zp<7>(0));
    cppcut_assert_equal((Index)0, term.index());
    cppcut_assert_equal(Zp<7>(0), term.coef());
  }
  {
    Term term(std::make_pair((Index)2, Zp<7>(-1)));
    cppcut_assert_equal((Index)2, term.index());
    cppcut_assert_equal(Zp<7>(-1), term.coef());
  }
}

void test_equal()
{
  {
    Term term1((Index)2, Zp<7>(0)), term2((Index)3, Zp<7>(0));
    cppcut_assert_operator(term1, !=, term2);
  }
  {
    Term term1((Index)2, Zp<7>(8)), term2((Index)2, Zp<7>(8));
    cppcut_assert_operator(term1, ==, term2);
  }
}

}

namespace TestChain
{
using namespace chomp;

// Return a chain of "[1 mod 7][2] + [2 mod 7][1]"
Chain setup_chain1()
{
  Chain chain;
  chain += Term(2, Zp<7>(1));
  chain += Term(1, Zp<7>(2));
  return chain;
}

// Return a chain of "[3 mod 7][2] + [1 mod 7][3]"
Chain setup_chain2()
{
  Chain chain;
  chain += Term(2, Zp<7>(3));
  chain += Term(3, Zp<7>(1));
  return chain;
}

void test_dimension()
{
  Chain chain;
  chain.dimension() = 2;
  cppcut_assert_equal(2, chain.dimension());
}

void test_plusassign_minusassign_Term()
{
  Chain chain;
  chain += Term(2, Zp<7>(1));
  chain += Term(1, Zp<7>(2));
  chain -= Term(2, Zp<7>(1));
  
  cppcut_assert_equal(Term(2, Zp<7>(1)), chain()[0]);
  cppcut_assert_equal(Term(1, Zp<7>(2)), chain()[1]);
  cppcut_assert_equal(Term(2, Zp<7>(6)), chain()[2]);
}

void test_plusassign_Chain()
{
  {
    Chain chain1 = setup_chain1();
    Chain chain2 = setup_chain2();
    chain1 += chain2;
    
    cppcut_assert_equal(Term(2, Zp<7>(1)), chain1()[0]);
    cppcut_assert_equal(Term(1, Zp<7>(2)), chain1()[1]);
    cppcut_assert_equal(Term(2, Zp<7>(3)), chain1()[2]);
    cppcut_assert_equal(Term(3, Zp<7>(1)), chain1()[3]);
  }
  {
    Chain chain = setup_chain1();
    chain += chain;
    cppcut_assert_equal((size_t)4, chain().size());
    cppcut_assert_equal(Term(2, Zp<7>(1)), chain()[0]);
    cppcut_assert_equal(Term(1, Zp<7>(2)), chain()[1]);
    cppcut_assert_equal(Term(2, Zp<7>(1)), chain()[2]);
    cppcut_assert_equal(Term(1, Zp<7>(2)), chain()[3]);
  }
}

void test_mulassign_Ring()
{
  Chain chain = setup_chain1();
  chain *= Zp<7>(3);

  cppcut_assert_equal(Term(2, Zp<7>(3)), chain()[0]);
  cppcut_assert_equal(Term(1, Zp<7>(6)), chain()[1]);
}

void test_output_stream()
{
  std::stringstream out;
  out << setup_chain1();
  cppcut_assert_equal(std::string("1[2] + 2[1]"), out.str());
}

void test_mul_Ring()
{
  {
    Chain chain = setup_chain1();
    chain = chain * Zp<7>(3);
    
    cppcut_assert_equal(Term(2, Zp<7>(3)), chain()[0]);
    cppcut_assert_equal(Term(1, Zp<7>(6)), chain()[1]);
  }
  {
    Chain chain = setup_chain1();
    chain = Zp<7>(3) * chain;
    
    cppcut_assert_equal(Term(2, Zp<7>(3)), chain()[0]);
    cppcut_assert_equal(Term(1, Zp<7>(6)), chain()[1]);
  }
}

class CompareTermByIndex {
 public:
  bool operator()(const Term &x, const Term &y) {
    return x.index() < y.index();
  }
};
  
void test_simplify()
{
  Chain chain = setup_chain1() + setup_chain2();
  chain = simplify(chain);
  
  std::sort(chain().begin(), chain().end(), CompareTermByIndex());
  cppcut_assert_equal((size_t)3, chain().size());
  cppcut_assert_equal(Term(1, Zp<7>(2)), chain()[0]);
  cppcut_assert_equal(Term(2, Zp<7>(4)), chain()[1]);
  cppcut_assert_equal(Term(3, Zp<7>(1)), chain()[2]);
}

}
