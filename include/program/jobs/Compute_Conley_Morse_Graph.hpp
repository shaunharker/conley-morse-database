/*
 *  Compute_Conley_Morse_Graph.hpp
 */

#ifndef _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_
#define _CMDP_COMPUTE_CONLEY_MORSE_GRAPH_HPP_

#include <map>
#include <utility>
#include <vector>
#include <cstddef>
#include <memory>
#include <set>
#include <algorithm>

#include "program/Configuration.h"
#include "program/jobs/Compute_Path_Bounds.h"
#include "program/jobs/Compute_Morse_Decomposition.h"
#include "algorithms/Homology.h"


/// Computes the union of all the final sets that arise from the given
/// Conley-Morse graph. Uses recursion to take into consideration
/// all the successive subdivisions of each Morse set.
/// This is a helper recursive procedure
/// for the function "Determine_All_Connections".
template < class Conley_Morse_Graph >
void Compute_Final_Sets ( std::set < typename Conley_Morse_Graph::Vertex > * all_of_them ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > ,
    std::set < typename Conley_Morse_Graph::Vertex > > * final_sets ,
  Conley_Morse_Graph const * current_cmg ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > ,
    typename Conley_Morse_Graph::Vertex > & final_set ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > ,
    Conley_Morse_Graph const * > & finer_cmg ) {

  // go through all the vertices in the given Conley-Morse graph
  typename Conley_Morse_Graph::VertexIteratorPair vertices = current_cmg -> Vertices ();
  for ( typename Conley_Morse_Graph::VertexIterator vertex_iter = vertices . first ;
    vertex_iter != vertices . second ; ++ vertex_iter )
  {
    // prepare a pair consisting of the current C-M graph and the vertex
    std::pair < Conley_Morse_Graph const * ,
      typename Conley_Morse_Graph::Vertex >
      current_vertex_pair = std::make_pair ( current_cmg , * vertex_iter );

    // find this pair in the domain of the final set mapping
    typedef std::map < std::pair < Conley_Morse_Graph const * ,
      typename Conley_Morse_Graph::Vertex > ,
      typename Conley_Morse_Graph::Vertex > Final_Set_Type;
    typename Final_Set_Type::iterator final_set_iter =
      final_set . find ( current_vertex_pair );

    // determine the set corresponding to this pair
    std::set < typename Conley_Morse_Graph::Vertex > & new_set =
      ( * final_sets ) [ current_vertex_pair ];

    // if the vertex corresponds to a final Morse set then set the value
    if ( final_set_iter != final_set . end () ) {
      new_set . insert ( final_set_iter -> second );
    }

    // otherwise use recursion to construct the set
    else {
      const Conley_Morse_Graph * new_cmg = finer_cmg [ current_vertex_pair ];
      Compute_Final_Sets ( & new_set , final_sets , new_cmg , final_set, finer_cmg );
    }

    // add the set to the union of these sets
    all_of_them -> insert ( new_set . begin () , new_set . end () );
  }
  return;
} /* Compute_Final_Sets */

// --------------------------------------------------

