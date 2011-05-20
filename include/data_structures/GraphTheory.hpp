// GraphTheory.hpp Shaun Harker 5/16/2011

#include <vector>
#include <stack>
#include <algorithm>
#include "boost/foreach.hpp"

/* Combinatorial Map */
template < class Toplex, class CellContainer >
CombinatorialMap<Toplex,CellContainer>::CombinatorialMap ( const size_type N ) : sentinel_ ( N ) {
  index_ . resize ( N, sentinel_ );
}

template < class Toplex, class CellContainer >
typename CombinatorialMap<Toplex,CellContainer>::size_type 
CombinatorialMap<Toplex,CellContainer>::insert ( const Vertex & v ) {
  index_ [ v ] = lookup_ . size ();
  lookup_ . push_back ( v );
  data_ . push_back ( std::vector < size_type > () );
  return index_ [ v ];
}

template < class Toplex, class CellContainer >
std::vector<typename CombinatorialMap<Toplex,CellContainer>::size_type> & 
CombinatorialMap<Toplex,CellContainer>::adjacencies ( const size_type & v ) {
  return data_ [ v ];
}

template < class Toplex, class CellContainer >
const std::vector<typename CombinatorialMap<Toplex,CellContainer>::size_type> & 
CombinatorialMap<Toplex,CellContainer>::adjacencies ( const size_type & v ) const {
  return data_ [ v ];
}

template < class Toplex, class CellContainer >
typename CombinatorialMap<Toplex,CellContainer>::size_type 
CombinatorialMap<Toplex,CellContainer>::sentinel ( void ) const {
  return sentinel_;
}

template < class Toplex, class CellContainer >
typename CombinatorialMap<Toplex,CellContainer>::size_type 
CombinatorialMap<Toplex,CellContainer>::index ( const Vertex & input ) const {
  return index_ [ input ];
}

template < class Toplex, class CellContainer >
typename CombinatorialMap<Toplex,CellContainer>::Vertex 
CombinatorialMap<Toplex,CellContainer>::lookup ( const size_type & input ) const {
  return lookup_ [ input ];
}

template < class Toplex, class CellContainer >
void CombinatorialMap<Toplex,CellContainer>::index ( std::vector < size_type > * output, 
                                       const CellContainer & input ) const {
  output -> clear ();
  BOOST_FOREACH ( Vertex cell, input ) {
    size_type cell_index = index_ [ cell ];
    if ( cell_index != sentinel_ ) output -> push_back ( cell_index );
  }
}

template < class Toplex, class CellContainer >
void CombinatorialMap<Toplex,CellContainer>::lookup ( CellContainer * output, 
                                        const std::vector < size_type > & input ) const {
  output -> clear ();
  std::insert_iterator < CellContainer > ii ( *output, output -> begin () );
  BOOST_FOREACH ( size_type cell_index, input ) {
    * ii ++ = lookup_ [ cell_index ];
  }
}
template < class Toplex, class CellContainer >
typename CombinatorialMap<Toplex,CellContainer>::size_type 
CombinatorialMap<Toplex,CellContainer>::num_vertices ( void ) const {
  return lookup_ . size ();
}

/* compute_combinatorial_map */
template < class Toplex, class Map, class CellContainer >
CombinatorialMap < Toplex, CellContainer >
compute_combinatorial_map (const Toplex & my_toplex, 
                           const Map & f) {
  typedef typename Toplex::size_type size_type;
  CombinatorialMap<Toplex, CellContainer> graph ( my_toplex . tree_size () );
  
  /* First, we develop an indexing system for the top cells in "sets" */
  // BoOST_FOREACH ( typename Toplex::Top_Cell cell, my_toplex ) {

  for ( typename Toplex::const_iterator it = my_toplex . begin (); it != my_toplex . end ();
       ++ it ) {
    graph . insert ( *it );
  }
  /* Next, we compute the graph and store it via the indexing scheme. */
  // BoOST_FOREACH ( typename Toplex::Top_Cell domain_cell, my_toplex ) {
  for ( typename Toplex::const_iterator it = my_toplex . begin (); it != my_toplex . end ();
         ++ it ) {
    typename Toplex::Top_Cell domain_cell = *it;
    size_type source = graph . index ( domain_cell );
    CellContainer image;
    std::insert_iterator < CellContainer > ii ( image, image . begin () );
    my_toplex . cover ( ii, f ( my_toplex . geometry ( domain_cell ) ) );
    graph . index ( & graph . adjacencies ( source ), image );
  } /* boost_foreach */
  return graph;
} /* compute_combinatorial_map */


