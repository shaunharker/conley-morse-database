/*
 *  Coordinator.h
 */

#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_
#include <vector>
#include "distributed/Distributed.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "data_structures/Database.h"

using namespace Adaptive_Cubical;

/// Coordinator class
class Coordinator : public CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  
  void StartMorseStage ( void );
  void StartConleyStage ( void );

  virtual CoordinatorBase::State Prepare(Message *job);
  virtual CoordinatorBase::State MorsePrepare(Message *job);
  virtual CoordinatorBase::State ConleyPrepare(Message *job);

  virtual void Process(const Message &result);
  virtual void MorseProcess(const Message &result);
  virtual void ConleyProcess(const Message &result);

  void finalize ( void );
 private:
  int stage; // stage 0 -- clutching graph, stage 1 -- conley index
  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t num_jobs_received_;
  Toplex param_toplex;
  Database database;
  std::vector < Toplex_Subset > PS_patches; // morse_work_items
  std::vector<std::pair<int, int> > conley_work_items;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
