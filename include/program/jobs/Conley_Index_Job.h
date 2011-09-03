/*
 *  Conley_Index_Job.h
 */

#ifndef _CMDP_CONLEY_INDEX_JOB_
#define _CMDP_CONLEY_INDEX_JOB_

#include "delegator/Message.h"

template <class Toplex, class ParameterToplex, class ConleyIndex>
void Conley_Index_Job ( Message * result , const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/jobs/Conley_Index_Job.hpp"
#endif

#endif
