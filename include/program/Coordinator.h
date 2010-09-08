/*
 *  Coordinator.h
 */

#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "complexes/Adaptive_Complex.h"
#include "algorithms/basic.h"
#include "distributed/Distributed.h"
#include "data_structures/Cached_Box_Information.h"

using namespace Adaptive_Cubical;

typedef Toplex Parameter_Toplex;
typedef std::map <Toplex::Top_Cell, Cached_Box_Information> Cached_Box_Map;
typedef std::pair <Toplex::Top_Cell, Cached_Box_Information> Cached_Box_Pair;
typedef unsigned int size_type;

/// Coordinator class
class Coordinator : public CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  virtual CoordinatorBase::State Prepare(Message *job);
  virtual void Process(const Message &result);
 private:
  typedef unsigned int size_type;
  size_type num_jobs_;
  size_type num_jobs_sent_;
  size_type num_jobs_received_;
  std::vector < Toplex_Subset > patches_;
  Toplex parameter_toplex;
  std::map < Toplex::Top_Cell, Cached_Box_Information > toplex_cached_info;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
