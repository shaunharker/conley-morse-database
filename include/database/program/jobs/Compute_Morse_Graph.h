/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_


/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.

template < class Morse_Graph, class Toplex, class Map >
void Compute_Morse_Graph (Morse_Graph * CG, 
                          Toplex * phase_space, 
                          const Map & interval_map,
                          const unsigned int Min = 12, 
                          const unsigned int Max = 15,
                          const unsigned int Comp = 10000);
  

#include "database/program/jobs/Compute_Morse_Graph.hpp"

#endif
