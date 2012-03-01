/* Union-Find Data Structure (by Ippei) */
// We use this to keep track of the equivalence classes

#ifndef CMDP_UNIONFIND
#define CMDP_UNIONFIND

#include <algorithm>
#include <vector>
#include <boost/unordered_map.hpp> // hmm, shouldn't need this!!!!!
#include <boost/unordered_set.hpp> // hmm, shouldn't need this!!!!!

#include <boost/foreach.hpp>
#include <boost/iterator_adaptors.hpp>
#include "boost/tuple/tuple.hpp"

/** Quotient Set class.
 *  Holds all equivalent classes using union-find structure.
 *  T must be: copyable, assignable, default constructable, and comparable
 */
template<class T>
class UnionFind {
public:
  /** Default constructor; produce empty union-find structure */
  UnionFind ( void );
  
  /** Copy the iterator range into distinct equivalence classes */
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
  
  /* Access to elements as a set */
  const boost::unordered_set<T> & Elements ( void ) const;
  
private:
  struct Entry {
    T parent;
    /* if rank == 0, this entry is not a root, otherwise, this entry is root and
     * rank means the size of the set.
     */
    int rank; 
  };
  mutable boost::unordered_map<T, Entry> tree_;
  boost::unordered_set < T > elements_;
};

template<class T>
UnionFind<T>::UnionFind ( void ) {
}

template<class T>
template<class Iterator>
UnionFind<T>::UnionFind(Iterator begin, Iterator end) {
  for (Iterator i=begin; i != end; i++) {
    tree_[*i].rank = 1;
  }
}

template<class T> void UnionFind<T>::Union(T x, T y) {
  x = Representative(x);
  y = Representative(y);
  if (x != y) {
    Entry *x_entry = &tree_[x];
    Entry *y_entry = &tree_[y];
    // For efficiency, we take x (without loss) 
    // to have the higher rank. (See wikipedia page, for example)
    if (x_entry->rank < y_entry->rank) {
      std::swap(x, y);
      std::swap(x_entry, y_entry);
    }
    x_entry->rank += y_entry->rank;
    y_entry->parent = x;
    y_entry->rank = 0;
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
  elements_ . insert ( x );
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

  boost::unordered_map<T, size_t> roots;
  typename boost::unordered_map<T, size_t>::iterator it;
  bool inserted;
  typedef typename boost::unordered_map<T, size_t>::value_type vtype;
  size_t n = 0;
  
  BOOST_FOREACH (boost::tie(key, entry), tree_) {
    T rep = Representative(key);
    boost::tie(it, inserted) = roots.insert(vtype(rep, n)); // relies on bouncing the insert
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

template<class T>
const boost::unordered_set<T> & UnionFind<T>::Elements ( void ) const {
  return elements_;
}

#endif
