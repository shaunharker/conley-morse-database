/* MorseGraph.h */

#ifndef _CMDP_MORSE_GRAPH_
#define _CMDP_MORSE_GRAPH_

#include <utility>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/foreach.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "boost/serialization/shared_ptr.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "database/structures/Grid.h"
#include "chomp/ConleyIndex.h"


/** Conley-Morse Graph. (Can be called either MorseGraph or ConleyMorseGraph)
 *  It is the directed acyclic graph whose vertices may
 *  be annotated with "Grid" objects, representing combinatorial Morse sets,
 *  and "ConleyIndex_t" objects, representing Conley Indexes.
 *  The edges of the graph represent the reachability relation among the
 *  combinatorial Morse sets. */
class MorseGraph {
 public:

  // Vertex types
  typedef int Vertex;  
  typedef boost::counting_iterator<Vertex> VertexIterator;
  typedef std::pair<VertexIterator, VertexIterator> VertexIteratorPair;

  // Edge types
  typedef std::pair<Vertex, Vertex> Edge;
  typedef boost::unordered_set<Edge>::const_iterator EdgeIterator;
  typedef std::pair<EdgeIterator, EdgeIterator> EdgeIteratorPair;

  /** Create an empty graph */
  MorseGraph();
  /** Initialize MorseGraph with phase space */
  template < class grid_ptr >
  MorseGraph ( grid_ptr phasespace ) : phasespace_(phasespace) {}
  /** Load MorseGraph from file */
  MorseGraph ( const char * filename );
  // WRITE FUNCTIONS
  
  /** Create a new vertex and return the descriptor of the vertex.
   *  The vertex is not connected to anywhere.
   *  An empty grid and ConleyIndex are allocated and
   *  assigned to the vertex just after this function is called.
   *  ("empty" objects mean the objects created by default-constructor.)
   */
  Vertex AddVertex();
  
  /** Add a edge from the "from" vertex to the "to" vertex.
   */
  Edge AddEdge(Vertex from, Vertex to);

  /** Remove a "from"-"to" edge
   *
   *  NOTE: some edge iterators become invalid when you call this function 
   */
  void RemoveEdge(Vertex from, Vertex to);
  
  //// READ FUNCTIONS
  
  /** return a number of vertices */
  unsigned int NumVertices() const;
  
  /** return a iterator pair to all vertices */
  VertexIteratorPair Vertices() const;
  
  /** return a iterator pair to all edges */
  EdgeIteratorPair Edges() const;
    
  //// PROPERTY ACCESS
  
  /** Get the grid associated with the vertex. */
  boost::shared_ptr<Grid> & phaseSpace ( void );
  boost::shared_ptr<const Grid> phaseSpace ( void ) const;
  boost::shared_ptr<Grid> & grid (Vertex vertex);
  boost::shared_ptr<const Grid> grid (Vertex vertex) const;
  boost::shared_ptr<chomp::ConleyIndex_t> & conleyIndex (Vertex vertex);
  boost::shared_ptr<const chomp::ConleyIndex_t> conleyIndex (Vertex vertex) const;
  
  /** Remove the grids associated with the vertices */
  void clearGrids ( void );
  
  //// FILE IO

  /** Save to File */
  void save ( const char * filename ) const {
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
  }
  
  /** Load from file */
  void load ( const char * filename ) {
    std::ifstream ifs(filename);
    if ( not ifs . good () ) {
      std::cout << "Could not load " << filename << "\n";
      throw "Could not load!";
    }
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
  }
private:
  // DATA
  int num_vertices_;
  boost::unordered_set < Edge > edges_;
  boost::shared_ptr < Grid > phasespace_;
  std::vector < boost::shared_ptr <Grid > > grids_;
  std::vector < boost::shared_ptr < chomp::ConleyIndex_t > > conleyindexes_;
  
  //// SERIALIZATION
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & num_vertices_;
    ar & edges_;
    ar & phasespace_;
    ar & grids_;
    ar & conleyindexes_;
  }
};

typedef MorseGraph ConleyMorseGraph;

/****************/
/*  DEFINITIONS */
/****************/

/** Create an empty graph */
inline
MorseGraph::MorseGraph ( void ) {
  num_vertices_ = 0;
}

inline
MorseGraph::MorseGraph ( const char * filename ) {
  load ( filename );
}

/** Create a new vertex and return the descriptor of the vertex.
 *  The vertex is not connected to anywhere.
 *  An empty gridPtr and ConleyIndex are allocated and
 *  assigned to the vertex just after this function is called.
 *  ("empty" objects mean the objects created by default-constructor.)
 */
inline MorseGraph::Vertex MorseGraph::AddVertex() {
  int v = num_vertices_ ++;
  grids_ . resize ( num_vertices_ );
  conleyindexes_ . resize ( num_vertices_ );
  return v;
}

/** Add a edge from the "from" vertex to the "to" vertex. */
inline MorseGraph::Edge MorseGraph::AddEdge(Vertex from, Vertex to) {
  return * edges_ . insert ( Edge (from, to) ) . first;
}

/** Remove a "from"-"to" edge. (invalidates iterators) */
inline void MorseGraph::RemoveEdge(Vertex from, Vertex to) {
  edges_ . erase ( Edge (from, to) );
}

/** return a number of vertices */
inline unsigned int MorseGraph::NumVertices() const {
  return num_vertices_;
}

/** return a iterator pair to all vertices */
inline MorseGraph::VertexIteratorPair
MorseGraph::Vertices() const {
  return VertexIteratorPair ( 0, num_vertices_ );
}

/** return a iterator pair to all edges */
inline MorseGraph::EdgeIteratorPair
MorseGraph::Edges() const {
  return EdgeIteratorPair(edges_.begin(), edges_.end());
}

// PROPERTY ACCESS

/** accessor method for phase space grid */
boost::shared_ptr<Grid> & MorseGraph::phaseSpace ( void ) {
  return phasespace_;
}
/** accessor method for phase space grid, const version */
boost::shared_ptr<const Grid> MorseGraph::phaseSpace ( void ) const {
  return phasespace_;
}

/** accessor method for grid assigned to vertex */
inline boost::shared_ptr<Grid> & MorseGraph::grid(Vertex vertex) {
  return grids_[vertex];
}

/** accessor method for grid assigned to vertex, const version */
inline
boost::shared_ptr<const Grid> MorseGraph::grid(Vertex vertex) const {
  return grids_[vertex];
}

/** accessor method for conley index assigned to vertex */
inline
boost::shared_ptr<chomp::ConleyIndex_t> & MorseGraph::conleyIndex(Vertex vertex) {
  return conleyindexes_ [ vertex ];
}

/** accessor method for conley index assigned to vertex, const version */
inline boost::shared_ptr<const chomp::ConleyIndex_t>
MorseGraph::conleyIndex (Vertex vertex) const {
  return conleyindexes_ [ vertex ];
}

/** method to clear grids associated with all vertices */
inline void MorseGraph::clearGrids ( void ) {
  phasespace_ . reset ();
  VertexIteratorPair vip = Vertices ();
  for ( VertexIterator vi = vip . first; vi != vip . second; ++ vi ) {
    grid ( *vi ) . reset ();
  }
}


#endif
