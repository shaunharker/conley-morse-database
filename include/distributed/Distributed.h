/*
 *  Distributed.h
 */

#ifndef _CMDP_DISTRIBUTED_
#define _CMDP_DISTRIBUTED_

#include "distributed/Message.h"
#include "distributed/Communicator.h"
 
/** Coordinator_Base */
template < class Communicator >
class Coordinator_Base {

private:
    Communicator & my_communicator;
	static const int JOB = 0;
	static const int RETIRE = 1;
	static const int READY = 0;
	static const int RESULTS = 1;

public:
    /** Constructors */
    Coordinator_Base ( Communicator & my_communicator );
    virtual ~Coordinator_Base ( void );

    /** run(). */
    int run ( void ); 
 
    /* Pure Virtual Functions to be supplied by derived class */
 
    /** Prepare a job to send. 
      * Return value is 0 if job is prepared.
      * Return value is 1 if a retirement job is prepared (no work left.)
      * Return value is 2 if no job prepared because none available until 
      *              more results are processed.
      */
    virtual int prepare ( Message * job ) = 0;
 
    /** Process the results of a job. */
    virtual void process ( const Message & results ) = 0;
};
 
/** Worker_Base */
template < class Communicator >
class Worker_Base {
private:
    Communicator & my_communicator;
    typename Communicator::Entity coordinator;
	static const int JOB = 0;
	static const int RETIRE = 1;
	static const int READY = 0;
	static const int RESULTS = 1;

public:
    /** Constructor */
 	Worker_Base ( Communicator & my_communicator );
	virtual ~Worker_Base ( void );
	
    /** run(). */
    int run ( void ); 
 
    /* Pure Virtual Functions to be supplied by derived class */
 
    /** Work the job. */
    virtual void work ( Message * results, const Message & job ) = 0;
};


/** Coordinator_Worker_Scheme */
template < template < class > class Coordinator, template < class > class Worker, class Communicator >
int Coordinator_Worker_Scheme ( int argc, char * argv [] );

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Distributed.hpp"
#endif

#endif
