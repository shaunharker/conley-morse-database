/*
 *  Worker.cpp
 */

#include "program/Worker.h"
// #include <unistd.h>

/* Sample worker */
Worker::Worker(int argc, char **argv) {
}

/* sample worker, double the integer */
void Worker::Work(Message * result, const Message &job) {
  int data;
  // sleep(2);
  job >> data;
  result->tag = 1;
  *result << data*2;
}
