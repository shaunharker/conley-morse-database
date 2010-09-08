/*
 *  Compute_Conley_Morse_Graph.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_

#include <map>
#include <utility>
#include <vector>

#include "program/Configuration.h"
#include "program/jobs/Compute_Path_Bounds.h"
#include "program/jobs/Compute_Morse_Decomposition.h"


/// Determines possible connections in the final Morse decomposition
/// and computes the combinatorial outer bounds for the connecting orbits,
/// based on the hierarchy of Morse decompositions at all the levels.
/// Also checks whether each set is a full repeller (no incoming edges)
/// or a full attractor (no outgoing edges) to make better decisions.
template < class Conley_Morse_Graph , class Combinatorial_Map >
void Determine_All_Connections ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > * connecting_orbits ,
  const Combinatorial_Map & combinatorial_map ,
  std::map < typename Conley_Morse_Graph::Vertex , Conley_Morse_Graph const * > & original_cmg ,
  std::map < typename Conley_Morse_Graph::Vertex , typename Conley_Morse_Graph::Vertex > & original_set ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > & finer_cmg ,
  std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > & coarser_cmg ,
  std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > & coarser_set ) {

  // --- not yet implemented ---
  return;
} /* Determine_All_Connections */

/// Tries to rule out connections between Morse sets, based on iterations
/// of the map that acts on geometric descriptions (an interval map,
/// as opposed to using the combinatorial map on the toplex).
/// Removes those edges from the graph for which it can prove that
/// all the iterations of one set (up to the given path length bound)
/// are disjoint from the other set.
template < class Conley_Morse_Graph , class Map , class Toplex >
void Rule_Out_Connections ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > & connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , long > & path_bounds ,
  const Map & interval_map , const Toplex & phase_space ) {

  // --- not yet implemented ---
  return;
} /* Rule_Out_Connections */

// --------------------------------------------------

