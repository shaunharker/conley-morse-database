/*
 *  Conley_Index_Job.h
 */

#ifndef _CMDP_CONLEY_INDEX_JOB_
#define _CMDP_CONLEY_INDEX_JOB_

#include "delegator/Message.h"

template <class Toplex, class ParameterToplex >
void Conley_Index_Job ( Message * result , const Message & job ); 

#include "program/jobs/Conley_Index_Job.hpp"

#endif
