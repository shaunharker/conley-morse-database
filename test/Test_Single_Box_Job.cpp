
#include "program/jobs/Compute_Conley_Morse_Graph.h"
#include "data_structures/Cached_Box_Information.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "algorithms/Homology.h"

int main ( int argc, char * argv [] ) 
{
  Decide_Subdiv_Level_Size< Adaptive_Cubical::Toplex > decide_subdiv ( 5 , 100000 );
  Decide_Conley_Index_Size< Adaptive_Cubical::Toplex > decide_conley_index (
    2 , 10000 , true , Decide_Conley_Index_Size< Adaptive_Cubical::Toplex >::MaxValue );

  ConleyMorseGraph < Adaptive_Cubical::Toplex::Subset, Conley_Index_t > conley_morse_graph;
  Adaptive_Cubical::Geometric_Description parameter_box ( 2 , 20.01 , 20.02 );
  Adaptive_Cubical::Geometric_Description phase_space_box ( 2 , 0 , 300 );
  Adaptive_Cubical::Toplex phase_space;
  phase_space . initialize ( phase_space_box );
  Cached_Box_Information cached_box_information;

  Compute_Conley_Morse_Graph ( & conley_morse_graph , & phase_space ,
    parameter_box , decide_subdiv , decide_conley_index ,
    & cached_box_information );

  return 0;
} /* main */
