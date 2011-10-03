/*
 *  Clutching_Graph_Job.h
 */

#ifndef _CMDP_CLUTCHING_GRAPH_JOB_
#define _CMDP_CLUTCHING_GRAPH_JOB_

#include "delegator/Message.h"

template <class Toplex, class ParameterToplex, class ConleyIndex>
void Clutching_Graph_Job ( Message * result , const Message & job ); 

#include "database/program/jobs/Clutching_Graph_Job.hpp"

#endif
