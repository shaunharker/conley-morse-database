// GraphTheory.hpp Shaun Harker 5/16/2011

#ifndef CMDP_GRAPHTHEORY
#define CMDP_GRAPHTHEORY

#include <vector>

/** class CombinatorialMap */
template < class Toplex, class CellContainer >
class CombinatorialMap {
public:
  typedef typename Toplex::size_type size_type;
  typedef typename Toplex::Top_Cell Vertex;
  CombinatorialMap ( const size_type N );
  size_type insert ( const Vertex & v );
  std::vector<size_type> & adjacencies ( const size_type & v );
  const std::vector<size_type> & adjacencies ( const size_type & v ) const;

  size_type sentinel ( void ) const;
  size_type index ( const Vertex & input ) const;
  size_type num_vertices ( void ) const;
  Vertex lookup ( const size_type & input ) const;
  void index ( std::vector < size_type > * output, const CellContainer & input ) const;
  void lookup ( CellContainer * output, const std::vector < size_type > & input ) const;
private:
  std::vector<std::vector<size_type> > data_;
  std::vector<Vertex> lookup_;
  std::vector<size_type> index_;
  size_type sentinel_;
};
  
/** compute_combinatorial_map */
template < class Toplex, class Map, class CellContainer>
CombinatorialMap<Toplex,CellContainer>
compute_combinatorial_map (const Toplex & my_toplex, 
                           const Map & f);


template < class Toplex, class Map, class CellContainer >
CombinatorialMap<Toplex,CellContainer>
compute_combinatorial_map (const std::vector < CellContainer > & sets,
                           const Toplex & my_toplex, 
                           const Map & f);

/** compute_morse_sets */
template < class MorseGraph, class Toplex, class CellContainer >
void compute_morse_sets (std::vector< CellContainer > * output, 
                         const CombinatorialMap<Toplex,CellContainer> & G, 
   /* optional output */ MorseGraph * MG = NULL);

/** compute_strong_components */
template < class OutEdgeGraph >
void compute_strong_components (std::vector<std::vector<typename OutEdgeGraph::size_type> > * output, 
                                const OutEdgeGraph & G, 
          /* optional output */ std::vector<typename OutEdgeGraph::size_type> * topological_sort = NULL);

/** compute_reachability */
template < class Graph >
void compute_reachability ( std::vector < std::vector < unsigned int > > * output, 
                           std::vector<std::vector<typename Graph::size_type> > & morse_sets, 
                           const Graph & G, 
                           const std::vector<typename Graph::size_type> & topological_sort );

#include "algorithms/GraphTheory.hpp"

#endif
