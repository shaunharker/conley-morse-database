/*
 *  Single_Box_Job.h
 */

#ifndef _CMDP_SINGLE_BOX_JOB_
#define _CMDP_SINGLE_BOX_JOB_

#include "distributed/Messaging.h"

/// Computes the Conley-Morse decomposition with respect to the given map
/// on the given phase space. The toplex representation of the phase space
/// is subdivided the given number of times.
/// The Conley-Morse graph is stored in the data structure provided
/// (which must be initially empty).
/// Morse sets are inserted into the provided array by means of "push_back",
/// as if it was an empty std::vector.
template < class Toplex_Template , class Parameter_Toplex_Template , class Map_Template , class Conley_Index >
void Compute_Conley_Morse_Graph (
  ConleyMorseGraph < typename Toplex_Template::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  Toplex_Template * phase_space ,
  const typename Toplex_Template::Geometric_Description & phase_space_box ,
  int subdivisions
);

/// Constructs the Conley-Morse decomposition for a single parameter box.
/// Runs a complete single box job as one of the worker's tasks.
/// Reads the computation parameters from the job message.
/// Saves the results of the computation in the result message.
template < class Toplex_Template, class Parameter_Toplex_Template , class Map_Template >
void Single_Box_Job ( Message * result, const Message & job ); 

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Single_Box_Job.hpp"
#endif

#endif
