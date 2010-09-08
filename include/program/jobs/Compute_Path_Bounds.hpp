/*
 *  Compute_Path_Bounds.hpp
 */

#ifndef _CMDP_COMPUTE_PATH_BOUNDS_HPP_
#define _CMDP_COMPUTE_PATH_BOUNDS_HPP_

// In this version, I consider only the direct path between morse sets. This MUST be modified.
template < class Conley_Morse_Graph >
void Compute_Path_Bounds ( std::map < typename Conley_Morse_Graph::Edge , size_t > *return_path_bounds ,
                           const Conley_Morse_Graph & conley_morse_graph ,
                           std::map < typename Conley_Morse_Graph::Vertex , Conley_Morse_Graph const * > & original_cmg ,
                           std::map < typename Conley_Morse_Graph::Vertex , typename Conley_Morse_Graph::Vertex > & original_set ,
                           std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > & finer_cmg ,
                           std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > & coarser_cmg ,
                           std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > & coarser_set ,
                           std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , size_t > > & exit_path_bounds ,
                           std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , size_t > > & entrance_path_bounds ,
                           std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , size_t > > & path_bounds ,
                           std::map < Conley_Morse_Graph const * , size_t > & through_path_bound ) {
  
  // Now I get an iterator pair of all edges.
  typename Conley_Morse_Graph::EdgeIteratorPair itrPairEdgesCMG( conley_morse_graph.Edges() );
  
  // Check all the edges in the final conley morse graph iterately.
  for ( typename Conley_Morse_Graph::EdgeIterator itrEdge( itrPairEdgesCMG.first );
        itrEdge != itrPairEdgesCMG.second;
        ++itrEdge ) {
    // Get the edge between two morse sets in the final conley morse graph.
    typename Conley_Morse_Graph::Edge edge( *itrEdge );
    
    // Get the source and target vertices of the edge.
    typename Conley_Morse_Graph::Vertex sourceVertex( conley_morse_graph.Source( edge ) );
    typename Conley_Morse_Graph::Vertex targetVertex( conley_morse_graph.Target( edge ) );
    
    // Determine whether the vertices belong to the same conley morse graph.
    // Get the original conley morse graphs.
    const Conley_Morse_Graph const *originalCMGSourceVertex( original_cmg[ sourceVertex ] );
    const Conley_Morse_Graph const *originalCMGTargetVertex( original_cmg[ targetVertex ] );

    // If the vertices belong to the same conley morse graph, I simply copy the path length between them from path_bounds.
    if ( originalCMGSourceVertex == originalCMGTargetVertex ) {
      // They belong to the same conley morse graph, so I simply copy the path_bounds.
      return_path_bound->insert( *( path_bounds[ originalCMGTargetVertex ].find( edge ) ) );
    } else {
      // They belong to the different conley morse graphs.
      // First, examine whether they belong to the same conley morse graph in a coaser scale.
      // Get vectors of coarser conley morse graphs.
      std::vector< Conley_Morse_Graph const * > coarserCMGSourceVertex;
      std::vector< Conley_Morse_Graph const * > coarserCMGTargetVertex;
      
      // The first component is a original conley morse graph
      coarserCMGSourceVertex.insert( originalCMGSourceVertex );
      coarserCMGTargetVertex.insert( originalCMGTargetVertex );

      // Then, insert coaser conley morse graphs if exist.
      Search_Coarser_Conley_Morse_Graph( coarserCMGSourceVertex, coarser_cmg, originalCMGSourceVertex );
      Search_Coarser_Conley_Morse_Graph( coarserCMGTargetVertex, coarser_cmg, originalCMGTargetVertex );

      // Check whether the vertices belong to the same conley morse graph in the coarsest level.
      if ( coarserCMGSourceVertex.back() != coarserCMGTargetVertex.back() ) {
        // They don't belong to the same conley morse graph, that means that there are no connections between them.
        return_path_bounds->insert( std::pair< typename Conley_Morse_Graph::Edge, size_t >( edge, 0 ) );
      } else {
        // They belong to the same conley morse graph in some coarser scales and have a connection.
        typename std::vector< Conley_Morse_Graph const * >::iterator itrCommonFinestCMGSourceVertex( std::find( coarserCMGSourceVertex.begin(), coarserCMGSourceVertex.end(), coarserCMGSourceVertex.back() ) );
        typename std::vector< Conley_Morse_Graph const * >::iterator itrCommonFinestCMGTargetVertex( std::find( coarserCMGTargetVertex.begin(), coarserCMGTargetVertex.end(), coarserCMGTargetVertex.back() ) );
        
        // Get a path length between Morse sets at the common coarser level
        // This considers only the direct connection, and MUST be modified later.
        // Get a edge between vertices;
        std::pair< typename Conley_Morse_Graph::Edge, bool > coarserEdge( ( *itrCommonFinestCMGTargetVertex )->GetEdge( coarser_set[ *( itrCommonFinestCMGSourceVertex - 1 ) ], coarser_set[ *( itrCommonFinestCMGTargetVertex - 1 ) ] ) );
        if ( !coarserEdge.second ) {
          // There are no direct edges.
          // I simply return 0. Is it right?
          return_path_bounds->insert( std::pair< typename Conley_Morse_Graph::Edge, size_t >( edge, 0 ) );
        } else {
          // There is a direct edge, and I retrieve the path length between them.
          const size_t pathBoundsAtCommonLevel( path_bounds[ *itrCommonFinestCMGTargetVertex ][ coarserEdge.first] );
          
          // Calculate the exit path bounds
          size_t exitPathBounds( Extract_Exit_Path_Bounds( exit_path_bounds, originalCMGSourceVertex, original_set[ sourceVertex ] ) );
          for ( typename std::vector< Conley_Morse_Graph const * >::iterator itrCMGSourceVertex( coarserCMGVertex.begin() );
                itrCMGSourceVertex != itrCommonFinestCMGSourceVertex - 1;
                ++itrCMGSourceVertex ) {
            exitPathBounds += Extract_Exit_Path_Bounds( exit_path_bounds, *( itrCMGSourceVertex + 1 ), coarser_set[ *itrCMGSourceVertex ] );
          }
          
          // Calculate the entrance path bounds
          size_t entrancePathBounds( Extract_Entrance_Path_Bounds( entrance_path_bounds, originalCMGTargetVertex, original_set[ targetVertex ] ) );
          for ( typename std::vector< Conley_Morse_Graph const * >::iterator itrCMGTargetVertex( coarseCMGTargetVertex.begin() );
                itrCMGTargetVertex != itrCommonFinestCMGTargetVertex - 1;
                ++itrCMGTargetVertex ) {
            entrancePathBounds += Extract_Entrance_Path_Bounds( entrance_path_bounds, *( itrCMGTargetVertex + 1 ), coarser_set[ *itrCMGTargetVertex ] );
          }
        }
        return_path_bound->insert( std::pair< typename Conley_Morse_Graph::Edge, size_t >( edge, pathBoundsAtCommonLevel + exitPathBounds + entrancePathBounds ) );
      }
    }
  }
} /* Compute_Path_Bounds */

