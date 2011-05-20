/*
 *  Clutching_Graph_Job.hpp
 */

#include "data_structures/Conley_Morse_Graph.h"
#include "distributed/Message.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <stack>
#include <vector>
#include <ctime>
#include <set>
#include "data_structures/Cached_Box_Information.h"
#include <boost/iterator_adaptors.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include "program/Configuration.h"
#include "program/jobs/Compute_Conley_Morse_Graph4.h"
#include "data_structures/UnionFind.hpp"

/* Prefix Tree Structure Used in Clutching Algorithm */

template < class T >
class PrefixNode {
  PrefixNode * left_;
  PrefixNode * right_;
  PrefixNode * parent_;
  std::vector<T> tag_;
public:
  PrefixNode ( void ) : left_ ( NULL ), right_ ( NULL ), parent_ ( NULL ), tag_ () {}
  PrefixNode ( PrefixNode * parent ) : left_ ( NULL ), right_ ( NULL ), parent_ ( parent ) {}
  ~PrefixNode ( void ) {
    if ( left_ != NULL ) delete left_;
    if ( right_ != NULL ) delete right_;
  }
  PrefixNode * left ( void ) {
    if ( left_ == NULL ) {
      left_ = new PrefixNode < T >;
      left_ -> parent_ = this;
    }
    return left_;
  }
  PrefixNode * right ( void ) {
    if ( right_ == NULL ) {
      right_ = new PrefixNode < T >;
      right_ -> parent_ = this;
    }
    return right_;
  }  
  PrefixNode * parent ( void ) {
    return parent_;
  } 
  std::vector<T> & tag ( void ) {
    return tag_;
  }
};

template < class T >
class PrefixTree {
  PrefixNode <T> * root_;
public:
  typedef std::vector<unsigned char> Prefix;
  typedef PrefixNode<T> * PrefixIterator;
  PrefixTree ( void ) : root_ ( NULL ) {}
  ~PrefixTree ( void ) {
    if ( root_ != NULL ) delete root_;
  }
  PrefixIterator insert ( const Prefix & p, const T & item ) {
    if ( root_ == NULL ) root_ = new PrefixNode < T >;
    PrefixNode <T> * node = root_;
    Prefix follow_me = p;
    for ( unsigned int i = 0; i < follow_me . size (); ++ i ) {
      if ( follow_me [ i ] == 0 ) {
        node = node -> left ();
      } else {
        node = node -> right ();
      }
    }
    node -> tag () . push_back ( item );
    return node;
  }
};

/** Compute a relation between two C-M graph
 *
 *  Return true if given two graphs has a "same" structure
 *  on the level of C-M graph.
 *
 *  All pairs of intersected components of two graphs
 *  are added to pairs.
 * 
 *  NOTE: now infromation of edges is not used
 */

