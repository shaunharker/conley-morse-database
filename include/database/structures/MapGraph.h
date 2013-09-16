// MapGraph.h

#ifndef CMDP_MAPGRAPH_H
#define CMDP_MAPGRAPH_H

#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <unistd.h>

#include "boost/unordered_map.hpp"
#include "boost/foreach.hpp"

#include "database/structures/Grid.h"

#ifdef CMDB_STORE_GRAPH
#include "database/program/ComputeGraph.h"
#endif

template < class Map >
class MapGraph {
public:
  // Typedefs
  typedef Grid::size_type size_type;
  typedef Grid::GridElement Vertex;
  
  // Public Methods
  
  // Constructor. Requires Grid and Map.
  MapGraph ( const Grid & t, 
             const Map & f );
  
  // Return vector of Vertices which are out-edge adjacencies of input v
  std::vector<Vertex> adjacencies ( const Vertex & v ) const;
  
  // Return number of vertices
  size_type num_vertices ( void ) const;

private:
  // Private methods
  std::vector<size_type> compute_adjacencies ( const size_type & v ) const;
  // Private data
  const Grid & grid_;
  const Map & f_;
  // Variables used if graph is stored in memory. (See CMDB_STORE_GRAPH define)
  bool stored_graph;
  std::vector<std::vector<Vertex> > adjacency_storage_;
};

// Repeated code in constructors is bad practice -- should fix that below
template < class Map >
MapGraph<Map>::
MapGraph ( const Grid & grid,
           const Map & f ) : 
grid_ ( grid ),
f_ ( f ),
stored_graph ( false ) {
  
#ifdef CMDB_STORE_GRAPH
  
  // Determine whether it is efficient to use an MPI job to store the graph
  if ( num_vertices () < 10000 ) {
    stored_graph = false;
    return;
  }
  stored_graph = true;
  
  // Make a file with required integrations
  MapEvals evals;
  evals . parameter () = f . parameter ();
  for ( size_type source = 0; source < num_vertices (); ++ source ) {
    Vertex domain_cell = lookup ( source );
    evals . insert ( domain_cell );
  }
  
  std::cout << "Saving grid to file.\n";
  // Save the grid and a list of required evaluations to disk
  grid_ . save ("grid.txt");
  evals . save ( "mapevals.txt" );
  
  // Call a program to compute the adjacency information
  std::cout << "Calling MPI program to evaluate map.\n";
  system("./COMPUTEGRAPHSCRIPT");
  std::cout << "MPI program returned.\n";

  // Load and store the adjacency information
  evals . load ( "mapevals.txt" );
  adjacency_storage_ . resize ( num_vertices () );
  for ( size_type source = 0; source < num_vertices (); ++ source ) {
    Vertex domain_cell = lookup ( source );    
    index ( &adjacency_storage_ [ source ], evals . val ( domain_cell ) );
  }
  std::cout << "Map stored.\n";
#endif
}

template < class Map >
std::vector<typename MapGraph<Map>::Vertex>
MapGraph<Map>::
adjacencies ( const size_type & source ) const {
  if ( stored_graph )
    return adjacency_storage_ [ source ];
  else
    return compute_adjacencies ( source );
}

template < class Map >
std::vector<typename MapGraph<Map>::Vertex>
MapGraph<Map>::
compute_adjacencies ( const Vertex & source ) const {
  //std::cout << "compute_adjacencies.\n";
  std::vector < Vertex > target = 
    grid_ . cover ( f_ ( grid_ . geometry ( source ) ) ); // here is the work
  //std::cout << "computed.\n";

  return target;
}


template < class Map >
typename MapGraph<Map>::size_type
MapGraph<Map>::num_vertices ( void ) const {
  return grid_ . size ();
}

#endif