// This function returns a vector of pointers to coaser conley morse graphs
template< class Conley_Morse_Graph >
void Search_Coarser_Conley_Morse_Graph( std::vector< Conley_Morse_Graph const * > &searched_conley_morse_graph,
                                        std::map< Conley_Morse_Graph const *, Conley_Morse_Graph const * > &coarser_cmg,
                                        const Conley_Morse_Graph const *conley_morse_graph ) {
  // Looking for a coaser conley morse graph
  typename std::map< Conley_Morse_Graph const *, Conley_Morse_Graph const * >::iterator itrCoarserCMG( coarser_cmg.find( conley_morse_graph ) );
  if ( itrCoaserCMG != coarser_cmg.end() ) {
    // A coarser conley morse graph is founded.
    searched_conley_morse_graph.insert( ( *itrCoarserCMG ).second );

    // We iterately look for coaser conley morse graphs.
    Search_Coarser_Conley_Morse_Graph( searched_conley_morse_graph, coarser_cmg, ( *itrCoarserCMG ).second );
  }
} /* Search_Coarser_Conley_Morse_Graph */

// this function returns the exit_path_bounds for given morse set and conley morse graph
template< class Conley_Morse_Graph >
size_t Extract_Exit_Path_Bounds( std::map< Conley_Morse_Graph const *, std::map< typename Conley_Morse_Graph::Vertex, size_t > > &exit_path_bounds,
                                 const Conley_Morse_Graph const *conley_morse_graph,
                                 const typename Conley_Morse_Graph::Vertex vertex ) {
  return exit_path_bounds[ conley_morse_graph ][ vertex ];
} /* Extract_Exit_Path_Bounds */

// this function returns the entrance_path_bound for given morse set and conley morse graph
template< class Conley_Morse_Graph >
size_t Extract_Entrance_Path_Bounds( std::map< Conley_Morse_Graph const *, std::map< typename Conley_Morse_Graph::Vertex, size_t > > &entrance_path_bounds,
                                     const Conley_Morse_Graph const *conley_morse_graph,
                                     const typename Conley_Morse_Graph::Vertex vertex ) {
  return entrance_path_bounds[ conley_morse_graph ][ vertex ];
} /* Extract_Entrance_Path_Bounds */

#endif
