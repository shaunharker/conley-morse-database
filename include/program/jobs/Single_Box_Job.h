/*
 *  Single_Box_Job.h
 */

#ifndef _CMDP_SINGLE_BOX_JOB_
#define _CMDP_SINGLE_BOX_JOB_

#include "distributed/Messaging.h"

template < class Toplex_Template >
const Message Single_Box_Job ( const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Single_Box_Job.hpp"
#endif

#endif
