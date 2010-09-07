/*
 *  Compute_Conley_Morse_Graph.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_

#include <map>
#include <vector>


template < class Toplex , class Parameter_Toplex ,
  class Map , class Conley_Index , class Subdiv_Decide , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > * conley_morse_graph ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  Toplex * phase_space ,
  const typename Toplex::Geometric_Description & phase_space_box ,
  const Subdiv_Decide & subdiv_decide ,
  Cached_Box_Informatin * cached_box_information ) {

  // short names for the types used in this function
  typedef typename Toplex::Toplex_Subset Toplex_Subset;
  typedef ConleyMorseGraph < typename Toplex::Toplex_Subset, Conley_Index > Conley_Morse_Graph;
  typedef std::vector < Conley_Morse_Graph * > Conley_Morse_Graphs;

  // create the objects of the maps
  Map interval_map ( parameter_box );
  Combinatorial_Map < Toplex , Map > combinatorial_map ( phase_space , interval_map );

  // prepare a map for pointing Morse sets to their subdivisions
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > finer_cmg;

  // prepare maps for pointing a Morse decomposition to the Morse set
  // in the coarser Morse decomposition
  std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > coarser_cmg;
  std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > coarser_set;

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
  conley_morse_graphs [ 0 ] -> GetCubeSet ( * ( conley_morse_graph -> Vertices () ) . first () ) -> cover ( phase_space_box );

  // prepare vectors for storing the number of the first Conley-Morse
  // decomposition and the one-after-the-last one at each subdivision level
  std::vector < size_t > cmg_subdiv_begin;
  std::vector < size_t > cmg_subdiv_end;

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
      Conley_Morse_Graph * conley_morse_graph = conley_morse_graphs [ cmg_number ];

      // process all the individual Morse sets
      typename Conley_Morse_Graph::VertexIteratorPair vertices = conley_morse_graph -> Vertices ();
      for ( Conley_Morse_Graph::VertexIterator morse_set_iterator = vertices . first () ;
        morse_set_iterator != vertices . second () ; ++ morse_set_iterator )
      {
        // check if the Morse set should be subdivided
        
        // subdivide the Morse set if decided to
        Toplex_Subset * morse_set ( conley_morse_graph -> GetCubeSet ( * morse_set_iterator ) );
        phase_space . subdivide ( morse_set );
      }
    }

    // = = =   U N D E R   C O N S T R U C T I O N   = = =

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

  }



  // determine which connections between Morse sets are suprious
  // and remove them from the final Conley-Morse graph
  // TODO

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
      delete morse_set;
      Conley_Index * conley_index = cmg -> GetConleyIndex ( * morse_set_iterator );
      delete conley_index;
    }
    delete cmg;
  }
  return;
} /* Compute_Conley_Morse_Graph */


#endif
