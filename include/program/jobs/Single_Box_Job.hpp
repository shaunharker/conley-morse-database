/*
 *  Single_Box_Job.hpp
 */

#include "data_structures/Directed_Graph.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "algorithms/Conley_Index.h"


/// Computes the Morse decomposition of a given set
/// using the SCC algorithm.
template < class Toplex_Template , class Morse_Sets_Template , class Combinatorial_Map >
void Compute_Morse_Decomposition (
  typename Toplex_Template::Toplex_Subset * main_set ,
  Morse_Sets_Template * finer_morse_sets ,
  const typename Toplex_Template::Toplex_Subset & subdivided_morse_set ,
  const Toplex_Template & phase_space , const Combinatorial_Map & combinatorial_map
)
{
  // not yet implemented
  return;
} /* Compute_Morse_Decomposition */


/// Computes the Conley-Morse decomposition with respect to the given map
/// on the given phase space. The toplex representation of the phase space
/// is subdivided the given number of times.
/// The Conley-Morse graph is stored in the data structure provided
/// (which must be initially empty).
template < class Toplex_Template , class Parameter_Toplex_Template , class Map_Template , class Morse_Sets_Template >
void Compute_Conley_Morse_Graph (
  ConleyMorseGraph < typename Toplex_Template::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  Morse_Sets_Template * morse_sets ,
  Toplex_Template * phase_space ,
  const typename Toplex_Template::Geometric_Description & phase_space_box ,
  int subdivisions
)
{
  // short names for the types used in this function
  typedef Extendible_Array < typename Toplex_Template::Toplex_Subset > Morse_Sets;
  typedef typename Toplex_Template::Toplex_Subset Toplex_Subset;
  typedef ConleyMorseGraph < typename Toplex_Template::Toplex_Subset, Conley_Index > Conley_Morse_Graph;
  typedef Extendible_Array < Conley_Morse_Graph > Conley_Morse_Graphs;

  // create the objects of the maps
  Map_Template interval_map ( parameter_box );
  Combinatorial_Map < Toplex_Template , Map_Template > combinatorial_map ( phase_space , interval_map );

  // create the initial (trivial) Morse decomposition of the entire phase space
  Toplex_Subset empty_set;
  morse_sets. push_back ( empty_set );
  morse_sets [ 0 ] = phase_space. cover ( phase_space_box );
  conley_morse_graph. AddVertex ();
  conley_morse_graph. SetCubeSet ( 0 , & morse_sets [ 0 ] );

  // refine the Morse decomposition a few times
  for ( int subdiv = 0 ; subdiv < subdivisions ; ++ subdiv ) {
    // prepare an array for storing finer Morse sets
    Morse_Sets finer_morse_sets;

    // prepare an array for decompositions of the Morse sets
    Conley_Morse_Graphs finer_cmgraphs;

    // process all the individual Morse sets
    for ( Morse_Sets::iterator morse_set_iterator = morse_sets . begin () ;
    morse_set_iterator != morse_sets. end () ; ++ morse_set_iterator ) {
      // subdivide the Morse set in the coarser Morse decomposition
      const Toplex_Subset & morse_set ( * morse_set_iterator );
      Toplex_Subset subdivided_morse_set;
      for ( Toplex_Subset::const_iterator box_iterator = morse_set . begin () ;
      box_iterator != morse_set . end () ; ++ box_iterator ) {
        Toplex_Subset subdivided = phase_space . subdivide ( phase_space . find ( * box_iterator ) );
        for ( Toplex_Subset::const_iterator sub_iterator = subdivided . begin () ;
        sub_iterator != subdivided . end () ; ++ sub_iterator ) {
          subdivided_morse_set . insert ( * sub_iterator );
        }
      }

      // compute its Morse decomposition as SCCs of the graph of the map
      Conley_Morse_Graph finer_cmgraph;
      finer_cmgraphs . push_back ( finer_cmgraph );
      Compute_Morse_Decomposition ( & finer_cmgraphs . back () , & finer_morse_sets ,
        subdivided_morse_set , phase_space , combinatorial_map );
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
    finer_morse_sets. swap ( morse_sets );
    finer_morse_graph. swap ( conley_morse_graph );
  }
  return;
} /* Compute_Conley_Morse_Graph */


template < class Toplex_Template, class Parameter_Toplex_Template , class Map_Template >
void Single_Box_Job ( Message * result, const Message & job ) {
  // short names for the types used in this function
  typedef Extendible_Array < typename Toplex_Template::Toplex_Subset > Morse_Sets;
  typedef typename Toplex_Template::Geometric_Description Geometric_Description;
  typedef typename Parameter_Toplex_Template::Geometric_Description Parameter_Geometric_Description;
  typedef ConleyMorseGraph < typename Toplex_Template::Toplex_Subset, Conley_Index > Conley_Morse_Graph;

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
  job . close ();

  // create the object of the toplex for the given phase space box
  Toplex_Template phase_space ( phase_space_box );

  // a collection of Morse sets will be stored in an extendible array
  // in order to avoid copying of large objects
  Morse_Sets morse_sets;

  // compute the Conley-Morse graph
  Conley_Morse_Graph conley_morse_graph;
  Compute_Conley_Morse_Graph ( & conley_morse_graph , & morse_sets , & phase_space , phase_space_box , subdivisions );

  // prepare the message with the result of the computations
  result -> open_for_writing ();
  *result << job_number;
  *result << conley_morse_graph;
  result -> close ();
  return;
} /* Single_Box_Job */