template<class CMGraph, class Toplex>
bool ClutchingTwoGraphs( std::set < std::pair < typename CMGraph::Vertex, typename CMGraph::Vertex > > * result,
                        const CMGraph &graph1,
                        const CMGraph &graph2,
                        const Toplex &toplex1,
                        const Toplex &toplex2 ) {
  typedef typename CMGraph::Vertex Vertex;
  typedef std::vector<unsigned char> Prefix;
  /* Build a Prefix Tree */
  PrefixTree<Vertex> tree;
  std::stack < PrefixNode<Vertex> * > first_cells, second_cells;
  BOOST_FOREACH (Vertex v, graph1.Vertices()) {
    BOOST_FOREACH ( Top_Cell t, graph1 . CubeSet ( v ) ) {
      Prefix p = toplex1 . prefix ( t );
      first_cells . push ( tree . insert ( p, v ) );
    }
  }
  BOOST_FOREACH (Vertex v, graph2.Vertices()) {
    BOOST_FOREACH ( Top_Cell t, graph2 . CubeSet ( v ) ) {
      Prefix p = toplex2 . prefix ( t );
      second_cells . push ( tree . insert ( p, v ) );
    }
  }  
  /* Climb from each item place in tree and record which pairs we get */
  std::set < std::pair < Vertex, Vertex > > bipartite_graph;
  
  while ( not first_cells . empty () ) {
    PrefixNode < Vertex > * node = first_cells . top ();
    first_cells . pop ();
    Vertex a, b;
    a = node -> tag () [ 0 ];
    // If there are two items here, then we don't need to climb.
    if ( node -> tag () . size () > 1 ) {
      b = node -> tag () [ 1 ];
      bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) );
      continue;
    } 
    // Otherwise, we do need to climb.
    while ( node -> parent () != NULL ) {
      node = node -> parent ();
      if ( node -> tag () . size () > 0 ) {
        b = node -> tag () [ 0 ];
        bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) );
        //std::cout << "(" << a << ", " << b << ")\n";
        break; // Konstantin pointed this out
      }
    }
  }
  
  // Now an exact repeat for the other case
  while ( not second_cells . empty () ) {
    PrefixNode < Vertex > * node = second_cells . top ();
    second_cells . pop ();
    Vertex a, b;
    a = node -> tag () [ 0 ];
    // If there are two items here, then we don't need to climb.
    if ( node -> tag () . size () > 1 ) {
      b = node -> tag () [ 1 ];
      bipartite_graph . insert ( std::pair < Vertex, Vertex > ( a, b ) ); // pointless, actually; already there
      continue;
    } 
    // Otherwise, we do need to climb.
    while ( node -> parent () != NULL ) {
      node = node -> parent ();
      if ( node -> tag () . size () > 0 ) {
        b = node -> tag () [ 0 ];
        bipartite_graph . insert ( std::pair < Vertex, Vertex > ( b, a ) ); // notice the reversal
        //std::cout << "(" << b << ", " << a << ")\n";
        break; // Konstantin pointed this out
      }
    }
  }
  
  // Report matching information
  if ( result != NULL ) *result = bipartite_graph;
  
  // Now check to see if the clutching graph is indeed a matching
  std::set < Vertex > first_vertices, second_vertices;
  if ( graph1 . NumVertices () != graph2 . NumVertices () ) return false;
  typedef std::pair<Vertex,Vertex> VertexPair;
  BOOST_FOREACH ( const VertexPair & vertex_pair, bipartite_graph ) {
    if ( first_vertices . insert ( vertex_pair . first ) . second == false ) return false;
    if ( second_vertices . insert ( vertex_pair . second ) . second == false ) return false;
  }
  return true;
}

template<class CMGraph, class Toplex>
class Patch {
public:
  typedef size_t ParamBoxDescriptor;
  typedef boost::counting_iterator<size_t> ParamBoxIterator;
  typedef std::vector<std::pair<size_t, size_t> > AdjParamPairs;
  typedef std::pair<ParamBoxIterator, ParamBoxIterator> ParamBoxIteratorPair;
  typedef AdjParamPairs::const_iterator AdjParamBoxIterator;
  typedef std::pair<AdjParamBoxIterator, AdjParamBoxIterator> AdjParamBoxIteratorPair;
  
  Patch(const std::vector<CMGraph> &cmgraphs,
        const std::vector<Toplex> &toplexes,
        const std::vector<std::vector <size_t> > &neighbours)
  : cmgraphs_(cmgraphs),
  toplexes_(toplexes) {
    for (size_t i=0; i < neighbours.size(); i++) {
      BOOST_FOREACH (size_t j, neighbours[i]) {
        pairs_.push_back(std::pair<size_t, size_t>(i,j));
      }
    }
  }
  
  /** Return a pointer to C-M Graph related to that paramter */
  const CMGraph* GetCMGraph(ParamBoxDescriptor d) const {
    return &cmgraphs_[d];
  }
  const Toplex* GetToplex(ParamBoxDescriptor d) const {
    return &toplexes_[d];
  }
  
  ParamBoxIteratorPair ParamBoxes() const {
    return ParamBoxIteratorPair(boost::make_counting_iterator((size_t)0),
                                boost::make_counting_iterator(cmgraphs_.size()));
  }
  
  AdjParamBoxIteratorPair AdjecentBoxPairs() const {
    return AdjParamBoxIteratorPair(pairs_.begin(), pairs_.end());
  }
private:
  const std::vector<CMGraph> &cmgraphs_;
  const std::vector<Toplex> &toplexes_;
  AdjParamPairs pairs_;
};

/** Compute an equivalent classes and fill the result to "ret".
 *
 *  NOTE: *ret must be empty
 */
