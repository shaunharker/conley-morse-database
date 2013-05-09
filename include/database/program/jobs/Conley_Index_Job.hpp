/*
 *  Conley_Index_Job.hpp
 */

#include "chomp/ConleyIndex.h"

#include "database/program/Configuration.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/MorseGraph.h"
#include "database/structures/Database.h"
#include "database/structures/Grid.h"
#include "database/structures/PointerGrid.h"

#include "chomp/Rect.h"

template <class PhaseGrid >
void Conley_Index_Job ( Message * result , const Message & job ) {
  using namespace chomp;
  // Read job
  size_t job_number;
  chomp::Rect geo;
  std::pair < int, int > ms;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Rect PHASE_BOUNDS;
  std::vector < bool > PERIODIC;
  job >> job_number;
  job >> geo;
  job >> ms;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> PHASE_BOUNDS;
  job >> PERIODIC;
  
  std::cout << "CIJ: job_number = " << job_number << "  (" << ms . first << ", " << ms . second << ")\n";

  // Compute Morse Graph
  MorseGraph mg;
  
  boost::shared_ptr<Grid> phase_space ( new PhaseGrid );
  phase_space -> initialize ( PHASE_BOUNDS, PERIODIC );
  
     std::cout << "CIJ: geo = " << geo << "\n";
  
  GeometricMap map ( geo );

  std::cout << "CIJ: calling Compute_Morse_Graph\n";
  
  Compute_Morse_Graph ( &mg,
                        phase_space,
                        map,
                        PHASE_SUBDIV_MIN,
                        PHASE_SUBDIV_MAX,
                        PHASE_SUBDIV_LIMIT );
  
  // Select Subset
  typedef std::vector < Grid::GridElement > Subset;
  Subset subset = phase_space -> subset ( * mg . grid ( ms . second ) );

  std::cout << "CIJ: calling Conley_Index on Morse Set " << ms . second << "\n";
  
  // Compute Conley Index Record of Morse Set
  ConleyRecord record;
  record . id_ = ms;
  ConleyIndex ( & record . ci_, // ConleyIndex_t
                *phase_space,
                subset,
                map );

  std::cout << "CIJ: producing Database Record\n";
  // Produce Database to hold Conley Index Record
  
  Database database;
  database . insert ( record );
  
  // Return Result
  * result << job_number;
  * result << database;
  
}
