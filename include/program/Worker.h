/*
 *  Worker.h
 */

#ifndef _CMDP_WORKER_
#define _CMDP_WORKER_

#include "distributed/Distributed.h"

class Worker: WorkerBase {
 public:
  Worker(int argc, char **argv);
  virtual void Work(Message * result, const Message &job);
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Worker.hpp"
#endif

#endif
