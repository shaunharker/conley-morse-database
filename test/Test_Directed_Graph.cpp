#include "data_structures/Directed_Graph.h"
#include <set>
#include <iostream>

struct Toplex 
{
  typedef unsigned int size_type;
  typedef size_type Top_Cell;
  typedef std::set<Top_Cell> Subset;
};

int main ( int argc, char * argv [] ) 
{
  typedef DirectedGraph <Toplex> Graph;

  int nVertices = 10;
  Toplex::Subset E[nVertices];

  /*
    The sample graph G has 10 vertices with edges defined by
    0 |--> 1
    1 |--> 2
    2 |--> 3, 5
    3 |--> 4
    4 |--> 1
    5 |--> 5, 6
    6 |-->
    7 |--> 8, 9
    8 |--> 9
    9 |--> 7
    Therefore, there should be three compones: {1234}, {5} and {789}  
  */

  E[0].insert(1);
  E[1].insert(2);
  E[2].insert(3);
  E[2].insert(5);
  E[3].insert(4);
  E[4].insert(1);
  E[5].insert(5);
  E[5].insert(6);
  E[7].insert(8);
  E[7].insert(9);
  E[8].insert(9);
  E[9].insert(7);

  Graph G;
  for (int i = 0; i < nVertices; i++) {
    G.insert(Graph::value_type(i, E[i]));
  }
  std::cout << "The size of the Graph: " << G.size() << std::endl; 

  std::vector< Toplex::Subset > Components;
  G.computeStronglyConnectedComponents(Components);
  std::cout << "The number of SCCs: " << Components.size() << std::endl;

  std::vector< Toplex::Subset >::iterator it = Components.begin();
  while (it != Components.end()) {
    std::cout << "component: ";
    Toplex::Subset::iterator comp_it = (*it).begin();
    while (comp_it != (*it).end()) {
      std::cout << *comp_it;;
      ++comp_it;
    }
    std::cout << std::endl;
    ++it;
  }

  Graph H;
  DirectedGraph<Toplex>::Components I;
  DirectedGraph<Toplex>::Components J;
  DirectedGraph<Toplex>::Component F;
  F.insert(5);
  I.push_back(F);
  H = collapseVertices(G, I, J);

  Components.clear();
  std::cout << "The size of the Graph: " << H.size() << std::endl; 

  H.computeStronglyConnectedComponents(Components);
  std::cout << "The number of SCCs: " << Components.size() << std::endl;
  
  it = Components.begin();
  while (it != Components.end()) {
    std::cout << "component: ";
    Toplex::Subset::iterator comp_it = (*it).begin();
    while (comp_it != (*it).end()) {
      std::cout << *comp_it;;
      ++comp_it;
    }
    std::cout << std::endl;
    ++it;
  }


  return 0;
}
