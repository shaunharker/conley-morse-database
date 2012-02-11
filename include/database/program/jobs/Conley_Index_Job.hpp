/*
 *  Conley_Index_Job.hpp
 */

#include "chomp/ConleyIndex.h"

#include "database/program/Configuration.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/Conley_Morse_Graph.h"
#include "database/structures/Database.h"

template <class Toplex, class ParameterToplex >
void Conley_Index_Job ( Message * result , const Message & job ) {
  // Read job
  size_t job_number;
  typename Toplex::Geometric_Description geo;
  std::pair < int, int > ms;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Rect PHASE_BOUNDS;

  job >> job_number;
  job >> geo;
  job >> ms;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> PHASE_BOUNDS;
  
  //std::cout << "CIJ: job_number = " << job_number << "  (" << ms . first << ", " << ms . second << ")\n";

  // Compute Morse Graph
  typedef std::vector< typename Toplex::Top_Cell > Subset;
  typedef ConleyMorseGraph< std::vector < typename Toplex::Top_Cell >, ConleyIndex_t> CMGraph;
  CMGraph cmg;
  
  Toplex phase_space;
  phase_space . initialize ( PHASE_BOUNDS );
  
  //std::cout << "CIJ: geo = " << geo << "\n";
  
  GeometricMap map ( geo );

  //std::cout << "CIJ: calling Compute_Morse_Graph\n";
  
  Compute_Morse_Graph ( & cmg, 
                        & phase_space, 
                          map,
                          PHASE_SUBDIV_MIN, 
                          PHASE_SUBDIV_MAX, 
                          PHASE_SUBDIV_LIMIT );
  
  // Select Subset
  Subset subset = cmg . CellSet ( ms . second );

  //std::cout << "CIJ: calling Conley_Index\n";

  // Compute Conley Index Record of Morse Set
  ConleyRecord record;
  record . id_ = ms;
  ConleyIndex ( & record . ci_, // ConleyIndex_t
                phase_space, 
                subset,
                map );

  //std::cout << "CIJ: producing Database Record\n";
  // Produce Database to hold Conley Index Record
  Database database;
  database . insert ( record );
  
  // Return Result
  * result << job_number;
  * result << database;
  
}
