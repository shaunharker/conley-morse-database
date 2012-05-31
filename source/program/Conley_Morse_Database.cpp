/*
 *  Conley_Morse_Database.cpp
 */

#include "delegator/delegator.h"  /* For Coordinator_Worker_Scheme<>() */
#ifdef COMPUTE_MORSE_SETS
#include "database/program/MorseProcess.h"
#endif
#ifdef COMPUTE_CONLEY_INDEX
#include "database/program/ConleyProcess.h"
#endif

int main ( int argc, char * argv [] ) {
  delegator::Start ();
#ifdef COMPUTE_MORSE_SETS
  delegator::Run < MorseProcess > (argc, argv);
#endif
#ifdef COMPUTE_CONLEY_INDEX
  delegator::Run < ConleyProcess > (argc, argv);
#endif	 
  delegator::Stop ();
  return 0;
}
