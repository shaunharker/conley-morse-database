/*
 *  Clutching_Graph_Job.h
 */

#ifndef _CMDP_CLUTCHING_GRAPH_JOB_
#define _CMDP_CLUTCHING_GRAPH_JOB_

#include "distributed/Message.h"

template <class Toplex, class ParameterToplex, class ConleyIndex>
void Clutching_Graph_Job ( Message * result , const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Clutching_Graph_Job.hpp"
#endif

#endif