template < class Toplex, class Map, class CellContainer >
CombinatorialMap < Toplex, CellContainer >
compute_combinatorial_map (const std::vector < CellContainer > & sets,
                           const Toplex & my_toplex, 
                           const Map & f) {
  typedef typename Toplex::size_type size_type;
  CombinatorialMap<Toplex,CellContainer> graph ( my_toplex . tree_size () );

  /* First, we develop an indexing system for the top cells in "sets" */
  BOOST_FOREACH ( const CellContainer & my_subset, sets ) {
    BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
      graph . insert ( cell );
    }
  }
  /* Next, we compute the graph and store it via the indexing scheme. */
  BOOST_FOREACH ( const CellContainer & my_subset, sets ) {
    BOOST_FOREACH ( typename Toplex::Top_Cell domain_cell, my_subset ) {
      size_type source = graph . index ( domain_cell );    
      CellContainer image;
      std::insert_iterator < CellContainer > ii ( image, image . begin () );
      my_toplex . cover ( ii, f ( my_toplex . geometry ( domain_cell ) ) );
      graph . index ( & graph . adjacencies ( source ), image );
    } /* boost_foreach */
  } /* boost_foreach */
  return graph;
} /* compute_combinatorial_map */

/* compute_morse_sets */
template < class MorseGraph, class Toplex, class CellContainer >
void compute_morse_sets (std::vector< CellContainer > * output, 
                         const CombinatorialMap<Toplex,CellContainer> & G, 
   /* optional output */ MorseGraph * MG ) {
  typedef typename CombinatorialMap<Toplex,CellContainer>::size_type size_type;
  std::vector<std::vector< size_type > > untranslated;
  if ( MG == NULL ) {
    // The user doesn't want a Morse Graph.
    compute_strong_components ( &untranslated, G );
    BOOST_FOREACH ( std::vector < size_type > & translate_me, untranslated ) {      
      output -> push_back ( CellContainer () );
      G . lookup ( & (output -> back ()), translate_me );
    }
  } else {
    // The user wants us to determine reachability and provide a Morse Graph.
    /* Produce Strong Components and Topological Sort */
    std::vector< size_type > topological_sort;
    compute_strong_components ( &untranslated, G, &topological_sort );
    BOOST_FOREACH ( std::vector < size_type > & translate_me, untranslated ) {
      output -> push_back ( CellContainer () );
      G . lookup ( & (output -> back ()), translate_me );
    }
    /* Produce Reachability Information */
    std::vector < std::vector < unsigned int > > reach_info;
    compute_reachability ( & reach_info, untranslated, G, topological_sort );
    
    /* Produce Morse Graph Vertices */
    typedef typename MorseGraph::Vertex cmg_vertex_t;
    std::map < unsigned int, cmg_vertex_t > translate_to_cmg;
    for (unsigned int s = 0; s < output -> size (); ++ s ) {
      cmg_vertex_t new_vertex = MG -> AddVertex ();
      translate_to_cmg [ s ] = new_vertex;
      MG -> CubeSet ( new_vertex ) = (*output) [ s ];
    }
    
    /* Produce Morse Graph Edges */
    for (unsigned int s = 0; s < reach_info . size (); ++ s ) {
      BOOST_FOREACH ( unsigned int t, reach_info [ s ] ) {
        MG -> AddEdge ( translate_to_cmg [ s ], translate_to_cmg [ t ] );
      }
    }
    
  } /* if-else */
  
}

