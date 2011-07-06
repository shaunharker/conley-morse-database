/*
 *  Coordinator.h
 */

#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include "distributed/Distributed.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "data_structures/Database.h"

using namespace Adaptive_Cubical;

/// Coordinator class
class Coordinator : public CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  virtual CoordinatorBase::State Prepare(Message *job);
  virtual void Process(const Message &result);
  void finalize ( void );
 private:
  Toplex param_toplex;
  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t num_jobs_received_;
  std::vector < Toplex_Subset > PS_patches;
  Database database;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
