/*
 *  Conley_Morse_Database.cpp
 */


#include "distributed/Distributed.h"  /* For Coordinator_Worker_Scheme<>() */
#include "distributed/Communicator.h" /* For Simple_MPI_Communicator */
#include "program/Worker.h"           /* For Worker<> */
#include "program/Coordinator.h"      /* For Coordinator<> */

int main ( int argc, char * argv [] ) {
	return Coordinator_Worker_Scheme < Coordinator, Worker, Simple_MPI_Communicator > ( argc, argv );
}