/// Determines possible connections in the final Morse decomposition
/// and computes the combinatorial outer bounds for the connecting orbits,
/// based on the hierarchy of Morse decompositions at all the levels.
/// Also checks whether each set is a full repeller (no incoming edges)
/// or a full attractor (no outgoing edges) to make better decisions.
template < class Toplex , class Conley_Morse_Graph , class Combinatorial_Map >
void Determine_All_Connections ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > * connecting_orbits ,
  const std::vector < Conley_Morse_Graph * > & conley_morse_graphs ,
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > > & all_connecting_orbits ,
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > > & exit_subsets ,
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > > & entrance_subsets ,
  std::map < typename Conley_Morse_Graph::Vertex , Conley_Morse_Graph const * > & original_cmg ,
  std::map < typename Conley_Morse_Graph::Vertex , typename Conley_Morse_Graph::Vertex > & original_set ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > & finer_cmg ,
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , typename Conley_Morse_Graph::Vertex > & final_set ,
  std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > & coarser_cmg ,
  std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > & coarser_set ) {

  // some typedefs to simplify the definitions
  typedef std::set < typename Conley_Morse_Graph::Vertex > Set_Of_Vertices;
  typedef std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > CMG_Vertex_Pair;
      
  // create a mapping that assigns to each vertex of each CMG
  // the set of final vertices that come from that vertex
  // (as Morse subsets appearing in finer subdivisions)
  std::map < CMG_Vertex_Pair , Set_Of_Vertices > final_sets;
  Set_Of_Vertices all_of_them;
  Compute_Final_Sets < Conley_Morse_Graph > ( & all_of_them ,
    & final_sets , conley_morse_graphs [ 0 ] , final_set , finer_cmg );

  // for every Conley-Morse graph in the list of Conley-Morse graphs
  for ( typename std::vector < Conley_Morse_Graph * >::const_iterator cmg_iter =
    conley_morse_graphs . begin () ; cmg_iter != conley_morse_graphs . end () ; ++ cmg_iter ) {

    // for every edge in the Conley-Morse graph
    typename Conley_Morse_Graph::EdgeIteratorPair edges = ( * cmg_iter ) -> Edges ();
    for ( typename Conley_Morse_Graph::EdgeIterator edge_iter = edges . first ;
      edge_iter != edges . second ; ++ edge_iter ) {

      // determine the source set and the target set
      Set_Of_Vertices & source_set = 
        final_sets [ std::make_pair ( * cmg_iter , ( * cmg_iter ) -> Source ( * edge_iter ) ) ];
      Set_Of_Vertices & target_set = 
        final_sets [ std::make_pair ( * cmg_iter , ( * cmg_iter ) -> Target ( * edge_iter ) ) ];

      // for every element of the final set for the source vertex
      for ( typename Set_Of_Vertices::iterator source_iter = source_set . begin () ;
        source_iter != source_set . end () ; ++ source_iter ) {

        // for every element of the final set for the target vertex
        for ( typename Set_Of_Vertices::iterator target_iter = target_set . begin () ;
          target_iter != target_set . end () ; ++ target_iter ) {

          // add the corresponding edge in the final Conley-Morse graph
          conley_morse_graph -> AddEdge ( * source_iter , * target_iter );
        }
      }
    }
  }
  return;
} /* Determine_All_Connections */

// --------------------------------------------------

