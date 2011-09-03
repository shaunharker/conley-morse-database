/*
 *  Conley_Morse_Database.cpp
 */

#include "delegator/delegator.h"  /* For Coordinator_Worker_Scheme<>() */
#include "program/MorseProcess.h"
#include "program/ConleyProcess.h"
#include "program/Configuration.h"

int main ( int argc, char * argv [] ) {
  delegator::Start ();
	delegator::Run < MorseProcess > (argc, argv); 
#ifndef NO_CONLEY_INDEX_IN_DATABASE
	delegator::Run < ConleyProcess > (argc, argv);
#endif	 
	delegator::Stop ();
	return 0;
}
