/*
 *  Conley_Morse_Database.cpp
 */

#include "delegator/delegator.h"  /* For Coordinator_Worker_Scheme<>() */
#include "program/MorseProcess.h"
#include "program/ConleyProcess.h"

int main ( int argc, char * argv [] ) {
  delegator::Start ();
	delegator::Run < MorseProcess > (argc, argv); 
	delegator::Run < ConleyProcess > (argc, argv); 
	delegator::Stop ();
	return 0;
}