/* compute_strong_components */
template < class OutEdgeGraph >
void compute_strong_components (std::vector<std::vector<typename OutEdgeGraph::size_type> > * output, 
                                const OutEdgeGraph & G, 
          /* optional output */ std::vector<typename OutEdgeGraph::size_type> * topological_sort ) {
  // typedefs and const variables
  typedef typename OutEdgeGraph::size_type size_type;
  typedef std::pair<size_type, size_type> Edge;
  const size_type sentinel = G . sentinel ();
  // Things that could be put in external memory
  std::stack<Edge> dfs_stack;
  std::stack<Edge> lowlink_stack;
  std::stack<size_type> component; 
  // internal memory objects
  size_type N = G . num_vertices ();
  std::vector<size_type> index ( N, sentinel ); // An index of "sentinel" means has not been explored.
  std::vector<bool> committed ( N, 0 );
  size_type current_index = 0;
  lowlink_stack . push ( std::make_pair ( sentinel, sentinel ) ); // so there is always a top.
  // Main Loop
  //std::cout << "SCC. N = " << N << ", sentinel = " << sentinel << "\n";
  //std::cout << "sentinel = " << sentinel << "\n";
  for ( size_type root = 0; root < N; ++ root ) {
    if ( index [ root ] != sentinel ) continue;
    dfs_stack . push ( Edge ( sentinel, root ) );
    while ( not dfs_stack . empty () ) {
      Edge S = dfs_stack . top ();
      //std::cout << "Top = (" << dfs_stack . top () . first << ", " << dfs_stack . top () . second << ")\n";
      size_type & v = S . second;
      if ( index [ v ] == sentinel ) {
        // FIRST VISIT
        //std::cout << "First visit to vertex " << v << ".\n";
        index [ v ] = current_index;
        //std::cout << "index [ " << v << " ] := " << index[v] << "\n";
        size_type lowlink = current_index;
        component . push ( v );
        ++ current_index;
        // Learn adjacencies
        const std::vector<size_type> & children = G . adjacencies ( v );
        // Work through children
        BOOST_FOREACH ( size_type w, children ) {
          //std::cout << "Inspecting edge (" << v << ", " << w << ")\n";
          if ( w == v ) {
            // Self-edge. For Path-SCC, we will want to know about this.
            // We store a special lowlink code, which will be read last
             lowlink_stack . push ( Edge ( v, sentinel ) );
          }
          if ( index [ w ] != sentinel ) {
            // Already been explored. If it isn't already committed to an SCC, 
            // we get its index.
            if ( not committed [ w ] ) lowlink = std::min ( lowlink, index [ w ] );
            continue;
          }
          // Push the child onto the DFS stack
          dfs_stack . push ( Edge ( v, w ) );
        }
        lowlink_stack . push ( Edge ( v, lowlink ) );

      } else {
        if ( lowlink_stack . top () . first != v ) {
          // THIRD VISIT OR LATER
          //std::cout << "Later visit to vertex " << v << ".\n";
          dfs_stack . pop ();
          continue;
        }
        // SECOND VISIT.
        //std::cout << "Second visit to vertex " << v << ".\n";
        bool self_connected = false;
        // Read the lowlinks of the children off from the stack
        size_type lowlink = index [ v ];
        while ( lowlink_stack . top () . first == v ) {
          //std::cout << "Popping (" << lowlink_stack . top () . first << ", " << lowlink_stack . top () . second << ") from lowlink stack\n";
          if ( lowlink_stack . top () . second == sentinel ) self_connected = true;
          lowlink = std::min ( lowlink, lowlink_stack . top () . second );
          lowlink_stack . pop ();
        }
        //std::cout << "(Index, Lowlink) = " << index [ v ] << ", " << lowlink << "\n";
        // Now push the lowlink onto the stack for parent
        size_type & w = S . first; // Parent in DFS tree.
        if ( w != sentinel ) lowlink_stack . push ( std::make_pair ( w, lowlink ) );
        
        // Check if we have found an SCC
        if ( lowlink == index [ v ] ) {
          // Pop the vertices from component until we get to v, 
          // mark them as committed, and send them to the output.
          std::vector < size_type > SCC;
          while ( component . top () != v ) {
            //std::cout << "Popping vertex " << component . top () << " (index = " << index[component.top()] << ") from component stack\n";
            SCC . push_back ( component . top () );
            component . pop ();
          }
          //std::cout << "Final: Popping vertex " << component . top () << " (index = " << index[component.top()] << ") from component stack\n";
          SCC . push_back ( component . top () );
          component . pop (); // DRY style mistake, fix?
          // Mark the vertices in the component as committed to an SCC
          BOOST_FOREACH ( size_type u, SCC ) {
            committed [ u ] = true;
          }
          /* Send the SCC to output if contains at least one edge */
          if ( SCC . size () > 1 || self_connected ) {
            //output -> push_back (SCC);
            output -> push_back ( std::vector < size_type > () );
            std::swap ( output -> back (), SCC );
          }

        }
        // If user wants topological sort, give it to them.
        if ( topological_sort != NULL ) topological_sort -> push_back ( dfs_stack . top () . second );
        // Pop the vertex from the dfs stack        
        dfs_stack . pop ();
      } // if visited
    } // while dfs stack non-empty
  } // while not all nodes explored
  
}

