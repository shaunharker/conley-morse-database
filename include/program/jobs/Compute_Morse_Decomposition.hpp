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

  // Compute Morse sets along with their entrance and exit sets
  
  std::vector < typename Toplex::Subset > morse_sets;
  std::vector < typename Toplex::Subset > morse_exits;
  std::vector < typename Toplex::Subset > morse_entrances;
  
  computeSCC (& morse_sets,
              & morse_entrances,
              & morse_exits,
              entrance_subset,
              exit_subset,
              combinatorial_map );
  
  std::vector<size_t> ConnectingPathBounds;
  std::vector<size_t> EntrancePathBounds;
  std::vector<size_t> ExitPathBounds;
  
  computePathBounds( combinatorial_map, morse_sets, entrance_subset, exit_subset, /* inputs */
                     ConnectingPathBounds,
                     EntrancePathBounds,
                     ExitPathBounds,
                     *through_path_bound /* outputs */);
                    
  /* Loop through Morse Sets and construct a disconnected Conley Morse Graph (CMG) */
  const size_t number_of_morse_sets = morse_sets . size ();
  std::vector<typename Conley_Morse_Graph::Vertex> vertex_indexing ( number_of_morse_sets );
  for ( size_t index = 0; index < morse_sets . size (); ++ index ) {      
    typename Conley_Morse_Graph::Vertex new_vertex = conley_morse_graph -> AddVertex ();
    vertex_indexing [ index ] = new_vertex;
    conley_morse_graph -> SetCubeSet ( new_vertex, morse_sets [ index ] );
    std::swap ( exit_subsets -> operator [] ( new_vertex ), morse_exits [ index ] );
    exit_path_bounds -> operator [] ( new_vertex ) = ExitPathBounds [ index ];
    std::swap ( entrance_subsets -> operator [] ( new_vertex ), morse_entrances [ index ] );
    entrance_path_bounds -> operator [] ( new_vertex ) = EntrancePathBounds [ index ];
  } /* boost_foreach */
  
  /* Produce the Edges on the CMG */
  size_t index = 0;
  for ( unsigned int i = 0; i < number_of_morse_sets; ++ i ) {
    for ( unsigned int j = 0; j < number_of_morse_sets; ++ j ) {
      /* ConnectingPathBounds [ index ] tells us
          either a) the maximum number of steps to get from vertex i to vertex j
          or else b) that one cannot get from vertex i to vertex j */
      if ( ConnectingPathBounds [ index ] > 0 ) {
        /* TODO BUG: are i and j reversed? 50/50 chance this is right ;) */
        typename Conley_Morse_Graph::Vertex new_edge = conley_morse_graph -> 
          AddEdge (vertex_indexing [ i ], 
                   vertex_indexing [ j ] );
        path_bounds [ new_edge ] = ConnectingPathBounds [ index ];
        // if ( connecting_orbits != NULL ) connecting_orbits -> [ new_edge ] = /* ??? */;
      } /* if */
      ++ index;
    } /* for */
  } /* for */
  
  return;
} /* Compute_Morse_Decomposition */


#endif
