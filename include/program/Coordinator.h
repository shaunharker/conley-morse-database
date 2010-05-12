/*
 *  Coordinator.h
 */


#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include "distributed/Distributed.h"

/* Sample Coordinator class */
class Coordinator : CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  virtual State Prepare(Message *job);
  virtual void Process(const Message &result);
 private:
  int n_, done_;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
