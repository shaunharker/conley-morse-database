/*
 *  Compute_Conley_Morse_Graph.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_


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


#endif
