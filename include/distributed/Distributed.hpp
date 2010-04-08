/*
 *  Distributed.hpp
 */

#include <set>
#include <deque>

/* Coordinator_Base */

template < class Communicator >
Coordinator_Base<Communicator>::Coordinator_Base( Communicator & my_communicator ) : my_communicator ( my_communicator ) { }

template < class Communicator >
Coordinator_Base<Communicator>::~Coordinator_Base( void )  { }


#define JOB_LIMIT  64
#define RESULTS_LIMIT 8

/* Coordinator_Base<Communicator>::run()
 * Essential idea: try to minimize time workers are not doing jobs.
 * Strategy: keep a queue of workers, a queue of jobs, and a queue of results to process
 * Fill the job queue before moving onto processing jobs.
 * If a message is received at any point, it is either RESULTS or READY.
 * If RESULTS, add to queue. If READY, give a job immediately if possible, or else add to worker queue.
 *
 * Notes: At any given time, either the job queue or the worker queue is empty.
 *        The strategy of filling the entire job queue before processing results may be questionable.
 */
template < class Communicator >
int Coordinator_Base<Communicator>::run ( void ) {
    typedef typename Communicator::Entity Entity;
    
    int number_of_workers = 0;
    bool no_more_jobs = false;
    
	/* Initialize data structures. */
	std::deque<Entity> worker_queue;
	std::deque<Message> job_queue;
	std::deque<Message> results_queue;
	std::set<Entity> unretired_workers;
    
	do {
	
		std::cout << "Coordinator: Job queuing loop.\n";
        /* JOB QUEUING LOOP  
		  Make jobs and give them to available workers until either
           a) The limit of jobs we are willing to store is met
           b) A message is available 
           c) We must await results before computing the next job */
		while ( not my_communicator . probe ( my_communicator . ANYTAG ) && job_queue . size () < JOB_LIMIT ) {
            Message job;
            /* Prepare a job. (if we cannot, break )*/
            int prepare_status = prepare ( &job );
			
			/* Check the status of prepare and act accordingly. */
			
			/* Has prepare created a job for us? */
			if ( prepare_status == 0 )  {
				job . tag = JOB;
			} /* if */
			
			/* Does prepare say it's time to retire? */
			if ( prepare_status == 1 )  {
				job . tag = RETIRE; 
				/* If there are no workers in line to retire, exit the job-queueing loop.*/
				if ( worker_queue . empty () ) {
					no_more_jobs = true; 
					break;
				} /* if */
			} /* if */
			
			/* Does prepare say jobs cannot be created for now? If so, exit the job-queueing loop. */
			if ( prepare_status == 2 ) break;
            
			/* Check if there is a worker to give the job to. */
			if ( not worker_queue . empty () ) {
                /* Get a worker from the front of the queue. */
				Entity & worker =  worker_queue . front ();
                /* Give the job to the worker. */
                my_communicator . send ( job, worker );
                /* The worker is no longer in line. */
                worker_queue . pop_front ();
                /* If worker is being retired, take note. */
                if ( job . tag == RETIRE ) {
                    unretired_workers . erase ( worker );
                    --number_of_workers;
                    no_more_jobs = true;
                } /* if */
			} else {
                /* There aren't any workers to take the job. Put it the job queue. */
				job_queue . push_back ( job ); 
			} /* if-else */
		} /* while */
        
		std::cout << "Coordinator: Processing loop.\n";
		/* Process results until finished or until a message comes */
		while ( not my_communicator . probe ( my_communicator . ANYTAG ) && results_queue . size () > 0 ) {
            /* Get a message from the queue. */
            Message & results = results_queue . front ();
            /* Process the message */
            process ( results );
            /* The results message is no longer in the queue. */
            results_queue . pop_front ();
		} /* while */
		
		/* Determine whether or not to break from loop. Break if the work is done and nothing is left to process. */
		if ( number_of_workers == 0 && no_more_jobs && results_queue . empty () &&  
		     not my_communicator . probe ( my_communicator . ANYTAG ) ) break;
		
		std::cout << "Coordinator: Receiving.\n";
		/* Receive a message */
        Message incoming;
        Entity worker;
		my_communicator . receive ( &incoming, &worker, my_communicator . ANYTAG, my_communicator . ANYSOURCE );
		
		std::cout << "Coordinator: Deal with READY message if applicable.\n";

		/* Deal with a READY message */
		if ( incoming . tag == READY ) {
			++ number_of_workers;
            unretired_workers . insert ( worker );
			if ( not job_queue . empty () ) {
                /* Get a job from the front of the queue. */
				Message & job =  job_queue . front ();
                /* Give the job to the worker. */
                my_communicator . send ( job, worker );
                /* If worker is being retired, take note. */
                if ( job . tag == RETIRE ) {
                    unretired_workers . erase ( worker );
                    --number_of_workers;
                    no_more_jobs = true;
                } /* if */
                /* The job is no longer in line. */
                job_queue . pop_front ();
			} else {
                /* Put the worker in line */
                worker_queue . push_back ( worker );
			} /* if-else */
		} /* if */
		
		std::cout << "Coordinator: Deal with RESULTS message if applicable.\n";
		/* Deal with a RESULTS message */
		if ( incoming . tag == RESULTS ) {
			-- number_of_workers;
			if ( results_queue . size () < RESULTS_LIMIT ) {
                /* Put the results in queue to be processed later */
				std::cout << "  Putting results in queue.\n";
                results_queue . push_back ( incoming );
            } else { /* Oh no! We are swamped. */
                /* In swamping condition we must process immediately */
				std::cout << "  Swamped! Calling process.\n";
                process ( incoming );    
            } /* if-else */
        } /* if */
       
		std::cout << "Coordinator: Bottom of loop.\n";

    } while ( number_of_workers > 0 || not no_more_jobs || not results_queue . empty () ||  
			  my_communicator . probe ( my_communicator . ANYTAG ) );

	std::cout << "Coordinator: Main Loop exited \n";
	
    /* All results have been processed. Tell all unretired workers to retire. */
    /* Create a retire job */
    Message retire_job;
    retire_job . tag = RETIRE;
    /* Send retire job out to all unretired workers. */
    for ( typename std::set<Entity>::const_iterator worker = unretired_workers . begin ();
    worker != unretired_workers . end (); ++ worker ) 
		my_communicator . send ( retire_job, *worker );
    
	std::cout << "Coordinator: Finished, returning. \n";

    /* Finish normally, return 0 */
    return 0;
}

