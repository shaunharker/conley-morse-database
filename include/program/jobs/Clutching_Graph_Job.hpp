/*
 *  Clutching_Graph_Job.hpp
 */

#include "data_structures/Conley_Morse_Graph.h"
#include "distributed/Message.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <vector>
#include "data_structures/Cached_Box_Information.h"
#include <boost/iterator_adaptors.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include "program/Configuration.h"
#include "program/jobs/Compute_Conley_Morse_Graph.h"
#include "data_structures/UnionFind.hpp"
    
template<class CMGraph>
class VertexPairs {
 public:
  typedef typename CMGraph::Vertex Vertex;
  typedef typename std::pair<Vertex, Vertex> Pair;
  typedef typename std::vector<std::pair<Vertex, Vertex> >::const_iterator ConstIterator;
  typedef typename std::vector<std::pair<Vertex, Vertex> >::iterator Iterator;

  /* Create an emtpy set */
  VertexPairs() {}
  
  /* Add a pair */
  void Add(Vertex v1, Vertex v2) {
    pairs_.push_back(std::pair<Vertex, Vertex>(v1, v2));
  }

  /* Return a pair of begin/end iterators */
  std::pair<ConstIterator, ConstIterator> Pairs() const {
    return std::pair<ConstIterator, ConstIterator>(pairs_.begin(), pairs_.end());
  }

  std::pair<Iterator, Iterator> Pairs() {
    return std::pair<Iterator, Iterator>(pairs_.begin(), pairs_.end());
  }
  
 private:
  std::vector<std::pair<Vertex, Vertex> > pairs_;
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
bool ClutchingTwoGraphs(
    const CMGraph &graph1,
    const CMGraph &graph2,
    const Toplex &toplex1,
    const Toplex &toplex2,
    VertexPairs<CMGraph> *pairs) {
  typedef typename CMGraph::Vertex Vertex;

  std::set<Vertex> used;
  bool result = true;

  if (graph1.NumVertices() != graph2.NumVertices())
    result = false;
  
  BOOST_FOREACH (Vertex v1, graph1.Vertices()) {
    int n = 0;
    BOOST_FOREACH (Vertex v2, graph2.Vertices()) {
      if (Check_if_Intersect(toplex1, graph1.CubeSet(v1), toplex2, graph2.CubeSet(v2))) {
        if (pairs)
          pairs->Add(v1, v2);
        
        n++;
        /* if two components in one C-M graph intersect
           one reccurent set in other C-M graph, the two graphs does not
           share the "same" structure */
        if (n >= 2)
          result = false;
        if (!used.insert(v2).second)
          result = false;

#if 0
        /* if two intersected components have different properties,
           the two graphs does not
           share the "same" structure */
        result = result &&
                 (graph1.ConleyIndex(v1) == graph2.ConleyIndex(v2));
#endif
      }
    }
    /* if a reccurent set in one C-M graph intersects
       no reccurent set in other C-M graph, the two graphs does not
       share the "same" structure */
    if (n == 0)
      result = false;
  }
  return result;
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
void ClutchingGraph(
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
      if (ClutchingTwoGraphs<CMGraph>(*graph1, *graph2, *toplex1, *toplex2, NULL))
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
  
  typedef ConleyMorseGraph<typename Toplex::Subset, ConleyIndex> CMGraph;
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

  std::cout << "ClutchingGraph: The patch size is " << N << "\n";
  
  for (size_t n=0; n<N; n++) {
    std::cout << "Computing Conley Morse Graph for parameter box " << geometric_descriptions [ n ] << "\n";
    phase_space_toplexes[n].initialize(space_bounds);
#if 1
    std::map<size_t, Cached_Box_Information>::iterator it = cache_info.find(n);
    Cached_Box_Information* info = (it == cache_info.end()) ? NULL : &(it->second);
    Compute_Conley_Morse_Graph
        <CMGraph, Toplex, ParameterToplex, LeslieMap , Decide_Subdiv ,
        Decide_Conley_Index , Cached_Box_Information >
      (&conley_morse_graphs[n],
       &phase_space_toplexes[n],
       geometric_descriptions[n],
       decide_subdiv,
       decide_conley_index,
       info);
#endif
  }

  Patch<CMGraph, Toplex> patch(conley_morse_graphs, phase_space_toplexes, neighbour);
  ClutchingGraph<CMGraph, Patch<CMGraph, Toplex>, Toplex>(patch, &equivalent_classes);

  *result << job_number;
  *result << cache_info;
  *result << conley_morse_graphs;
  *result << cell_names;
  *result << equivalent_classes;
}
