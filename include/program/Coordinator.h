/*
 *  Coordinator.h
 */


#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include "distributed/Distributed.h"
 
/** Coordinator */
template < class Communicator >
class Coordinator : public Coordinator_Base < Communicator > {
 
public:
 
	/** Constructor */
	Coordinator ( Communicator & my_communicator );
	
    /** Prepare a job to send. */
    int prepare ( Message * job );
 
    /** Process the results of a job. */
    void process ( const Message & results );
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