template<class CMGraph, class Patch, class Toplex>
void ProduceClutchingGraph(
                           const Patch &patch,
                           std::vector<std::vector<typename Patch::ParamBoxDescriptor> > *ret) {
  typename Patch::ParamBoxIteratorPair param_boxes_iters;
  typedef typename Patch::ParamBoxDescriptor ParamBoxDescriptor;
  
  param_boxes_iters = patch.ParamBoxes();
  
  UnionFind<ParamBoxDescriptor> quotient_set(param_boxes_iters.first,
                                             param_boxes_iters.second);
  
  ParamBoxDescriptor p1, p2;
  BOOST_FOREACH (boost::tie(p1, p2), patch.AdjecentBoxPairs()) {
    if (!quotient_set.Find(p1, p2)) {
      const CMGraph *graph1 = patch.GetCMGraph(p1);
      const CMGraph *graph2 = patch.GetCMGraph(p2);
      const Toplex *toplex1 = patch.GetToplex(p1);
      const Toplex *toplex2 = patch.GetToplex(p2);
      if (ClutchingTwoGraphs<CMGraph>(NULL, *graph1, *graph2, *toplex1, *toplex2))
        quotient_set.Union(p1, p2);
    }
  }
  
  quotient_set.FillToVector(ret);
  return;
}

/** Main function for clutching graph job.
 *
 *  This function is callled from worker, and compare graph structure
 *  for each two adjacent boxes.
 */
template <class Toplex, class ParameterToplex, class ConleyIndex >
void Clutching_Graph_Job ( Message * result , const Message & job ) {
  size_t job_number;
  std::vector<typename ParameterToplex::Geometric_Description> geometric_descriptions;
  std::map<size_t, Cached_Box_Information> cache_info;
  std::vector<std::vector<size_t> > neighbour;
  
#if 0
  typedef ConleyMorseGraph<typename Toplex::Subset, ConleyIndex> CMGraph;
#else
  typedef ConleyMorseGraph< std::vector < typename Toplex::Top_Cell >, ConleyIndex> CMGraph;
#endif
  std::vector < typename Toplex::Top_Cell > cell_names;
  job >> job_number;
  job >> cell_names;
  job >> geometric_descriptions;
  job >> cache_info;
  job >> neighbour;
  
  const size_t N = geometric_descriptions.size();
  
  std::vector<Toplex> phase_space_toplexes(geometric_descriptions.size());
  
  std::vector<CMGraph> conley_morse_graphs(geometric_descriptions.size());
  std::vector<std::vector<size_t> > equivalent_classes;
  
  clock_t start = clock ();
  
  //std::cout << "ProduceClutchingGraph: The patch size is " << N << "\n";
  
  for (size_t n=0; n<N; n++) {
    //std::cout << "Computing Conley Morse Graph for parameter box " << geometric_descriptions [ n ] << "\n";
    clock_t start1 = clock ();
    phase_space_toplexes[n].initialize(space_bounds);
#if 0
    std::map<size_t, Cached_Box_Information>::iterator it = cache_info.find(n);
    Cached_Box_Information* info = (it == cache_info.end()) ? NULL : &(it->second);
    Compute_Conley_Morse_Graph3 <CMGraph, Toplex, ParameterToplex, GeometricMap >
    (&conley_morse_graphs[n],
     &phase_space_toplexes[n],
     geometric_descriptions[n], true, false);
    std::cout << "Job " << job_number << " subtask done. Time = " << (float)(clock() - start1 ) / (float) CLOCKS_PER_SEC << "\n";
#else
    std::map<size_t, Cached_Box_Information>::iterator it = cache_info.find(n);
    Cached_Box_Information* info = (it == cache_info.end()) ? NULL : &(it->second);
    Compute_Conley_Morse_Graph4 <CMGraph, Toplex, ParameterToplex, GeometricMap >
    (&conley_morse_graphs[n],
     &phase_space_toplexes[n],
     geometric_descriptions[n], true, false);
    std::cout << "Job " << job_number << " subtask done. Time = " << (float)(clock() - start1 ) / (float) CLOCKS_PER_SEC << "\n";
    
#endif
  }
  
  std::cout << "Job " << job_number << " finished CMG construction. Time = " << (float)(clock() - start ) / (float) CLOCKS_PER_SEC << "\n";
  
  start = clock ();
  
  Patch<CMGraph, Toplex> patch(conley_morse_graphs, phase_space_toplexes, neighbour);
  ProduceClutchingGraph<CMGraph, Patch<CMGraph, Toplex>, Toplex>(patch, &equivalent_classes);

  std::cout << "Job " << job_number << " finished Clutching. Time = " << (float)(clock() - start ) / (float) CLOCKS_PER_SEC << "\n";

  *result << job_number;
  *result << cache_info;
  *result << conley_morse_graphs;
  *result << cell_names;
  *result << equivalent_classes;
}
