/*
 *  Clutching_Graph_Job.h
 */

#ifndef _CMDP_CLUTCHING_GRAPH_JOB_
#define _CMDP_CLUTCHING_GRAPH_JOB_

#include "distributed/Messaging.h"

template < class Toplex_Template >
const Message Clutching_Graph_Job ( const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Clutching_Graph_Job.hpp"
#endif

#endif