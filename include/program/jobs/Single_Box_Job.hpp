/*
 *  Single_Box_Job.hpp
 */

#ifndef _CMDP_SINGLE_BOX_JOB_HPP_
#define _CMDP_SINGLE_BOX_JOB_HPP_

#include <map>

#include "data_structures/Directed_Graph.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "algorithms/Conley_Index.h"

#include "program/jobs/Compute_Path_Bounds.h"


template < class Toplex, class Parameter_Toplex ,
  class Map , class Conley_Index , class Cached_Box_Information >
void Single_Box_Job ( Message * result, const Message & job ) {

  // short names for the types used in this function
  typedef std::vector < typename Toplex::Toplex_Subset > Morse_Sets;
  typedef typename Toplex::Geometric_Description Geometric_Description;
  typedef typename Parameter_Toplex::Geometric_Description Parameter_Geometric_Description;
  typedef ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > Conley_Morse_Graph;

  // read the job parameters from the input message
  job . open_for_reading ();
  long job_number ( 0 );
  job >> job_number;
  Parameter_Geometric_Description parameter_box;
  job >> parameter_box;
  Geometric_Description phase_space_box;
  job >> phase_space_box;
  int subdivisions ( 0 );
  job >> subdivisions;
  Cached_Box_Informatin cached_box_information;
  job >> cached_box_information;
  job . close ();

  // create the object of the toplex for the given phase space box
  Toplex phase_space ( phase_space_box );

  // a collection of Morse sets will be stored in an std::vector;
  // note that this might be inefficient because of copying large objects
  Morse_Sets morse_sets;

  // compute the Conley-Morse graph
  Conley_Morse_Graph conley_morse_graph;
  Compute_Conley_Morse_Graph < Toplex , Parameter_Toplex , Map , Conley_Index , Cached_Box_Information> (
    & conley_morse_graph , parameter_box , & phase_space , phase_space_box ,
    subdivisions , & cached_box_information );

  // prepare the message with the result of the computations
  result -> open_for_writing ();
  *result << job_number;
  *result << conley_morse_graph;
  *result << cached_box_information;
  result -> close ();
  return;
} /* Single_Box_Job */


#endif
