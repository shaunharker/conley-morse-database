// GraphTheory.hpp Shaun Harker 5/16/2011

#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <memory>

#include "boost/foreach.hpp"
#include "database/structures/MapGraph.h"

#define DEBUGPRINT if(0)

/*******************
 *   MORSE THEORY  *
 *******************/


/* computeMorseSetsAndReachability */
template < class Map >
void computeMorseSetsAndReachability (std::vector< boost::shared_ptr<Grid> > * output,
                                      std::vector<std::vector<unsigned int> > * reach,
                                      const Grid & G,
                                      const Map & f ) {
  //std::cout << "Create Graph\n";// Create Graph
  MapGraph < Map > mapgraph ( G, f );

  /* Produce Strong Components and Reachability */
  std::vector < std::vector < Grid::GridElement > > components;
  std::vector < Grid::size_type > topological_sort;
  //std::cout << "Compute Strong Components\n";
  computeStrongComponents ( &components, mapgraph, &topological_sort );
#ifdef CMG_VERBOSE
  std::cout << "Found " << components . size () << " morse sets.\n";
#endif
#ifndef NO_REACHABILITY
  //std::cout << "Compute Reachability\n";
  computeReachability ( reach, components, mapgraph, topological_sort );
#endif
  /* Create output grids */
  //std::cout << "Create Output Grids\n";
  output -> clear ();
  BOOST_FOREACH ( const std::vector<Grid::GridElement> & component, components ) {
    boost::shared_ptr < Grid > component_grid ( G . subgrid ( component ) );
    output -> push_back ( component_grid );
  }
}

#if 0

// CURRENTLY UNUSABLE. Need to update to use auto_ptr
// Need to use new grid interface.
// is shared_ptr more appropriate? confusion.
/* compute_morse_sets */
template < class MorseGraph, class Grid, class Map >
void computeMorseSets (std::vector< std::auto_ptr<Grid> > * output,
                       const Grid & G,
                       const Map & f,
                       /* optional output */ MorseGraph * MG ) {
  
  // Clear output
  output -> clear ();
  
  // Create Graph
  MapGraph < Grid, Map > mapgraph ( G, f );
  
  // Check if Morse Graph output required.
  if ( MG == NULL ) {
    // The user doesn't want a Morse Graph.
    std::vector < typename MapGraph::set > components;
    computeStrongComponents ( &components, mapgraph );
    BOOST_FOREACH ( const typename MapGraph::set & component, components ) {
      (*output) . push_back ( std::auto_ptr<Grid> (G . subgrid ( component )) );
    }
  } else {
    // The user wants us to determine reachability and provide a Morse Graph.
    /* Produce Strong Components and Topological Sort */
    std::vector< typename MapGraph::vertex > topological_sort;
    std::vector < typename MapGraph::set > components;
    computeStrongComponents ( &components, mapgraph, &topological_sort );
    BOOST_FOREACH ( const typename MapGraph::set & component, components ) {
      (*output) . push_back ( std::auto_ptr<Grid> (G . subgrid ( component )) );
    }
    
    /* Produce Reachability Information */
    std::vector < std::vector < unsigned int > > reach_info;
    computeReachability ( &reach_info, components, mapgraph, topological_sort );
    
    /* Produce Morse Graph Vertices */
    typedef typename MorseGraph::Vertex cmg_vertex_t;
    std::map < unsigned int, cmg_vertex_t > translate_to_cmg;
    for (unsigned int s = 0; s < output -> size (); ++ s ) {
      cmg_vertex_t new_vertex = MG -> AddVertex ();
      translate_to_cmg [ s ] = new_vertex;
      MG -> CellSet ( new_vertex ) = (*output) [ s ];
    }
    
    /* Produce Morse Graph Edges */
    for (unsigned int s = 0; s < reach_info . size (); ++ s ) {
      BOOST_FOREACH ( unsigned int t, reach_info [ s ] ) {
        MG -> AddEdge ( translate_to_cmg [ s ], translate_to_cmg [ t ] );
      }
    }
    
  } /* if-else */
  
}
#endif

/*******************
 *   GRAPH THEORY  *
 *******************/

