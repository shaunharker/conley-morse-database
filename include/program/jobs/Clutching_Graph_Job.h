/*
 *  Clutching_Graph_Job.h
 */

#ifndef _CMDP_CLUTCHING_GRAPH_JOB_
#define _CMDP_CLUTCHING_GRAPH_JOB_

#include "distributed/Messaging.h"

template < class Toplex_Template >
void Clutching_Graph_Job ( Message * result , const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Clutching_Graph_Job.hpp"
#endif

#endif
