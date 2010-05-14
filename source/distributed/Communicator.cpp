/*
 *  Communicator
 *
 */

#include "mpi.h"
#include "distributed/Communicator.h"
#include <string>
#include <cassert>


typedef SimpleMPICommunicator::Channel Channel;

SimpleMPICommunicator::SimpleMPICommunicator(int *argc, char ***argv)
    : buffer_(kBufferSize) {
  MPI_Init(argc, argv);
}


SimpleMPICommunicator::~SimpleMPICommunicator(void) {
  MPI_Finalize();
}

bool SimpleMPICommunicator::Coordinating(void) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return rank == 0;
}

Channel SimpleMPICommunicator::CoordinatorChannel(void) {
  assert(!Coordinating());
  return 0;
}

void SimpleMPICommunicator::Send(const Message &message,
                                 Channel destination) {
  int size = message.str().size();
  MPI_Send(const_cast<char*>(message.str().data()), size, MPI_BYTE,
           destination, message.tag, MPI_COMM_WORLD);
}

/* TODO: direcly pass stringbuf buffer to MPI_Recv.
 */
void SimpleMPICommunicator::Receive(Message *message,
                                    Channel source) {
  MPI_Status status;
  /* Get data size to determine buffer size before real communication */
  MPI_Probe(source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  int count;
  MPI_Get_count(&status, MPI_BYTE, &count);
  buffer_.resize(count);
  
  MPI_Recv(&buffer_[0], count, MPI_BYTE, source,
           MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  message->tag = status.MPI_TAG;
  message->str(std::string(&buffer_[0], count));
}

Channel SimpleMPICommunicator::Probe(void) {
  MPI_Status status;
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  return status.MPI_SOURCE;
}

void SimpleMPICommunicator::Broadcast(const Message &message) {
  int n;
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  for (int i=1; i<n; i++)
    Send(message, i);
}
