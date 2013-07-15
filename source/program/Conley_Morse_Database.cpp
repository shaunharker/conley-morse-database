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
#ifdef COMPUTE_CONTINUATION
#include "ModelMap.h"
#include "database/structures/Database.h"
#endif
#ifdef COMPUTE_CONLEY_INDEX
#include "database/program/ConleyProcess.h"
#endif

#include <boost/serialization/export.hpp>
#ifdef HAVE_SUCCINCT
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);


int main ( int argc, char * argv [] ) {
  delegator::Start ();
#ifdef COMPUTE_MORSE_SETS
  time_t morsestarttime = time ( NULL );
  delegator::Run < MorseProcess > (argc, argv);
  time_t morsetime = time ( NULL ) - morsestarttime;
  {
  std::string filestring ( argv[1] );
  std::string appendstring ( "/MorseProcessTime.txt" );
  std::ofstream morsetimeoutfile ( (filestring + appendstring) . c_str () );
  morsetimeoutfile << morsetime << " seconds.\n";
  morsetimeoutfile . close ();
  }
#endif

#ifdef COMPUTE_CONTINUATION
  int comm_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  if ( comm_rank == 0 ) {
    Database database;
    {
    std::string filestring ( argv[1] );
    std::string appendstring ( "/database.raw" );
    database . load ( (filestring + appendstring) . c_str () );
    }
    database . removeBadBoxes<ModelMap> ();
    database . postprocess ();
    {
    std::string filestring ( argv[1] );
    std::string appendstring ( "/database.mdb" );
    database . save ( (filestring + appendstring) . c_str () );
    }
  }
#endif

#ifdef COMPUTE_CONLEY_INDEX
  time_t conleystarttime = time ( NULL );
  delegator::Run < ConleyProcess > (argc, argv);
  time_t conleytime = time ( NULL ) - conleystarttime;
  {
  std::string filestring ( argv[1] );
  std::string appendstring ( "/ConleyProcessTime.txt" );
  std::ofstream conleytimeoutfile ( (filestring + appendstring) . c_str () );
  conleytimeoutfile << conleytime << " seconds.\n";
  conleytimeoutfile . close ();
  }
#endif
  delegator::Stop ();


  return 0;
}
