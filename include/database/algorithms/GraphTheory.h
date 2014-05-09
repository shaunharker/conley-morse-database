// GraphTheory.hpp Shaun Harker 5/16/2011

#ifndef CMDP_GRAPHTHEORY
#define CMDP_GRAPHTHEORY

#include "database/structures/Grid.h"
#include "database/maps/Map.h"
#include <vector>
#include <queue>

#include <boost/shared_ptr.hpp>

void computeMorseSetsAndReachability (std::vector< boost::shared_ptr<Grid> > * output,
                                      std::vector<std::vector<unsigned int> > * reach,
                                      boost::shared_ptr<const Grid> G,
                                      boost::shared_ptr<const Map> f );

#if 0

/** compute_morse_sets */
template < class MorseGraph, class Graph, class CellContainer >
void compute_morse_sets (std::vector< CellContainer > * output,
                         const Graph & G,
                         /* optional output */ MorseGraph * MG = NULL);

#endif

/** compute_strong_components */
template < class OutEdgeGraph >
void computeStrongComponents (std::vector<std::deque<typename OutEdgeGraph::size_type> > * output, 
                                const OutEdgeGraph & G, 
          /* optional output */ std::deque<typename OutEdgeGraph::size_type> * topological_sort = NULL,
          /* optional output */ std::deque<typename OutEdgeGraph::size_type> * SCC_root = NULL);
/** compute_reachability */
template < class Graph >
void computeReachability ( std::vector < std::vector < unsigned int > > * output, 
                           std::vector<std::deque<typename Graph::size_type> > & morse_sets, 
                           const Graph & G, 
                           const std::deque<typename Graph::size_type> & topological_sort );

#include "database/algorithms/GraphTheory.hpp"

#endif
