/*
 *  Compute_Morse_Decomposition.hpp
 */

#ifndef _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_
#define _CMDP_COMPUTE_MORSE_DECOMPOSITION_HPP_

#include "data_structures/Directed_Graph.h" /* For DirectedGraph<Toplex> */

template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Compute_Morse_Decomposition ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > * exit_subsets ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * exit_path_bounds ,
  std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > * entrance_subsets ,
  std::map < typename Conley_Morse_Graph::Vertex , size_t > * entrance_path_bounds ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > * connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , size_t > * path_bounds ,
  size_t * through_path_bound ,
  const Toplex & phase_space ,
  const typename Toplex::Subset & set_to_decompose ,
  const typename Toplex::Subset & exit_subset ,
  const typename Toplex::Subset & entrance_subset ,
  Combinatorial_Map & combinatorial_map ) {

  // --- not yet implemented ---

  // suggested outline:
  // 1) create a subgraph of the combinatorial map
  //    restricted to the given set to decompose
  // 2) mark those vertices in the set to decompose which have edges coming in from outside
  // 3) mark those vertices in the set to decompose which have edges going out to outside
  
  /* Naive construction of a subgraph, entrance_set, and exit set:
     subgraph is copied, entrance_set is assumed to be entire subgraph,
     and exit set is detected accurately*/
  DirectedGraph<Toplex> subgraph;
  typename Toplex::Subset Exit;
  const typename Toplex::Subset & Entrance = set_to_decompose;
  
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, set_to_decompose ) {
    /* intersect should be defined along with Toplex */
    typename Toplex::Subset image = combinatorial_map ( cell );
    BOOST_FOREACH ( typename Toplex::Top_Cell image_cell, image ) {
      if ( set_to_decompose . find ( image_cell ) == set_to_decompose . end () ) {
        Exit . insert ( cell );
      } else {
        subgraph [ cell ] . insert ( image_cell );
      } /* if-else */
    } /* boost_foreach */
  } /* boost_foreach */

  // 4) call Zin's function for computing SCCs
  //    and the strict upper bounds for the path lengths
  
  // computeSCC will allocate Morse Sets and return them.
  typename DirectedGraph<Toplex>::Components SCC = computeSCC ( subgraph );
  
  std::vector<size_t> ConnectingPathBounds;
  std::vector<size_t> EntrancePathBounds;
  std::vector<size_t> ExitPathBounds;
  
  computePathBounds( subgraph, SCC, Entrance, Exit, /* inputs */
                     ConnectingPathBounds,
                     EntrancePathBounds,
                     ExitPathBounds,
                     *through_path_bound /* outputs */);
                    
  // 5) copy and adjust the Morse sets returned by Zin's function
  //    to the Conley-Morse graph object
  
  /* Loop through SCC (a vector of Toplex::Subset's) and construct a disconnected CMG */
  std::vector<typename Conley_Morse_Graph::Vertex> vertex_indexing ( SCC . size () );
  size_t index = 0;
  BOOST_FOREACH ( Toplex::Subset * morse_set, SCC ) {
    typename Conley_Morse_Graph::Vertex new_vertex = conley_morse_graph -> AddVertex ();
    vertex_indexing [ index ] = new_vertex;
    conley_morse_graph -> SetCubeSet ( new_vertex, morse_set );
    entrance_path_bounds -> operator [] ( new_vertex ) = EntrancePathBounds [ index ];
    exit_path_bounds -> operator [] ( new_vertex ) = ExitPathBounds [ index ];
    ++ index;
  } /* boost_foreach */
  
  index = 0;
  for ( unsigned int i = 0; i < SCC . size (); ++ i ) {
    for ( unsigned int j = 0; j < SCC . size (); ++ j ) {
      /* ConnectingPathBounds [ index ] tells us
          either a) the maximum number of steps to get from vertex i to vertex j
          or else b) that one cannot get from vertex i to vertex j */
      if ( ConnectingPathBounds [ index ] > 0 ) {
        /* TODO BUG: are i and j reversed? 50/50 chance this is right ;) */
        typename Conley_Morse_Graph::Vertex new_edge = conley_morse_graph -> AddEdge (i, j)
        path_bounds [ new_edge ] = ConnectingPathBounds [ index ];
        // if ( connecting_orbits != NULL ) connecting_orbits -> [ new_edge ] = /* ??? */;
      } /* if */
      ++ index;
    } /* for */
  } /* for */
  
  return;
} /* Compute_Morse_Decomposition */


#endif
