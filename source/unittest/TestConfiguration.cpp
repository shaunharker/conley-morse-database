
#include <database/program/Configuration.h>

#include <cppcutter.h>

namespace TestConfiguration {
using namespace chomp;

void test_LoadFromStream() {
  typedef std::vector<double> v;
  
  Configuration config;
  std::ifstream stream("config.xml");
  config.LoadFromStream(&stream);
  
  cppcut_assert_equal(std::string("Leslie Model, Depth 12"),
                      config.MODEL_NAME);
  cppcut_assert_equal(std::string(" This is a description. "),
                      config.MODEL_DESC);
  cppcut_assert_equal(2, config.PARAM_DIM);
  cppcut_assert_equal(6, config.PARAM_SUBDIV_DEPTH);
  cppcut_assert_equal(Rect(2, v{8.0, 3.0}, v{37.0, 50.0}),
                      config.PARAM_BOUNDS);
  cppcut_assert_equal(2, config.PHASE_DIM);
  cppcut_assert_equal(12, config.PHASE_SUBDIV_MIN);
  cppcut_assert_equal(16, config.PHASE_SUBDIV_MAX);
  cppcut_assert_equal(10000, config.PHASE_SUBDIV_LIMIT);
  cppcut_assert_equal(Rect(2, v{0.0, 0.0}, v{320.0, 224.0}),
                      config.PHASE_BOUNDS);
}

}
