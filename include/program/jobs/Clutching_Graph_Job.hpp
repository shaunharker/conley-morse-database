/*
 *  Clutching_Graph_Job.hpp
 */

#include "data_structures/Conley_Morse_Graph.h"
#include "distributed/Message.h"
#include <boost/foreach.hpp>
#include <algorithm>
#include <vector>


/** Quotient Set class.
 *  Holds all equivalent classes using union-find structure.
 *  T must be: copyable, assignable, default constructable, and comparable
 */
template<class T>
class UnionFind {
 public:
  template<class Iterator>
  UnionFind(Iterator begin, Iterator end);
  
  /** If x and y contain different sets, these two sets are unioned.
   */
  void Union(T x, T y);
  /** Return true if x and y contain the same class.
   */
  bool Find(T x, T y) const;
  /** Return the representative element of the set containing x */
  T Representative(T x) const;

  /** Merge other UnionFind */
  void Merge(const UnionFind<T> &other);

  /** Add a new element {x} to Quotient set. if x is already contained,
   *  this function causes no effect.
   */
  void Add(T x);

  /** Add new element from iterator */
  template<class Iterator>
  void Add(Iterator begin, Iterator end);

  /* Copy all data to vector */
  void FillToVector(std::vector<std::vector<T> > *ret);
  
 private:
  struct Entry {
    T parent;
    /* if rank == 0, this entry is not a root, otherwise, this entry is root and
     * rank means the size of the set.
     */
    int rank; 
  };
  mutable std::map<T, Entry> tree_;
};

template<class T> void UnionFind<T>::Union(T x, T y) {
  x = Representative(x);
  y = Representative(y);
  if (x != y) {
    Entry *x_entry = &tree_[x];
    Entry *y_entry = &tree_[y];
    if (x_entry->rank < y_entry->rank)
      std::swap(x_entry, y_entry);
    x_entry->rank += y_entry->rank;
    y_entry->parent = x;
    y_entry->rank = 0;
  }
}

template<class T>
template<class Iterator>
UnionFind<T>::UnionFind(Iterator begin, Iterator end) {
  for (Iterator i=begin; i != end; i++) {
    tree_[*i].rank = 1;
  }
}

template<class T>
bool UnionFind<T>::Find(T x, T y) const {
  return Representative(x) == Representative(y);
}

template<class T>
T UnionFind<T>::Representative(T x) const {
  if (tree_[x].rank > 0) {
    return x;
  } else {
    T root = Representative(tree_[x].parent);
    tree_[x].parent = root;
    return root;
  }
}

template<class T>
void UnionFind<T>::Add(T x) {
  if (tree_.find(x) == tree_.end()) {
    tree_[x].rank = 1;
  }
}

template<class T>
void UnionFind<T>::Merge(const UnionFind<T> &other) {
  T key;
  Entry entry;

  BOOST_FOREACH (boost::tie(key, entry), other.tree_) {
    T root = other.Representative(key);
    Add(root);
    Add(key);
    Union(root, key);
  }
}

template<class T>
void UnionFind<T>::FillToVector(std::vector<std::vector<T> > *ret) {
  Entry entry;
  T key;

  std::map<T, size_t> roots;
  typename std::map<T, size_t>::iterator it;
  bool inserted;
  typedef typename std::map<T, size_t>::value_type vtype;
  size_t n = 0;
  
  BOOST_FOREACH (boost::tie(key, entry), tree_) {
    T rep = Representative(key);
    boost::tie(it, inserted) = roots.insert(vtype(rep, n));
    size_t k;
    if (inserted) {
      ret->push_back(std::vector<T>(1, rep));
      k = n;
      n++;
    } else {
      k = it->second;
    }
    if (key != rep)
      ret->at(k).push_back(key);
  }
}

    
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
template<class CMGraph>
bool ClutchingTwoGraphs(
    const CMGraph &graph1,
    const CMGraph &graph2,
    VertexPairs<CMGraph> *pairs) {
  typedef typename CMGraph::Vertex Vertex;

  std::set<Vertex> used;
  bool result = true;

  if (graph1.NumVertices() != graph2.NumVertices())
    result = false;
  
  BOOST_FOREACH (Vertex v1, graph1.Vertices()) {
    int n = 0;
    BOOST_FOREACH (Vertex v2, graph2.Vertices()) {
      if (Check_If_Intersect(*graph1.GetCubeSet(v1), *graph2.GetCubeSet(v2))) {
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

        /* if two intersected components have different properties,
           the two graphs does not
           share the "same" structure */
        result = result &&
                 (*graph1.GetConleyIndex(v1) == *graph2.GetConleyIndex(v2));
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

/** Compute an equivalent classes and fill the result to "ret".
 *
 *  NOTE: *ret must be empty
 */
template<class CMGraph, class Patch>
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
      if (ClutchingTwoGraphs<CMGraph>(*graph1, *graph2, NULL))
        quotient_set.Union(p1, p2);
    }
  }

  quotient_set.FillToVector(ret);
  return;
}

template <class Toplex, class ParameterToplex, class ConleyIndex >
void Clutching_Graph_Job ( Message * result , const Message & job ) {
  
  
}