template < class Communicator >
Worker_Base<Communicator>::Worker_Base ( Communicator & my_communicator ) : my_communicator ( my_communicator ) {
    coordinator = my_communicator . DIRECTOR; 
}


/* Worker_Base */

template < class Communicator >
Worker_Base<Communicator>::~Worker_Base( void )  { }

template < class Communicator >
int Worker_Base<Communicator>::run ( void ) {
    /* Main loop */
    while ( true ) {
        /* Send a READY message. */
        Message ready;
        ready . tag = READY;
        my_communicator . send ( ready, coordinator );
        /* Receive a job. */
        Message job;
		my_communicator . receive ( &job, &coordinator, my_communicator . ANYTAG, my_communicator . ANYSOURCE );
        /* Quit if it is a retire job. */
        if ( job . tag == RETIRE ) break;
        /* Work the job. */
        Message results;
        work ( &results, job );
		results . tag = RESULTS;
        /* Send the results back to the coordinator. */
        my_communicator . send ( results, coordinator );
    }
	return 0;
}


template < template < class > class Coordinator, template < class > class Worker, class Communicator >
int Coordinator_Worker_Scheme ( int argc, char * argv [] ) {
  int return_condition;
  /* Create communicator and initialize communications */
  Communicator my_communicator;
  my_communicator . initialize ( argc, argv );
  /* Determine identity. */
  if ( my_communicator . SELF == my_communicator . DIRECTOR ) {
    /* I am the coordinator. Create a Coordinator object and call its 'run' routine. */
    Coordinator<Communicator> my_coordinator ( my_communicator );
    return_condition = my_coordinator . run ();
  } else {
    /* I am a worker. Create a Worker object and call its 'run' routine. */
    Worker<Communicator> my_worker ( my_communicator );
    return_condition = my_worker . run ();
  }
  /* Finalize the communications. */
  my_communicator . finalize ();
  return return_condition; 
} /* Coordinator_Worker_Scheme<> */