/* compute_strong_components */
template < class OutEdgeGraph >
void computeStrongComponents (std::vector<std::vector<typename OutEdgeGraph::size_type> > * output,
                              const OutEdgeGraph & G,
         /* optional output */std::vector<typename OutEdgeGraph::size_type> * topological_sort ) {
  // typedefs and const variables
  long int effort = 0;
  typedef typename OutEdgeGraph::size_type size_type;
  typedef std::pair<size_type, size_type> Edge;
  const size_type sentinel = G . num_vertices ();
  // Things that could be put in external memory
  std::deque<Edge> dfs_stack;
  std::stack<Edge> lowlink_stack;
  std::stack<size_type> component; 
  // internal memory objects
  size_type N = G . num_vertices ();
  std::vector<size_type> index ( N, sentinel ); // An index of "sentinel" means has not been explored.
  std::vector<bool> committed ( N, 0 );
  size_type current_index = 0;
  lowlink_stack . push ( std::make_pair ( sentinel, sentinel ) ); // so there is always a top.
  
  // DEBUG
  int memoryuse = 0;
  
  // Main Loop
#ifdef CMG_VERBOSE
  long num_edges = 0;
  std::cout << "Computing Strongly Connected Components.\n";
  std::cout . flush ();
  size_type progress = 0;
  size_type progresspercent = 0;
#endif
  //std::cout << "sentinel = " << sentinel << "\n";
  for ( size_type root = 0; root < N; ++ root ) {
    if ( index [ root ] != sentinel ) continue;
    dfs_stack . push_back ( Edge ( sentinel, root ) );
    while ( not dfs_stack . empty () ) {
      // BEGIN MEMORY USAGE CHECK
      int mem = dfs_stack . size () + lowlink_stack . size () + component . size ();
      if ( mem / (int)N > memoryuse ) {
        memoryuse = mem / N;
        //std::cout << "memoryuse = " << memoryuse << "\n";
      }
      if ( dfs_stack . size () > 2 * N ) {
        // clean dfs stack
        // go backwards through dfs stack and 
        // eliminate duplicates, keeping only the last occurring
        // (here, duplicates means the target of the edge is the same)
        // Reasoning: If a vertex appears as the target of two edges,
        // the first appearance is definitely not the one in which
        // we follow the edge; by the time we pop that edge, the vertex
        // will have been explored. Thus, we can clean it.
        std::vector < bool > dfs_clean_helper ( N, false );
        std::deque < Edge > new_dfs_stack;
        while ( not dfs_stack . empty () ) {
          Edge copy_edge = dfs_stack . back ();
          if ( dfs_clean_helper [ copy_edge . second ] ) {
            dfs_stack . pop_back ();
            continue;
          } else {
            dfs_clean_helper [ copy_edge . second ] = true;
            new_dfs_stack . push_front ( dfs_stack . back () );
            dfs_stack . pop_back ();
          }
        }
        std::swap ( dfs_stack, new_dfs_stack );
      }
      // END MEMORY USAGE CHECK
      
      
      Edge S = dfs_stack . back ();
      ++effort;
      
      //std::cout << "Top = (" << dfs_stack. back () . first << ", " << dfs_stack. back () . second << ")\n";
      
      size_type & v = S . second;
      if ( index [ v ] == sentinel ) {
        // FIRST VISIT
#ifdef CMG_VERBOSE
        ++ progress;
        if ( (100*progress)/N > progresspercent) {
          progresspercent = (100*progress)/N;
          std::cout << "\r" << progresspercent << "%    ";
          std::cout . flush ();
        }
#endif
        
        //std::cout << "First visit to vertex " << v << ".\n";
        index [ v ] = current_index;
        //std::cout << "index [ " << v << " ] := " << index[v] << "\n";
        size_type lowlink = current_index;
        component . push ( v );
        ++ current_index;
        // Learn adjacencies
        std::vector<size_type> children = G . adjacencies ( v );
#ifdef CMG_VERBOSE
        num_edges += children . size ();
#endif
        // Work through children
        BOOST_FOREACH ( size_type w, children ) {
          ++ effort;
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
          dfs_stack . push_back ( Edge ( v, w ) );
        }
        lowlink_stack . push ( Edge ( v, lowlink ) );

      } else {
        if ( lowlink_stack . top () . first != v ) {
          // THIRD VISIT OR LATER
          //std::cout << "Later visit to vertex " << v << ".\n";
          dfs_stack . pop_back ();
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
        if ( w != sentinel ) {
          if ( lowlink_stack . top () . first != w ) {
            std::cout << "Failure in SCC algorithm theory\n";
            exit ( 1 );
          }
          if ( lowlink_stack . top () . second == sentinel ) {
            lowlink_stack . push ( std::make_pair ( w, lowlink ) ); 
          } else {
            size_type & change = lowlink_stack . top () . second;
            change = std::min( lowlink, change ); 
          }
        }
        
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
            output -> push_back ( std::vector < size_type > () );
            std::swap ( output -> back (), SCC );
          }

        }
        // If user wants topological sort, give it to them.
        if ( topological_sort != NULL ) topological_sort -> push_back ( dfs_stack. back () . second );
        // Pop the vertex from the dfs stack        
        dfs_stack . pop_back ();
      } // if visited
    } // while dfs stack non-empty
  } // while not all nodes explored
#ifdef CMG_VERBOSE
  std::cout << "\r100%    ";
  std::cout << "       V = " << N << " E = " << num_edges << "  E/V = " << (double) num_edges / (double) N << "\n";
#endif
  DEBUGPRINT std::cout << "SCC effort = " << effort << "\n";
}

