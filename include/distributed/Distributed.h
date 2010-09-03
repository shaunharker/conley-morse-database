/*
 *  Distributed.h
 */

#ifndef _CMDP_DISTRIBUTED_
#define _CMDP_DISTRIBUTED_

#include "distributed/Message.h"
#include "distributed/Communicator.h"

/** Base class of coordinator. A coordinator prepares works for
 *  each (distrubuted computation) node and gather the computation
 *  results.
 *
 *  This class has only abstract member functions and users must be
 *  override these functions.
 *  Constructor prototype of derived class must be
 *  Coordinator(int argc, char **argv) because coordinator_worker_scheme::Run
 *  requires such a prototype.
 */
class CoordinatorBase {
 public:
  enum State {
    kOK, /* A new job is prepared */
    kFinish, /* All jobs are finished and stop computation */
    kPending, /* Wait for some jobs to be completed */
  };
  /** Users prepare a new job and set the job to "job".
   * 
   *  If you can prepare the new job, you must return kOK.
   *  If you want to wait some jobs which aren't finished yet,
   *  you must return kPending.
   *  If all jobs are completed, you must return kFinish.
   *  If all jobs are requested but not all jobs are finished,
   *  you must return kPending, don't return kFinish.
   */
  virtual State Prepare(Message *job) = 0;
  /** Users process a result of a job of "Prepare" function.
   */
  virtual void Process(const Message &result) = 0;
};

/** Base class of worker. A worker receive works from
 *  a coordinator, compute the result, and send
 *  the result to the coordinator.
 *
 *  This class has only abstract member functions and users must be
 *  override these functions.
 *  Constructor prototype of derived class must be
 *  Worker(int argc, char **argv) because coordinator_worker_scheme::Run
 *  requires such a prototype.
 */
class WorkerBase {
 public:
  /** Users execute a computation work prepared by "Prepare" function
   * in coordinator class, and set the result to "result".
   */
  virtual void Work(Message * result, const Message &job) = 0;
};

namespace coordinator_worker_scheme {
/** Initialize communication channels, start a coordinator or a worker,
 *  compute, gather results, and finalize these channels.
 */
template<class Communicator, class Coordinator, class Worker>
int Run(int argc, char **argv);

/** Computation in single process
 */
template<class Coordinator, class Worker>
int RunSingleProcess(int argc, char **argv);
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Distributed.hpp"
#endif

#endif

/* 
 * Local Variables:
 * mode: C++
 * End:
 */
