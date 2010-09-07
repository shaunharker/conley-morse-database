/*
 *  Single_Box_Job.hpp
 */

#ifndef _CMDP_SINGLE_BOX_JOB_HPP_
#define _CMDP_SINGLE_BOX_JOB_HPP_

#include <map>

#include "data_structures/Directed_Graph.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "algorithms/Conley_Index.h"


/// Computes the Morse decomposition of a given set
/// using the SCC algorithm. The Conley Morse graph object
/// given in the input must be initially empty, and is filled in
/// the associated data structures with all the necessary information:
/// strict upper bounds for the lengths of paths that begin at the
/// Morse sets and exit the domain,
/// strict upper bounds for the lengths of paths that enter the domain
/// and then enter the Morse sets,
/// and strict upper bounds for the connecting path lengths.
/// The provided vectors must be initially empty.
template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , long > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , long > * path_bounds ,
  const typename Toplex::Toplex_Subset & domain ,
  const Combinatorial_Map & combinatorial_map ) {
  // not yet implemented: Zin
  return;
} /* Compute_Morse_Decomposition */


/// Computes a strict upper bound on the length of a connecting orbit
/// between the Morse sets.
template < class Conley_Morse_Graph >
void Compute_Path_Bounds ( std::map < typename Conley_Morse_Graph::Edge , long > * path_bounds // UNDER CONSTRUCTION
) {
  // not yet implemented: Masaki
} /* Compute_Path_Bounds */


template < class Toplex , class Parameter_Toplex ,
  class Map , class Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  int subdivisions ,
  Cached_Box_Informatin * cached_box_information ) {
  // short names for the types used in this function
  typedef typename Toplex::Toplex_Subset Toplex_Subset;
  typedef ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > Conley_Morse_Graph;
  typedef std::vector < Conley_Morse_Graph > Conley_Morse_Graphs;

  // create the objects of the maps
  Map interval_map ( parameter_box );
  Combinatorial_Map < Toplex , Map > combinatorial_map ( phase_space , interval_map );

  // create the initial (trivial) Morse decomposition of the entire phase space
  conley_morse_graph -> AddVertex ();
  conley_morse_graph -> SetCubeSet ( new Toplex_Subset );
  conley_morse_graph -> GetCubeSet ( * ( conley_morse_graph -> Vertices () ) . first () ) -> cover ( phase_space_box );

  // refine the Morse decomposition a few times
  for ( int subdiv = 0 ; subdiv < subdivisions ; ++ subdiv ) {
    // prepare an array for decompositions of the Morse sets
    Conley_Morse_Graphs finer_cmgraphs;

    // process all the individual Morse sets
    typename Conley_Morse_Graph::VertexIteratorPair vertices = conley_morse_graph -> Vertices ();
    for ( Conley_Morse_Graph::VertexIterator morse_set_iterator = vertices . first () ;
    morse_set_iterator != vertices . second () ; ++ morse_set_iterator ) {
      // subdivide the Morse set in the coarser Morse decomposition
      Toplex_Subset * morse_set ( conley_morse_graph -> GetCubeSet ( * morse_set_iterator ) );
      phase_space . subdivide ( morse_set );

      // compute its Morse decomposition as SCCs of the graph of the map
      Conley_Morse_Graph finer_cmgraph;
      Compute_Morse_Decomposition ( & finer_cmgraph , * morse_set , combinatorial_map );
      finer_cmgraphs . push_back ( finer_cmgraph );

      // compute the Conley indices (?) and simplify the Morse decomposition (?)
      // ...
    }

    // create a refined Morse decomposition of the entire phase space
    // from the Morse decompositions of single Morse sets
    Conley_Morse_Graph finer_morse_graph;
    for ( size_t n = finer_morse_sets . size (); n > 0; -- n)
    {
      finer_morse_graph . AddVertex ()
    }

    // determine which connections between Morse sets persist
    // and add the corresponding relations to the finer Conley-Morse graph
    // (not yet implemented)

    // swap the objects for the next iteration
    finer_morse_graph . swap ( * conley_morse_graph );
  }
  return;
} /* Compute_Conley_Morse_Graph */


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