/* compute_reachability */
template < class Graph >
void computeReachability ( std::vector < std::vector < unsigned int > > * output,
                           std::vector<std::vector<typename Graph::size_type> > & morse_sets, 
                           const Graph & G, 
                           const std::vector<typename Graph::size_type> & topological_sort ) {
  typedef typename Graph::size_type size_type;
  //size_type sentinel = G . sentinel ();
#ifdef CMG_VERBOSE
  std::cout << "Computing Reachability Information.\n";
  std::cout . flush ();
  size_type progress = 0;
  size_type progresspercent = 0;
#endif
  /* Count the Morse Sets */
  unsigned long effort = 0;
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
      ++ effort;
      morse_paint [ v ] = count;
    } 
  } 
  /* Break the Morse Sets up into Computational Groups of 64 and proceed */
  //std::cout << "Group Loop\n";

  size_type groups = ( (number_of_morse_sets - 1) / 64 ) + 1;
  
#ifdef CMG_VERBOSE
  size_type total_work_to_do = topological_sort . size () * groups;
#endif
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

    ++ effort;
    size_type group_size = std::min((size_type) 64, number_of_morse_sets - 64 * group_number);
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
      ++ effort;
      //std::cout << "Producing code = " << code << "\n";
      BOOST_FOREACH ( size_type v, morse_sets [ set_number ] ) {
        ++ effort;
        morse_code [ v ] = code;
      } 
    } 
    // Loop through topological sort.
    // Our goal is to produce "condensed_code", which we can read the info off from.
    // The intermediate information is stored in "morse_code"
    //std::cout << "Loop through top sort \n";
    for ( size_type vi = topological_sort . size () - 1; vi != 0; -- vi ) {
#ifdef CMG_VERBOSE
      ++ progress;
      if ( (100*progress)/total_work_to_do > progresspercent) {
        progresspercent = (100*progress)/total_work_to_do;
        std::cout << "\r" << progresspercent << "%    ";
        std::cout . flush ();
      }
#endif
      size_type v = topological_sort [ vi ];
      std::vector < size_type > children = G . adjacencies ( v ); // previously const &
      BOOST_FOREACH ( size_type w, children ) {
        ++ effort;
        morse_code [ w ] |= morse_code [ v ];
        condensed_code [ morse_paint [ w ] ] |= morse_code [ v ];
      }
    } // its okay to ignore 0
#ifdef CMG_VERBOSE
    ++ progress;
#endif
    {
    size_type v = topological_sort [ 0 ];
    std::vector < size_type > children = G . adjacencies ( v );
    BOOST_FOREACH ( size_type w, children ) {
      ++ effort;
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
      ++ effort;
      unsigned long bit = 1;
      for ( int i = 0; i < 64; ++ i ) {
        ++ effort;
        //std::cout << "(offset, i) = " << offset << ", " << i << "\n";
        if ( condensed_code [ count ] & bit ) {
          ++ effort;
          //std::cout << "bit = " << bit << "\n";
          //std::cout << "Writing connection (" << offset + i << ", " << count << ")\n";
          (*output)[offset + i] . push_back ( count );
        } // if
        bit <<= 1;
      } // for bit index
    } // for morse set
  } // for groups
#ifdef CMG_VERBOSE
  std::cout << "\r100%  Reachability Analysis Complete.\n ";
#endif
  DEBUGPRINT std::cout << "reach effort = " << effort << "\n";
} /* compute_reachability */


template < class OutEdgeGraph >
void find_short_cycles (std::vector<std::vector<typename OutEdgeGraph::size_type> > * output, 
                        const OutEdgeGraph & G,
                        int length ) {
  typedef typename OutEdgeGraph::size_type size_type;
  typedef std::pair<size_type, size_type> Edge;
  //const size_type sentinel = G . sentinel ();
  
  // bfs_tree maps a node to its parent and a set of children
  // Strategy:
  // Breadth first search.
  // bfs_queue only approach fails: why?
  // because once we detect a cycle, we don't know how to backtrace it.
  // solution: remember parents of each node. Then when we detect a cycle, 
  // backtrace up to original node.
  // Problem: what about depth-limited searches?
  // solution: remember depth of node.
  size_type N = G . num_vertices ();
  
  for ( size_type start = 0; start < N; ++ start ) { 
    std::queue < size_type > bfs_queue;
    boost::unordered_map < size_type, size_type > bfs_parent;
    boost::unordered_map < size_type, size_type > bfs_depth;
    bfs_queue . push ( start );
    bfs_depth [ start ] = 0;
    bfs_parent [ start ] = start; // to make it appear explored
    while ( not bfs_queue . empty () ) {
      size_type v = bfs_queue . front ();
      bfs_queue . pop ();
      if ( bfs_depth [ v ] > length ) continue;
      std::vector<size_type> adj = G . adjacencies ( v );
      BOOST_FOREACH ( const size_type & u, adj ) {
        if ( u == start ) {
          // Found a cycle
          size_type w = v;
          std::vector<size_type> cycle ( bfs_depth [ v ] + 1 );
          for ( int i = bfs_depth [ v ]; i >= 0; -- i ) {
            cycle [ i ] = w;
            w = bfs_parent [ w ];
          }
          output -> push_back ( cycle );
        }
        if ( bfs_parent . find ( u ) == bfs_parent . end () ) { 
          bfs_queue . push ( u );
          bfs_parent [ u ] = v;
          bfs_depth [ u ] = bfs_depth [ v ] + 1;
        }
      }
    }
  }
}

