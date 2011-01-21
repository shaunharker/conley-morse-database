/*
 *  Distributed.hpp
 */

#include <set>
#include <deque>
#include <boost/foreach.hpp>

namespace coordinator_worker_scheme {

enum {
  kReadyTag = 1000, /* Tag of ready message, "ready" means that the worker is free and
                     ready to compute a new job */
  kRetireTag = 1001, /* Tag of retire message, "retire" means that coordinator has
                        no more jobs, so worker should stop */
  kResultTag = 1002, /* Tag of "result" message, means that worker is sending a result
                       message. */
  kJobTag = 1003 /* Tag of "result" message, means that worker is sending a result
                     message. */
};

/** Wait for a worker to send a message and receive it.
 *  If the message is a result message, call Coordinator.Process.
 */
template<class Communicator, class Coordinator>
void WaitForWorker(Communicator *comm, Coordinator *coordinator,
                   std::deque<typename Communicator::Channel> *free_workers) {
  typedef typename Communicator::Channel Channel;
  Channel channel;
  Message message;
  
  channel = comm->Probe();
  comm->Receive(&message, channel);
  if (message.tag == kReadyTag) {
    free_workers->push_back(channel);
  } else {
    coordinator->Process(message);
  }
}

/** Wait for workers to be free.
 */
template<class Communicator, class Coordinator>
void WaitForFreeWorker(Communicator *comm, Coordinator *coordinator,
                        std::deque<typename Communicator::Channel> *free_workers) {
  for (;free_workers->empty();) {
    WaitForWorker(comm, coordinator, free_workers);
  }
}

template<class Communicator>
void AssignJobToFreeWorker(Communicator *comm,
                           const Message &job,
                           std::deque<typename Communicator::Channel> *free_workers) {
  typedef typename Communicator::Channel Channel;
  Channel f = free_workers->front();
  free_workers->pop_front();
  //std::cout << "Sending message with (job ) tag = " << job . tag << "\n";
  comm->Send(job, f);
}

/** Send retire message to all free workers.
 */
template<class Communicator>
void BroadcastRetire(
    Communicator *comm,
    const std::deque<typename Communicator::Channel> &free_workers) {
  typedef typename Communicator::Channel Channel;
  Message retire_message;
  retire_message.tag = kRetireTag;
  comm->Broadcast(retire_message);
}
                           
/** Run communicator loop. 
 */
template<class Communicator, class Coordinator>
int RunCoordinator(Communicator *comm, Coordinator *coordinator) {
  typedef typename Communicator::Channel Channel;

  
  std::deque<Channel> free_workers;
  typename CoordinatorBase::State state;

  for (;;) {
    Message job;
    state = coordinator->Prepare(&job);
    switch (state) {
      case CoordinatorBase::kOK:
        job . tag = kJobTag;
        WaitForFreeWorker(comm, coordinator, &free_workers);
        AssignJobToFreeWorker(comm, job, &free_workers);
        break;
      case CoordinatorBase::kPending:
        WaitForWorker(comm, coordinator, &free_workers);
        break;
      case CoordinatorBase::kFinish:
        BroadcastRetire(comm, free_workers);
        return 0;
        break;
    }
  }
}
 
/** Run worker loop.
 */
template<class Communicator, class Worker>
int RunWorker(Communicator *comm, Worker *worker) {
  Message ready_message;
  ready_message.tag = kReadyTag;
  for (;;) {
    Message job, result;
    //std::cout << "Sending message with (ready ) tag = " << ready_message . tag << "\n";
    comm->Send(ready_message, comm->CoordinatorChannel());
    comm->Receive(&job, comm->CoordinatorChannel());
    if (job.tag == kRetireTag)
      return 0;
    worker->Work(&result, job);
    result . tag = kResultTag;
    //std::cout << "Sending message with (result ) tag = " << result . tag << "\n";
    comm->Send(result, comm->CoordinatorChannel());
  }
}

template<class Communicator, class Coordinator, class Worker>
int Run(int argc, char **argv) {
  Communicator comm(&argc, &argv);

  if (comm.Coordinating()) {
    Coordinator coordinator(argc, argv);
    return RunCoordinator(&comm, &coordinator);
  } else {
    Worker worker(argc, argv);
    return RunWorker(&comm, &worker);
  }
}

template<class Coordinator, class Worker>
int RunSingleProcess(int argc, char **argv) {
  Coordinator coordinator(argc, argv);
  Worker worker(argc, argv);
  typename CoordinatorBase::State state;
  
  for (;;) {
    Message job, result;
    state = coordinator.Prepare(&job);
    switch (state) {
      case CoordinatorBase::kOK:
        worker.Work(&result, job);
        coordinator.Process(result);
        break;
      case CoordinatorBase::kPending:
        /* maybe bug */
        assert(false);
        break;
      case CoordinatorBase::kFinish:
        return 0;
        break;
    }
  }
    
  return 0;
}

} /* namespace coordinator_worker_scheme */