/// Tries to rule out connections between Morse sets, based on iterations
/// of the map that acts on geometric descriptions (an interval map,
/// as opposed to using the combinatorial map on the toplex).
/// Removes those edges from the graph for which it can prove that
/// all the iterations of one set (up to the given path length bound)
/// are disjoint from the other set.
/// TODO: Implement some optimization and smart reasoning.
/// In the current version, simply ALL the connections are verified.
/// In the future, one can use the transitivity to rule out
/// some connections or to determine which connections to check first.
/// Also it may be wise to check the shortest connecting orbits first
/// in order to gather some data quickly and then use it to speed up
/// the remaining computations.
/// This procedure should also make use of the connecting orbit bounds.
template < class Conley_Morse_Graph , class Map , class Toplex >
void Rule_Out_Connections ( Conley_Morse_Graph * conley_morse_graph ,
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > & connecting_orbits ,
  std::map < typename Conley_Morse_Graph::Edge , size_t > & path_bounds ,
  const Map & interval_map , const Toplex & phase_space ) {

  // prepare shortcuts for data types used in this function
  typedef std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > Connecting_Orbits;
  typedef std::map < typename Conley_Morse_Graph::Edge , size_t > Path_Bounds;
  typedef typename Conley_Morse_Graph::Edge Edge;
  typedef typename Conley_Morse_Graph::Vertex Vertex;
  typedef typename Toplex::Subset Toplex_Subset;

  // prepare an array of edges to remove
  typedef std::vector < std::pair < Vertex , Vertex > > Vertex_Pairs;
  Vertex_Pairs edges_to_remove;

  // go through all the edges for which path bounds are defined
  for ( typename Path_Bounds::iterator path_iter = path_bounds . begin () ;
    path_iter != path_bounds . end () ; ++ path_iter ) {

    // retrieve the edge, the path length bound, and the corresponding Morse sets
    const Edge & edge = path_iter -> first ;
    size_t bound = path_iter -> second ;
    Vertex source = conley_morse_graph -> Source ( edge );
    Vertex target = conley_morse_graph -> Target ( edge );
    const Toplex_Subset & source_set ( conley_morse_graph -> CubeSet ( source ) );
    const Toplex_Subset & target_set ( conley_morse_graph -> CubeSet ( target ) );

    // for all the top cells in the first set,
    // check if their iterations intersect the other set
    bool disjoint = true;
    for ( typename Toplex_Subset::const_iterator cell_iter = source_set . begin () ;
      disjoint && ( cell_iter != source_set . end () ) ; ++ cell_iter ) {
      typename Toplex::Geometric_Description box = phase_space . geometry ( * cell_iter );
      for ( size_t n = 0 ; disjoint && ( n < bound ) ; ++ n ) {
        box = interval_map ( box );
        Toplex_Subset cover = phase_space . cover ( box );
        // --- one version: ---
        Toplex_Subset intersection = intersect ( cover , target_set );
        if ( intersection . empty () )
          continue;
        // --- an alternative: ---
        //if ( ! Check_if_Toplex_Subsets_Intersect ( cover , target_set ) )
        //  continue;
        disjoint = false;
      }
    }

    // mark this edge for removal if the itrations are disjoint from the other set
    if ( disjoint )
      edges_to_remove . push_back ( std::make_pair ( source , target ) );
  }

  // remove the edges that have to be removed now
  for ( typename Vertex_Pairs::const_iterator edge_iter = edges_to_remove . begin ();
    edge_iter != edges_to_remove . end () ; ++ edge_iter ) {
    conley_morse_graph -> RemoveEdge ( edge_iter -> first , edge_iter -> second );
  }

  return;
} /* Rule_Out_Connections */

// --------------------------------------------------

