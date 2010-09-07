/*
 *  Single_Box_Job.h
 */

#ifndef _CMDP_SINGLE_BOX_JOB_
#define _CMDP_SINGLE_BOX_JOB_

#include "distributed/Messaging.h"


/// Constructs the Conley-Morse decomposition for a single parameter box.
/// Runs a complete single box job as one of the worker's tasks.
/// Reads the computation parameters from the job message.
/// Saves the results of the computation in the result message.
template < class Toplex, class Parameter_Toplex ,
  class Map , class Conley_Index , class Cached_Box_Information >
void Single_Box_Job ( Message * result, const Message & job );


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Single_Box_Job.hpp"
#endif

#endif
