/*
 *  Worker.hpp
 */

#include <unistd.h> /* for usleep */

/* Constructor. Passes Communicator object to base class. */
template < class Communicator >
Worker<Communicator>::Worker ( Communicator & my_communicator ) : Worker_Base<Communicator> ( my_communicator ) {}

/* * * * * * * * * */
/* User Code Below */
/* * * * * * * * * */

/* Edit the function below to implement different worker behaviors  */
/* for computing jobs.                                              */

/* This is an example. */
template < class Communicator >
void Worker<Communicator>::work ( Message * output_message, const Message & job ) {
	Message & results = *output_message; // improve the semantics
	int data;
	job >> data;
	usleep(100000); // sleep for .1 seconds.
	results << data; 
	return;
} /* Worker::work */
