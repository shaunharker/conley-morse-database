/*
 *  Communicator
 *
 */

#include "mpi.h"
#include "distributed/Communicator.h"
#include <string>


bool MPI_Entity::operator < ( const MPI_Entity & right_hand_side ) const {
	return name < right_hand_side . name;
} /* Simple_MPI_Communicator::Entity::operator < */

bool MPI_Entity::operator == ( const MPI_Entity & right_hand_side ) const {
	return name == right_hand_side . name;
} /* Simple_MPI_Communicator::Entity::operator == */

Simple_MPI_Communicator::~Simple_MPI_Communicator ( void ) {}

void Simple_MPI_Communicator::initialize ( int argc, char * argv []) {
	/* Initialize the MPI communications */
	MPI_Init(&argc, &argv);  
	/* Determine identity. */
	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	/* Initialize member variables. */
	ANYTAG = MPI_ANY_TAG;
	ANYSOURCE . name = MPI_ANY_SOURCE;
	DIRECTOR . name = 0;
	SELF . name = my_rank;
	/* Initialize buffer. */
	buffer_length = 65536; //64K
	buffer = new char [ buffer_length ];
} /* Simple_MPI_Communicator::Simple_MPI_Communicator */

void Simple_MPI_Communicator::finalize ( void ) {
	/* Finalize the MPI communications. */
	MPI_Finalize();
	/* Finalize buffer */
	delete buffer;
} /* Simple_MPI_Communicator::~Simple_MPI_Communicator */

void Simple_MPI_Communicator::send ( const Message & send_me, const Entity & target ) {
	std::string message = send_me . str ();
	MPI_Send ( const_cast<char *> ( message  . data () ), message . length (), 
	  MPI_CHAR, target . name, send_me . tag, MPI_COMM_WORLD );
} /* Simple_MPI_Communicator::send */

void Simple_MPI_Communicator::receive ( Message * receive_me, Entity * sender, int tag, const Entity & source ) {
	/* Receive the message */
	MPI_Status status;
	MPI_Recv ( buffer, buffer_length, MPI_CHAR, source . name, tag, MPI_COMM_WORLD, &status );
	/* Get the length of the message */
	int count;
	MPI_Get_count ( &status, MPI_CHAR, &count );
	/* Produce a string holding the message */
	std::string message ( buffer, count );
	/* Copy the message into the Message structure */
	receive_me -> str ( message );
	receive_me -> tag = status . MPI_TAG;
	/* Identity the source of the message */
	sender -> name = status . MPI_SOURCE;
} /* Simple_MPI_Communicator::receive */

bool Simple_MPI_Communicator::probe ( int tag ) {
	int flag; 
	MPI_Status status;
	MPI_Iprobe ( MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &status );
	return flag ? true : false;
} /* Simple_MPI_Communicator::probe */