template < class Toplex , class Parameter_Toplex ,
  class Map , class Conley_Index ,
  class Decide_Subdiv , class Decide_Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  const Decide_Subdiv & decide_subdiv ,
  const Decide_Conley_Index & decide_conley_index ,
  Cached_Box_Informatin * cached_box_information ) {

  // short names for the types used in this function
  typedef typename Toplex::Toplex_Subset Toplex_Subset;
  typedef ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > Conley_Morse_Graph;
  typedef std::vector < Conley_Morse_Graph * > Conley_Morse_Graphs;

  // create the objects of the maps
  Map interval_map ( parameter_box );
  Combinatorial_Map < Toplex , Map > combinatorial_map ( phase_space , interval_map );

  // prepare maps for pointing the final Morse sets to where they came from
  std::map < typename Conley_Morse_Graph::Vertex , Conley_Morse_Graph const * > original_cmg;
  std::map < typename Conley_Morse_Graph::Vertex , typename Conley_Morse_Graph::Vertex > original_set;

  // prepare a map for pointing Morse sets to their subdivisions
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > finer_cmg;

  // prepare maps for pointing a Morse decomposition to the Morse set
  // in the coarser Morse decomposition
  std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > coarser_cmg;
  std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > coarser_set;

  // prepare maps for pointing CMGs to the connecting orbits computed for them
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > > all_connecting_orbits;

  // prepare maps for pointing CMGs to the path length bounds computed for them
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , long > > exit_path_bounds;
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , long > > entrance_path_bounds;
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , long > > path_bounds;
  std::map < Conley_Morse_Graph const * , long > through_path_bound;

  // create an array to store all the computed Morse decompositions
  Conley_Morse_Graphs conley_morse_graphs;

  // create the initial (trivial) Morse decomposition of the entire phase space
  conley_morse_graphs . push_back ( new Conley_Morse_Graph );
  conley_morse_graphs [ 0 ] -> AddVertex ();
  conley_morse_graphs [ 0 ] -> SetCubeSet ( new Toplex_Subset );
  conley_morse_graphs [ 0 ] -> GetCubeSet ( * ( conley_morse_graphs [ 0 ] -> Vertices () ) . first () ) ->
    cover ( phase_space_box );

  // prepare vectors for storing the number of the first Conley-Morse
  // decomposition and the one-after-the-last one at each subdivision level
  std::vector < size_t > cmg_subdiv_begin;
  std::vector < size_t > cmg_subdiv_end;

  // prepare a mapping for storing connecting orbits in the final Morse decomposition
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Toplex_Subset > connecting_orbits;

  // prepare the current subdivision level (subdividing the entire
  // phase space corresponds to the subdivision level 0);
  // after the loop, this number stores the strict upper bound
  // on the subdivision levels that were used
  size_t subdiv = 0;

  // repeat subdivision steps to Conley-Morse decompositions waiting in a queue
  for ( ; ; ++ subdiv ) {
    // determine the first and one-after-the-last CMG to try subdividing
    size_t cmg_begin = subdiv ? cmg_subdiv_end [ subdiv - 1 ] : 0;
    size_t cmg_end = conley_morse_graphs . size ();
    if (cmg_begin == cmg_end)
      break;
    cmg_subdiv_begin . push_back ( cmg_begin );
    cmg_subdiv_end . push_back ( cmg_end );

    // go through all the CMGs and consider subdividing their Morse sets
    for ( size_t cmg_number = cmg_begin ; cmg_number != cmg_end ; ++ cmg_number ) {
      // extract the Conley-Morse graph
      Conley_Morse_Graph * current_cmg = conley_morse_graphs [ cmg_number ];

      // process all the individual Morse sets
      typename Conley_Morse_Graph::VertexIteratorPair vertices = current_cmg -> Vertices ();
      for ( Conley_Morse_Graph::VertexIterator morse_set_iterator = vertices . first () ;
        morse_set_iterator != vertices . second () ; ++ morse_set_iterator )
      {
        // determine the Morse set
        typename Conley_Morse_Graph::Vertex current_vertex = * morse_set_iterator;
        Toplex_Subset * morse_set ( current_cmg -> GetCubeSet ( current_vertex ) );

        // if the Morse set should not be subdivided anymore
        if ( ! decide_subdiv ( subdiv , morse_set ) ) {
          // add the Morse set to the final decomposition and set up the right links
          typename Conley_Morse_Graph::Vertex new_vertex = conley_morse_graph . AddVertex ();
          original_cmg [ new_vertex ] = current_cmg;
          original_set [ new_vertex ] = current_vertex;

          // move or compute its Conley index
          Conley_Index * conley_index ( current_cmg . GetConleyIndex ( current_vertex ) );
          if ( ( conley_index == 0 ) && decide_conley_index . compute_final ( morse_set ) ) {
            // TODO: Compute the Conley index of the Morse set
          }
          conley_morse_graph -> SetConleyIndex ( new_vertex , conley_index );
          current_cmg -> SetConleyIndex ( current_vertex , 0 );

          // move the cubical representation of the Morse set
          conley_morse_graph -> SetCubeSet ( new_vertex , morse_set );
          current_cmg -> SetCubeSet ( current_vertex , 0 );
          continue;
        }

        // subdivide the Morse set and compute its Morse decomposition
        phase_space -> subdivide ( morse_set );
        Conley_Morse_Graph * new_cmg = new Conley_Morse_Graph;
        conley_morse_graphs -> push_back ( new_cmg );
        Compute_Morse_Decomposition < Toplex, Conley_Morse_Graph , Combinatorial_Map > ( new_cmg ,
          & ( exit_path_bounds [ new_cmg ] ) , & ( entrance_path_bounds [ new_cmg ] ) ,
          & ( all_connecting_orbits [ new_cmg ] ) , & ( path_bounds [ new_cmg ] ) ,
          & ( through_path_bound [ new_cmg ] ) , morse_set , combinatorial_map );

        // compute the Conley indices of the constructed Morse sets
        typename Conley_Morse_Graph::VertexIteratorPair new_vertices = new_cmg -> Vertices ();
        for ( Conley_Morse_Graph::VertexIterator morse_set_iterator = new_vertices . first () ;
          morse_set_iterator != new_vertices . second () ; ++ morse_set_iterator )
        {
          if ( ( new_cmg -> NumVertices () == 1 ) && ( current_cmg -> GetConleyIndex ( current_vertex ) ) ) {
            ConleyIndex * new_index = new ConleyIndex;
            * new_index = * ( current_cmg -> GetConleyIndex ( current_vertex ) );
            new_cmg -> SetConleyIndex ( new_index );
          }
          if ( ! decide_conley_index . compute_after_subdivision ( subdiv , morse_set ) )
            continue;
          // TODO:  Compute the Conley index of the Morse set
        }

        // mark the links between the coarser and finer decompositions
        finer_cmg [ std::make_pair ( current_cmg , current_vertex ) ] = new_cmg;
        coarser_cmg [ new_cmg ] = current_cmg;
        coarser_set [ new_cmg ] = current_vertex;

        // forget the Conley index and cubical representation of the subdivided set
        current_cmg -> SetConleyIndex ( current_vertex , 0 );
        current_cmg -> SetCubeSet ( current_vertex , 0 );
      }
    }
  }

  // determine connections between the Morse sets in the final graph,
  // based on the coarser Morse sets on the way
  Determine_All_Connections ( & conley_morse_graph , & connecting_orbits , combinatorial_map ,
    original_cmg , original_set , finger_cmg , coarser_cmg , coarser_set );

  // compute the upper bounds for connection lengths between the Morse sets
  std::map < typename Conley_Morse_Graph::Edge , long > path_bounds;
  Compute_Path_Bounds ( & path_bounds , conley_morse_graph ,
    original_cmg , original_set , finger_cmg , coarser_cmg , coarser_set ,
    exit_path_bounds , entrance_path_bounds , path_bounds , through_path_bound );

  // determine which connections between Morse sets are suprious
  // and remove them from the final Conley-Morse graph
  Rule_Out_Connections ( & conley_morse_graph , connecting_orbits , path_bounds , interval_map , * phase_space );

/*
  // free up the dynamically allocated memory
  // for the intermediate Morse decompositions
  for ( Conley_Morse_Graphs::iterator cmg_iterator = conley_morse_graphs . begin ();
    cmg_iterator != conley_morse_graphs . end (); ++ cmg_iterator )
  {
    Conley_Morse_Graph * cmg = * cmg_iterator;
    typename Conley_Morse_Graph::VertexIteratorPair vertices = cmg -> Vertices ();
    for ( Conley_Morse_Graph::VertexIterator morse_set_iterator = vertices . first () ;
      morse_set_iterator != vertices . second () ; ++ morse_set_iterator )
    {
      Toplex_Subset * morse_set = cmg -> GetCubeSet ( * morse_set_iterator );
      if ( morse_set )
        delete morse_set;
      Conley_Index * conley_index = cmg -> GetConleyIndex ( * morse_set_iterator );
      if ( conley_index )
        delete conley_index;
    }
    delete cmg;
  }
*/
  return;
} /* Compute_Conley_Morse_Graph */


#endif
