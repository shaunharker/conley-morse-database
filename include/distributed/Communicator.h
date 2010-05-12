/*
 *  Communicator.h
 */


#ifndef _CMDP_COMMUNICATOR_
#define _CMDP_COMMUNICATOR_

#include "distributed/Message.h"

class SimpleMPICommunicator {
 public:
  /** Type of communication route identifier.
   *  In MPI, this is the type of ranks.
   */
  typedef int Channel;
  /** Maximum byte size of message string.
   *  Current maximum is 64kbyte.
   */
  static const int kBufferSize = 64*1024;
  /** Initialize MPI and open all communication channels */
  SimpleMPICommunicator(int *argc, char ***argv);
  /** Close all communication channels and finalize MPI */
  ~SimpleMPICommunicator(void);
  /** Return true if the process is coordinator process */
  bool Coordinating();
  /** Return a communication channel to coordinator */
  Channel CoordinatorChannel(void);
  /** Send a "message" to "destination" channel */
  void Send(const Message &message, Channel destination);
  /** Receive a "message" from "source" channel.
   *  If there are no available message, wait until a message comes.
   */
  void Receive(Message *message, Channel source);
  /** Wait until a message comes in and
   *  return one of available channel.
   *  If some messages already arrive, immediately return
   *  this function.
   */
  Channel Probe(void);
  /** Send a "message" to all workers.
   */
  void Broadcast(const Message &message);
 private:
  char buffer_[kBufferSize];
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Communicator.hpp"
#endif

#endif
/* 
 * Local Variables:
 * mode: C++
 * End:
 */

