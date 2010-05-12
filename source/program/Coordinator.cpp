/*
 *  Coordinator.cpp
 */

#include "program/Coordinator.h"

Coordinator::Coordinator(int argc, char **argv) {
  n_ = 0;
  done_ = 0;
}

Coordinator::State Coordinator::Prepare(Message *job) {
  if (done_ >= 10)
    return kFinish;
  if (n_ >= 10)
    return kPending;
  job->tag = 1;
  *job << n_;
  n_++;
  return kOK;
}

void Coordinator::Process(const Message &result) {
  int r;
  result >> r;
  std::cout << "result: " << r << std::endl;
  done_++;
}
