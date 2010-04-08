/*
 *  Worker.h
 */

#ifndef _CMDP_WORKER_
#define _CMDP_WORKER_

#include "distributed/Distributed.h"

/** Worker_Base */
template < class Communicator >
class Worker : public Worker_Base < Communicator > {

int rank;

public:

	/** Constructor */
	Worker ( Communicator & my_communicator );
	 
    /** Work the job. */
    void work ( Message * results, const Message & job );

};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Worker.hpp"
#endif

#endif
