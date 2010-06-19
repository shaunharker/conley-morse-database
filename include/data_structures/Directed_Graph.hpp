/*
 *  Directed_Graph.hpp
 */

template < class Topcell, class Toplex_Subset >
typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t 
DirectedGraph<Topcell, Toplex_Subset>::getStronglyConnectedComponents(
  DirectedGraph<Topcell, Toplex_Subset>::Components & C)
{
  // dummy: Pretend SCC has only one component (G itself).

  Toplex_Subset E;

  typename std::map<Topcell, Toplex_Subset>::iterator i;
  i = std::map<Topcell, Toplex_Subset>::begin();
  do {
    E.insert(i->first);
    i++;
  } while (i != std::map<Topcell, Toplex_Subset>::end());

  C.push_back(E);

  return C.size();
}

template < class Topcell, class Toplex_Subset >
typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t 
DirectedGraph<Topcell, Toplex_Subset>::getConnectingPathLength(
  typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t c1,
  typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t c2)
{
  // dummy
  return 0;
}

template < class Topcell, class Toplex_Subset >
typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t 
DirectedGraph<Topcell, Toplex_Subset>::getStablePathLength(
  typename DirectedGraph<Topcell, Toplex_Subset>::comp_index_t c) 
{
  // dummy
  return 0;
}
