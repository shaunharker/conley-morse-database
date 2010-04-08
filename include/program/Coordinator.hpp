/*
 *  Coordinator.hpp
 */

#include <iostream>

/* Constructor. Passes Communicator object to base class. */
template < class Communicator >
Coordinator<Communicator>::Coordinator ( Communicator & my_communicator ) : Coordinator_Base<Communicator> ( my_communicator ) {}

/* * * * * * * * * */
/* User Code Below */
/* * * * * * * * * */

/* Edit the functions below to implement different coordinator behaviors  */
/* for preparing and processing jobs.                                     */

/* This is an example. */
template < class Communicator >
int Coordinator<Communicator>::prepare ( Message * output ) {
    static int number = 0;
	Message & job = *output;
	
	/* Increment number. */
	++ number;
	/* Send out one thousand jobs, then send retire jobs. */
	if ( number > 1000 ) {
		return 1; /* Return 1 to indicate retirement phase */
	} else {
		job << number;
		return 0; /* Return 0 to indicate normal job creation. */
	}
	return 0;
} /* Coordinator::prepare */

/* This is an example. */
template < class Communicator >
void Coordinator<Communicator>::process ( const Message & results ) {
    int data;
	results >> data;
	std::cout << "Processing results: " << data << "\n";
	return;
} /* Coordinator::process */
