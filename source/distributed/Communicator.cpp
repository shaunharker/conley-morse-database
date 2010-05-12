/*
 *  Communicator
 *
 */

#include "mpi.h"
#include "distributed/Communicator.h"
#include <string>


typedef SimpleMPICommunicator::Channel Channel;

SimpleMPICommunicator::SimpleMPICommunicator(int *argc, char ***argv) {
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
  if (Coordinating()) {
    throw "Critical Program Error: Coordinator cannot access coordinator channel";
  } else {
    return 0;
  }
}

void SimpleMPICommunicator::Send(const Message &message,
                                 Channel destination) {
  int size = message.str().size();
  MPI_Send(const_cast<char*>(message.str().c_str()), size, MPI_BYTE,
           destination, message.tag, MPI_COMM_WORLD);
}

void SimpleMPICommunicator::Receive(Message *message,
                                    Channel source) {
  MPI_Status status;
  MPI_Recv(buffer_, sizeof(buffer_), MPI_BYTE, source,
           MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  int tag = status.MPI_TAG;
  int count;
  MPI_Get_count(&status, MPI_BYTE, &count);
  message->tag = tag;
  message->str(std::string(buffer_, count));
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