template < class Toplex >
void draw_ascii_subset ( const Toplex & my_toplex, const typename Toplex::Subset & my_subset ) {
  int Width = 128;
  int Height = 128;
  int i_max = 0;
  int j_max = 0;
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  std::vector < std::vector < bool > > data ( Height + 1 );
  for ( int j = 0; j <= Height; ++ j ) data [ j ] . resize ( Width + 1, false );
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
       it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    int i_left = ( box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (float) Width;
    int j_left = ( box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (float) Height;
    int i_right = ( box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (float) Width;
    int j_right = ( box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (float) Height;
    
    i_max = std::max ( i_right, i_max );
    j_max = std::max ( j_right, j_max );
    
    for ( int i = i_left; i < i_right; ++ i )
      for ( int j = j_left; j < j_right; ++ j ) 
        data [ j ] [ i ] = true;
  }
  /*
   for ( int j = Height; j >= 0; -- j ) {
   for ( int i = 0; i <= Width; ++ i ) {
   if ( data [ j ] [ i ] ) std::cout << "#";
   else std::cout << ".";
   }
   std::cout << "\n";
   } 
   */
  j_max += 5;
  i_max += 5;
  data . clear ();
  data . resize ( Height + 1);
  for ( int j = 0; j <= Height; ++ j ) data [ j ] . resize ( Width + 1, false );
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
       it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    int i_left = ( box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (float) Width
    * ((float) Width / (float) i_max );
    int j_left = ( box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (float) Height
    * ( (float) Height / (float) j_max );
    int i_right = ( box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (float) Width
    * ((float) Width / (float) i_max );
    int j_right = ( box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (float) Height
    * ( (float) Height / (float) j_max );
    
    for ( int i = i_left; i < i_right; ++ i )
      for ( int j = j_left; j < j_right; ++ j ) { 
        if ( j > Height || i > Width ) {
          std::cout << " i = " << i << " and j = " << j << "\n";
        } else {
          data [ j ] [ i ] = true;
        }
      }
  }
  for ( int j = Height; j >= 0; -- j ) {
    for ( int i = 0; i <= Width; ++ i ) {
      if ( data [ j ] [ i ] ) std::cout << "#";
      else std::cout << ".";
    } /* for */
    std::cout << "\n";
  } /* for */
} /* draw_ascii_subset */

template < class Conley_Morse_Graph, class Toplex , class Parameter_Toplex , class Map ,
  class Decide_Subdiv , class Decide_Conley_Index , class Cached_Box_Information >
void Compute_Conley_Morse_Graph ( Conley_Morse_Graph * conley_morse_graph ,
  Toplex * phase_space ,
  const typename Parameter_Toplex::Geometric_Description & parameter_box ,
  const Decide_Subdiv & decide_subdiv ,
  const Decide_Conley_Index & decide_conley_index ,
  Cached_Box_Information * cached_box_information ) {

  // short names for the types used in this function
  typedef std::vector < Conley_Morse_Graph * > Conley_Morse_Graphs;

  // create the objects of the maps
  Map interval_map ( parameter_box );
  typedef DirectedGraph < Toplex > Combinatorial_Map;
  Combinatorial_Map combinatorial_map = compute_directed_graph ( * phase_space , interval_map );

  // prepare maps for pointing the final Morse sets to where they came from
  std::map < typename Conley_Morse_Graph::Vertex , Conley_Morse_Graph const * > original_cmg;
  std::map < typename Conley_Morse_Graph::Vertex , typename Conley_Morse_Graph::Vertex > original_set;

  // prepare a map for pointing Morse sets to their subdivisions
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , Conley_Morse_Graph const * > finer_cmg;

  // prepare a map for pointing Morse sets to the corresponding vertices in the final CMG
  std::map < std::pair < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > , typename Conley_Morse_Graph::Vertex > final_set;

  // prepare maps for pointing a Morse decomposition to the Morse set
  // in the coarser Morse decomposition
  std::map < Conley_Morse_Graph const * , Conley_Morse_Graph const * > coarser_cmg;
  std::map < Conley_Morse_Graph const * , typename Conley_Morse_Graph::Vertex > coarser_set;

  // prepare maps for pointing CMGs to the connecting orbits computed for them
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > > all_connecting_orbits;

  // prepare maps for pointing CMGs to the exit/entrance subsets or Morse sets
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > > exit_subsets;
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , typename Toplex::Subset > > entrance_subsets;

  // prepare maps for pointing CMGs to the path length bounds computed for them
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , size_t > > exit_path_bounds;
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Vertex , size_t > > entrance_path_bounds;
  std::map < Conley_Morse_Graph const * , std::map < typename Conley_Morse_Graph::Edge , size_t > > path_bounds;
  std::map < Conley_Morse_Graph const * , size_t > through_path_bound;

  // create an array to store all the computed Morse decompositions
  Conley_Morse_Graphs conley_morse_graphs;

  // create the initial (trivial) Morse decomposition of the entire phase space
  conley_morse_graphs . push_back ( new Conley_Morse_Graph );
  conley_morse_graphs [ 0 ] -> AddVertex ();
  typename Conley_Morse_Graph::Vertex initial_vertex =
    * ( conley_morse_graphs [ 0 ] -> Vertices () . first );
  typename Toplex::Subset & initial_set ( conley_morse_graphs [ 0 ] -> CubeSet ( initial_vertex ) );
  initial_set = phase_space -> cover ( phase_space -> bounds () );

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
    // determine the first and one-past-the-last CMG to try subdividing
    size_t cmg_begin = subdiv ? cmg_subdiv_end [ subdiv - 1 ] : 0;
    size_t cmg_end = conley_morse_graphs . size ();
    std::cout << "(cmg_begin, cmg_end ) = " << cmg_begin << ", " << cmg_end << "\n";
    if (cmg_begin == cmg_end)
      break;
    cmg_subdiv_begin . push_back ( cmg_begin );
    cmg_subdiv_end . push_back ( cmg_end );

    // go through all the CMGs and consider subdividing their Morse sets
    for ( size_t cmg_number = cmg_begin ; cmg_number != cmg_end ; ++ cmg_number ) {
      std::cout << "  cmg_number = " << cmg_number << "\n";
      // extract the Conley-Morse graph whose Morse sets are to be subdivided
      Conley_Morse_Graph * current_cmg = conley_morse_graphs [ cmg_number ];

      // process all the individual Morse sets in this Morse decomposition
      typename Conley_Morse_Graph::VertexIteratorPair vertices = current_cmg -> Vertices ();
      for ( typename Conley_Morse_Graph::VertexIterator morse_set_iterator = vertices . first ;
        morse_set_iterator != vertices . second ; ++ morse_set_iterator )
      {
        std::cout << "    processing a morse_set.\n";
        // determine the Morse set
        typename Conley_Morse_Graph::Vertex current_vertex = * morse_set_iterator;
        typename Toplex::Subset & current_set ( current_cmg -> CubeSet ( current_vertex ) );

        // DEBUG Let's draw a picture.
        draw_ascii_subset ( *phase_space, current_set );
        // if the Morse set should not be subdivided anymore
        if ( ! decide_subdiv ( subdiv , current_set ) ) {
          std::cout << "      The Morse set will not be subdivided further.\n";
          // add the Morse set to the final decomposition and set up the right links
          typename Conley_Morse_Graph::Vertex new_vertex = conley_morse_graph -> AddVertex ();
          original_cmg [ new_vertex ] = current_cmg;
          original_set [ new_vertex ] = current_vertex;

          // move or compute its Conley index
          std::cout << "      Computing Conley Index.\n";
          Conley_Index_t & conley_index ( current_cmg -> ConleyIndex ( current_vertex ) );
          if ( conley_index . undefined () && decide_conley_index . compute_final ( current_set ) ) {
            Conley_Index ( & conley_index , * phase_space , current_set , interval_map );
          }
          std::swap ( conley_morse_graph -> ConleyIndex ( new_vertex ) ,
            current_cmg -> ConleyIndex ( current_vertex ) );

          // move the cubical representation of the Morse set
          std::swap ( conley_morse_graph -> CubeSet ( new_vertex ) ,
            current_cmg -> CubeSet ( current_vertex ) );

          // mark the connection between the original Morse set and the final one
          final_set [ std::make_pair ( current_cmg , current_vertex ) ] = new_vertex;
          continue;
        } /* if */

        // subdivide the current Morse set
        // TODO: Consider subdividing parts of the Morse set if necessary.
        // In the future, one might want to consider the option of subdividing
        // a subset of the Morse set only, e.g., to subdivide the top cells
        // located along the boundary while leaving the interior intact.
        typename Toplex::Subset subdivided_set = phase_space -> subdivide ( current_set );
        std::swap ( current_set , subdivided_set );

        // compute the combinatorial map on this Morse set
        std::cout << "    Computing Combinatorial Map\n";
        combinatorial_map = compute_directed_graph ( current_set , * phase_space , interval_map );

        // compute the Morse decomposition of this Morse set
        std::cout << "    Computing Morse Decomposition\n";
        Conley_Morse_Graph * new_cmg = new Conley_Morse_Graph;
        conley_morse_graphs . push_back ( new_cmg );
        Compute_Morse_Decomposition < Toplex , Conley_Morse_Graph , Combinatorial_Map > ( new_cmg ,
          & ( exit_subsets [ new_cmg ] ), & ( exit_path_bounds [ new_cmg ] ) ,
          & ( entrance_subsets [ new_cmg ] ) , & ( entrance_path_bounds [ new_cmg ] ) ,
          & ( all_connecting_orbits [ new_cmg ] ) , & ( path_bounds [ new_cmg ] ) ,
          & ( through_path_bound [ new_cmg ] ) , * phase_space , current_set ,
          exit_subsets [ current_cmg ] [ current_vertex ] ,
          entrance_subsets [ current_cmg ] [ current_vertex ] , combinatorial_map );

        // compute the Conley indices of the constructed Morse sets
        std::cout << "    Computing Conley Indices\n";
        typename Conley_Morse_Graph::VertexIteratorPair new_vertices = new_cmg -> Vertices ();
        for ( typename Conley_Morse_Graph::VertexIterator new_set_iterator = new_vertices . first ;
          new_set_iterator != new_vertices . second ; ++ new_set_iterator ) {

          // determine the Morse set
          typename Conley_Morse_Graph::Vertex new_vertex = * new_set_iterator;
          typename Toplex::Subset & new_set ( new_cmg -> CubeSet ( new_vertex ) );

          // if there is exactly one set in the Morse decomposition then it inherits the Conley index
          if ( ( new_cmg -> NumVertices () == 1 ) &&
            ! ( current_cmg -> ConleyIndex ( current_vertex ) ) . undefined () ) {

            std::swap ( current_cmg -> ConleyIndex ( current_vertex ) ,
              new_cmg -> ConleyIndex ( new_vertex ) );
            continue;
          } /* if */

          // compute the Conley index if requested to 
          if ( decide_conley_index . compute_after_subdivision ( subdiv , new_set ) ) {
            Conley_Index_t & conley_index ( new_cmg -> ConleyIndex ( new_vertex ) );
            Conley_Index ( & conley_index , * phase_space , new_set , interval_map );
          }
        } /* for */

        // mark the links between the coarser and finer decompositions
        finer_cmg [ std::make_pair ( current_cmg , current_vertex ) ] = new_cmg;
        coarser_cmg [ new_cmg ] = current_cmg;
        coarser_set [ new_cmg ] = current_vertex;

        // forget the Conley index and cubical representation of the subdivided set
        Conley_Index_t undefined_index;
        std::swap ( undefined_index , current_cmg -> ConleyIndex ( current_vertex ) );
        typename Toplex::Subset empty_set;
        std::swap ( empty_set , current_cmg -> CubeSet ( current_vertex ) );
      } /* for ( each Morse Set ) */
    } /* for ( each CMG ) */
  } /* for ( each subdivision level) */

  // determine connections between the Morse sets in the final graph,
  // based on the coarser Morse sets on the way
  std::map < typename Conley_Morse_Graph::Edge , typename Toplex::Subset > connecting_orbits;
  Determine_All_Connections < Toplex , Conley_Morse_Graph , Combinatorial_Map > (
    conley_morse_graph , & connecting_orbits , conley_morse_graphs , all_connecting_orbits ,
    exit_subsets , entrance_subsets ,
    original_cmg , original_set , finer_cmg , final_set , coarser_cmg , coarser_set );

  // compute the upper bounds for connection lengths between the Morse sets
  std::map < typename Conley_Morse_Graph::Edge , size_t > final_path_bounds;
  Compute_Path_Bounds < Conley_Morse_Graph > ( & final_path_bounds , * conley_morse_graph ,
    original_cmg , original_set , finer_cmg , final_set , coarser_cmg , coarser_set ,
    exit_path_bounds , entrance_path_bounds , path_bounds , through_path_bound );

  // determine which connections between Morse sets are suprious
  // and remove them from the final Conley-Morse graph
  Rule_Out_Connections < Conley_Morse_Graph , Map , Toplex > (
    conley_morse_graph , connecting_orbits ,
    final_path_bounds , interval_map , * phase_space );

  // free up the dynamically allocated memory
  // for the intermediate Morse decompositions
  for ( typename Conley_Morse_Graphs::iterator cmg_iterator = conley_morse_graphs . begin ();
    cmg_iterator != conley_morse_graphs . end (); ++ cmg_iterator ) {

    delete * cmg_iterator;
  }

  return;
} /* Compute_Conley_Morse_Graph */


#endif
