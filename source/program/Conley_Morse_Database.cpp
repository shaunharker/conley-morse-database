/*
 *  Conley_Morse_Database.cpp
 */

#include "delegator/delegator.h"  /* For Coordinator_Worker_Scheme<>() */
#include "program/MorseProcess.h"
#include "program/ConleyProcess.h"
#include "program/Configuration.h"

//#define CMD_MORSE
#define CMD_CONLEY
int main ( int argc, char * argv [] ) {
  delegator::Start ();
#ifdef CMD_MORSE
	delegator::Run < MorseProcess > (argc, argv); 
#endif
#ifdef CMD_CONLEY
	delegator::Run < ConleyProcess > (argc, argv);
#endif	 
	delegator::Stop ();
	return 0;
}
