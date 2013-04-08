/*
 *  Compute_Conley_Morse_Graph.h
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_

#include <boost/shared_ptr.hpp>

#include "database/structures/Conley_Morse_Graph.h"
#include "database/structures/Grid.h"

/// Computes the Morse decomposition with respect to the given map
/// on the given phase space, and creates its representation by means
/// of a Conley-Morse graph.

template < class Map >
void Compute_Morse_Graph (MorseGraph * MG,
                          boost::shared_ptr<Grid> phase_space,
                          const Map & interval_map,
                          const unsigned int Min,
                          const unsigned int Max,
                          const unsigned int Limit);  

#include "database/program/jobs/Compute_Morse_Graph.hpp"

#endif
