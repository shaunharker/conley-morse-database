/*
 *  Conley_Morse_Database.cpp
 */


#include "distributed/Distributed.h"  /* For Coordinator_Worker_Scheme<>() */
#include "distributed/Communicator.h" /* For Simple_MPI_Communicator */
#include "program/Worker.h"           /* For Worker<> */
#include "program/Coordinator.h"      /* For Coordinator<> */

int main ( int argc, char * argv [] ) {
  return coordinator_worker_scheme::Run<
  SimpleMPICommunicator, Coordinator, Worker>(argc, argv);
  //return coordinator_worker_scheme::RunSingleProcess<Coordinator, Worker>(argc, argv);
}
