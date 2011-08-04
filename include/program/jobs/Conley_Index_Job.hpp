/*
 *  Conley_Index_Job.hpp
 */

#include "data_structures/Conley_Morse_Graph.h"
#include "data_structures/Database.h"

template <class Toplex, class ParameterToplex, class ConleyIndex>
void Conley_Index_Job ( Message * result , const Message & job ) {
  // Read job
  int job_number;
  typename Toplex::Geometric_Description geo;
  std::pair < int, int > ms;
  job >> job_number;
  job >> geo;
  job >> ms;
  
  // Compute Morse Graph
  typedef std::vector< typename Toplex::Top_Cell > Subset;
  typedef ConleyMorseGraph< std::vector < typename Toplex::Top_Cell >, ConleyIndex> CMGraph;
  CMGraph cmg;
  
  Toplex phase_space;
  phase_space . initialize ( space_bounds );
  
  GeometricMap map ( geo );

  Compute_Morse_Graph ( & cmg, 
                        & phase_space, 
                          map,
                          MIN_PHASE_SUBDIVISIONS, 
                          MAX_PHASE_SUBDIVISIONS, 
                          COMPLEXITY_LIMIT );
  
  // Select Subset
  Subset subset = cmg . CellSet ( ms . second );
  
  // Compute Conley Index Record of Morse Set
  ConleyRecord record;
  record . id_ = ms;
  Conley_Index ( & record . ci_, // Conley_Index_t
                phase_space, 
                subset,
                map );
  
  // Produce Database to hold Conley Index Record
  Database database;
  database . insert ( record );
  
  // Return Result
  * result << job_number;
  * result << database;
  
}