#if 0
// working copy
/* compute_strong_components */
template < class OutEdgeGraph >
void compute_strong_components (std::vector<std::vector<typename OutEdgeGraph::size_type> > * output, 
                                const OutEdgeGraph & G, 
                                /* optional output */ std::vector<typename OutEdgeGraph::size_type> * topological_sort ) {
  // typedefs and const variables
  typedef typename OutEdgeGraph::size_type size_type;
  typedef std::pair<size_type, size_type> Edge;
  const size_type sentinel = G . sentinel ();
  // Things that could be put in external memory
  std::stack<Edge> dfs_stack;
  std::stack<Edge> lowlink_stack;
  std::stack<size_type> component; 
  // internal memory objects
  size_type N = G . num_vertices ();
  std::vector<size_type> index ( N, sentinel ); // An index of "sentinel" means has not been explored.
  std::vector<bool> committed ( N, 0 );
  size_type current_index = 0;
  lowlink_stack . push ( std::make_pair ( sentinel, sentinel ) ); // so there is always a top.
  // Main Loop
  //std::cout << "SCC. N = " << N << ", sentinel = " << sentinel << "\n";
  //std::cout << "sentinel = " << sentinel << "\n";
  for ( size_type root = 0; root < N; ++ root ) {
    if ( index [ root ] != sentinel ) continue;
    dfs_stack . push ( Edge ( sentinel, root ) );
    while ( not dfs_stack . empty () ) {
      Edge S = dfs_stack . top ();
      //std::cout << "Top = (" << dfs_stack . top () . first << ", " << dfs_stack . top () . second << ")\n";
      size_type & v = S . second;
      if ( index [ v ] == sentinel ) {
        // FIRST VISIT
        //std::cout << "First visit to vertex " << v << ".\n";
        index [ v ] = current_index;
        //std::cout << "index [ " << v << " ] := " << index[v] << "\n";
        //size_type lowlink = current_index;
        component . push ( v );
        ++ current_index;
        // Learn adjacencies
        const std::vector<size_type> & children = G . adjacencies ( v );
        // Work through children
        BOOST_FOREACH ( size_type w, children ) {
          //std::cout << "Inspecting edge (" << v << ", " << w << ")\n";
          //if ( w == v ) {
          // Self-edge. For Path-SCC, we will want to know about this.
          // We store a special lowlink code, which will be read last
          // lowlink_stack . push ( Edge ( v, sentinel ) );
          //}
          if ( index [ w ] != sentinel ) {
            // Already been explored. If it isn't already committed to an SCC, 
            // we get its index.
            //if ( not committed [ w ] ) lowlink = std::min ( lowlink, index [ w ] );
            continue;
          }
          // Push the child onto the DFS stack
          dfs_stack . push ( Edge ( v, w ) );
        }
        //lowlink_stack . push ( Edge ( v, lowlink ) );
        lowlink_stack . push ( Edge ( v, index [ v ] ) );
        
      } else {
        if ( lowlink_stack . top () . first != v ) {
          // THIRD VISIT OR LATER
          //std::cout << "Later visit to vertex " << v << ".\n";
          dfs_stack . pop ();
          continue;
        }
        // SECOND VISIT.
        // Learn adjacencies
        size_type lowlink = index [ v ];
        const std::vector<size_type> & children = G . adjacencies ( v );
        // Work through children
        BOOST_FOREACH ( size_type child, children ) {
          //std::cout << "Inspecting edge (" << v << ", " << w << ")\n";
          if ( child == v ) {
            // Self-edge. For Path-SCC, we will want to know about this.
            // We store a special lowlink code, which will be read last
            lowlink_stack . push ( Edge ( v, sentinel ) );
          }
          if ( index [ child ] != sentinel ) {
            // Already been explored. If it isn't already committed to an SCC, 
            // we get its index.
            if ( not committed [ child ] ) lowlink = std::min ( lowlink, index [ child ] );
            continue;
          }
          // Push the child onto the DFS stack
          //dfs_stack . push ( Edge ( v, w ) );
        }
        
        //std::cout << "Second visit to vertex " << v << ".\n";
        bool self_connected = false;
        // Read the lowlinks of the children off from the stack
        //size_type lowlink = index [ v ];
        while ( lowlink_stack . top () . first == v ) {
          //std::cout << "Popping (" << lowlink_stack . top () . first << ", " << lowlink_stack . top () . second << ") from lowlink stack\n";
          if ( lowlink_stack . top () . second == sentinel ) self_connected = true;
          lowlink = std::min ( lowlink, lowlink_stack . top () . second );
          lowlink_stack . pop ();
        }
        //std::cout << "(Index, Lowlink) = " << index [ v ] << ", " << lowlink << "\n";
        // Now push the lowlink onto the stack for parent
        size_type & w = S . first; // Parent in DFS tree.
        if ( w != sentinel ) lowlink_stack . push ( std::make_pair ( w, lowlink ) );
        
        // Check if we have found an SCC
        if ( lowlink == index [ v ] ) {
          // Pop the vertices from component until we get to v, 
          // mark them as committed, and send them to the output.
          std::vector < size_type > SCC;
          while ( component . top () != v ) {
            //std::cout << "Popping vertex " << component . top () << " (index = " << index[component.top()] << ") from component stack\n";
            SCC . push_back ( component . top () );
            component . pop ();
          }
          //std::cout << "Final: Popping vertex " << component . top () << " (index = " << index[component.top()] << ") from component stack\n";
          SCC . push_back ( component . top () );
          component . pop (); // DRY style mistake, fix?
          // Mark the vertices in the component as committed to an SCC
          BOOST_FOREACH ( size_type u, SCC ) {
            committed [ u ] = true;
          }
          /* Send the SCC to output if contains at least one edge */
          if ( SCC . size () > 1 || self_connected ) {
            //output -> push_back (SCC);
            output -> push_back ( std::vector < size_type > () );
            std::swap ( output -> back (), SCC );
          }
          
        }
        // If user wants topological sort, give it to them.
        if ( topological_sort != NULL ) topological_sort -> push_back ( dfs_stack . top () . second );
        // Pop the vertex from the dfs stack        
        dfs_stack . pop ();
      } // if visited
    } // while dfs stack non-empty
  } // while not all nodes explored
  
}

