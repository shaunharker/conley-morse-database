/*
 *  Worker.cpp
 */

#include "program/Worker.h"
#include "program/jobs/Clutching_Graph_Job2.h"
#include "toplexes/Adaptive_Cubical_Toplex.h" /* For Toplex */
#include "algorithms/Homology.h" /* for Conley_Index_t */

using namespace Adaptive_Cubical;

// #include <unistd.h>

Worker::Worker(int argc, char **argv) {
}

void Worker::Work(Message * result, const Message &job) {
  Clutching_Graph_Job < Toplex, Toplex, Conley_Index_t > ( result , job ); 
}
