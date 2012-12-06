/*
 *  Conley_Morse_Database.cpp
 */

#include <ctime>
#include <fstream>
#define CMG_VERBOSE
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
  time_t morsestarttime = time ( NULL );
  delegator::Run < MorseProcess > (argc, argv);
  time_t morsetime = time ( NULL ) - morsestarttime;
  std::ofstream morsetimeoutfile ( "MorseProcessTime.txt");
  morsetimeoutfile << morsetime << " seconds.\n";
  morsetimeoutfile . close ();
#endif
#ifdef COMPUTE_CONLEY_INDEX
  time_t conleystarttime = time ( NULL );
  delegator::Run < ConleyProcess > (argc, argv);
  time_t conleytime = time ( NULL ) - conleystarttime;
  std::ofstream conleytimeoutfile ( "ConleyProcessTime.txt");
  conleytimeoutfile << conleytime << " seconds.\n";
  conleytimeoutfile . close ();
#endif
  delegator::Stop ();


  return 0;
}