#endif

/* compute_reachability */
template < class Graph >
void compute_reachability ( std::vector < std::vector < unsigned int > > * output, 
                           std::vector<std::vector<typename Graph::size_type> > & morse_sets, 
                           const Graph & G, 
                           const std::vector<typename Graph::size_type> & topological_sort ) {
  typedef typename Graph::size_type size_type;
  //size_type sentinel = G . sentinel ();
  /* Count the Morse Sets */
  //std::cout << "REACHABILITY.\n";
  //std::cout << "Count the Morse Sets\n";
  size_type number_of_morse_sets = morse_sets . size ();
  //std::cout << "There are " << number_of_morse_sets << " of them.\n";
  if ( number_of_morse_sets == 0 ) return; // trivial case
  output -> resize ( number_of_morse_sets );
  /* Paint the Morse Sets */
  // For each morse set, go through its vertices and 
  // color them according to which morse set they are in.
  // Vertices not in a morse set are colored "number_of_morse_sets"
  //std::cout << "Paint the Morse Sets\n";

  std::vector < size_type > morse_paint ( G . num_vertices (), number_of_morse_sets );
  for ( size_type count = 0; count < number_of_morse_sets; ++ count ) {
    BOOST_FOREACH ( size_type v, morse_sets [ count ] ) {
      morse_paint [ v ] = count;
    } 
  } 
  /* Break the Morse Sets up into Computational Groups of 64 and proceed */
  //std::cout << "Group Loop\n";

  size_type groups = ( (number_of_morse_sets - 1) / 64 ) + 1;
  // We use a vector called morse_code. It's function it to maintain
  // information about which morse sets can reach a given vertex.
  // By processing in topological order, it is possible to give morse_code
  // the correct values in a single pass.
  std::vector < unsigned long > morse_code;
  // We use a vector called condensed_code in order to store the final information
  // about which morse sets can reach a given morse set. It can be inferred from morse_code
  // during the same sweep in which we construct morse_code.
  std::vector < unsigned long > condensed_code;
  // Loop through groups.
  for ( size_type group_number = 0; group_number < groups; ++ group_number ) {
    //std::cout << "Process Group " << group_number << "\n";

    size_type group_size = std::min((unsigned int) 64, number_of_morse_sets - 64 * group_number);
    size_type offset = 64 * group_number;
    //std::cout << "group_size = " << group_size << "\n";
    morse_code . clear ();
    morse_code . resize ( G . num_vertices (), 0 );
    condensed_code . clear ();
    condensed_code . resize ( number_of_morse_sets + 1, 0 );
    // Paint the codes.
    // We do an initial sweep painting the sources onto their sets. 
    for ( size_type count = 0; count < group_size; ++ count ) {
      size_type set_number = offset + count;
      unsigned long code = ((unsigned long)1) << count;
      //std::cout << "Producing code = " << code << "\n";
      BOOST_FOREACH ( size_type v, morse_sets [ set_number ] ) {
        morse_code [ v ] = code;
      } 
    } 
    // Loop through topological sort.
    // Our goal is to produce "condensed_code", which we can read the info off from.
    // The intermediate information is stored in "morse_code"
    //std::cout << "Loop through top sort \n";
    for ( size_type vi = topological_sort . size () - 1; vi != 0; -- vi ) {
      size_type v = topological_sort [ vi ];
      const std::vector < size_type > & children = G . adjacencies ( v );
      BOOST_FOREACH ( size_type w, children ) {
        morse_code [ w ] |= morse_code [ v ];
        condensed_code [ morse_paint [ w ] ] |= morse_code [ v ];
      }
    } // its okay to ignore 0
    {
    size_type v = topological_sort [ 0 ];
    const std::vector < size_type > & children = G . adjacencies ( v );
    BOOST_FOREACH ( size_type w, children ) {
      morse_code [ w ] |= morse_code [ v ];
      condensed_code [ morse_paint [ w ] ] |= morse_code [ v ];
    }
    } //dry problem
    // now condensed_code is indexed by targets, and contains the sources reaching it.
    
    //std::cout << "Loop through Morse Sets\n";
    // Loop through Morse Sets to learn reachability information
    for ( size_type count = 0; count < number_of_morse_sets; ++ count ) {
      // Read condensed code to determine which of the group reached this morse set
      /* TODO: WRITE NASTY, OPTIMIZED UNROLLED BIT MANIPULATION CODE */
      //std::cout << " condensed_code [ " << count << " ] = " << condensed_code [ count ] << "\n";

      unsigned long bit = 1;
      for ( int i = 0; i < 64; ++ i ) {
        //std::cout << "(offset, i) = " << offset << ", " << i << "\n";
        if ( condensed_code [ count ] & bit ) {
          //std::cout << "bit = " << bit << "\n";
          //std::cout << "Writing connection (" << offset + i << ", " << count << ")\n";
          (*output)[offset + i] . push_back ( count );
        } // if
        bit <<= 1;
      } // for bit index
    } // for morse set
  } // for groups
  
} /* compute_reachability */
