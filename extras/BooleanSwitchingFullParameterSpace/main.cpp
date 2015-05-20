
// Serialization CPP code
#include <boost/serialization/export.hpp>
#include "database/structures/Grid.h"
#include "database/structures/PointerGrid.h"
#include "database/structures/SuccinctGrid.h"
#include "database/structures/UniformGrid.h"
#include "database/structures/EdgeGrid.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/AbstractParameterSpace.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(UniformGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EdgeGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameter);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameterSpace);
BOOST_CLASS_EXPORT_IMPLEMENT(AbstractParameterSpace);

#define BS_DEBUG_MODELMAP
#include <iostream>
#include "Model.h"
#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "boost/foreach.hpp"
#include <memory>
#include "Parameter/BooleanSwitchingParameterSpace.h"
#include "Parameter/FactorGraph.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/Database.h"

#include <boost/algorithm/string.hpp>

#include "Colormap.h"

//arguments :

//path/database.mdb path2 mynetwork.txt

int main ( int argc, char * argv [] ) {
  Database database;
  database . load ( argv [ 1 ] );

  const AbstractParameterSpace & space =
  dynamic_cast<const AbstractParameterSpace&> (database . parameter_space ());

  boost::unordered_set < uint64_t > parameters;
  boost::unordered_set < uint64_t > mgr_indices;

  const std::vector < MGCC_Record > & mgcc_records =
  database . MGCC_Records ();

  std::cout << "Number of records : " << mgcc_records.size() << "\n";

  Model model;
  model . initialize ( argc-1, argv+1 );
  //
  BooleanSwitchingParameterSpace & boolean_space = *
  std::dynamic_pointer_cast<BooleanSwitchingParameterSpace> (
  model . parameterSpace () );

  typedef std::pair<uint64_t, uint64_t> Edge;
  std::vector<Edge> edges;

  // will store the nodes belonging to each mgcc
  std::vector< std::pair< uint64_t, std::vector<uint64_t> > > mgccNodes;

  // Extract a subpace of the parameter space
  std::unordered_set<uint64_t> subgraph;

  // Loop through MGCC
  for ( uint64_t mgcc=0; mgcc<mgcc_records.size(); ++mgcc ) {

    std::vector<uint64_t> mynodes;

    // Loop through the MGCCPs
    for ( unsigned int mgccpi=0; mgccpi<mgcc_records[mgcc].mgccp_indices.size(); ++mgccpi ) {

      //
      const MGCCP_Record & mgccp_record =
      database . MGCCP_Records () [ mgcc_records[mgcc].mgccp_indices[mgccpi] ];
      //
      uint64_t morsegraph_index = mgccp_record . morsegraph_index;
      mgr_indices . insert ( morsegraph_index );
      const MorseGraphRecord & morsegraph_record =
      database . morsegraphData () [ morsegraph_index ];
      //
      uint64_t dag_index = morsegraph_record . dag_index;
      const DAG_Data & dag_data = database . dagData () [ dag_index ];
      //
      std::vector < uint64_t > pindex = mgccp_record . parameter_indices;
      //

      //
     for ( uint64_t p=0; p<pindex.size()-1; ++p ) {
        subgraph.insert(pindex[p]);
        mynodes.push_back(pindex[p]);
      }
      subgraph.insert(pindex[pindex.size()-1]);
      mynodes.push_back(pindex[pindex.size()-1]);
    }

    mgccNodes . push_back ( std::pair<uint64_t,std::vector<uint64_t> >(mgcc,mynodes) );

  }

  for ( uint64_t p : subgraph ) {
    std::vector<uint64_t> adj = boolean_space . adjacencies ( p );
    for ( uint64_t q : adj ) {
      if ( subgraph . count ( q ) ) {
        if ( p < q ) { edges . push_back ( std::make_pair ( p, q ) ); }
      }
    }
  }

  std::ofstream myfile;
  myfile . open ( "fullParameterGraph.gv" );
  myfile << "graph {\n";
  for ( std::pair< uint64_t,std::vector<uint64_t> > n : mgccNodes ) {
    for ( uint64_t value : n.second ) {
      // myfile << value << "[shape=circle, style=filled,colorscheme=paired12, fillcolor="<< n.first+1 <<" ]\n";
      myfile << value << "[shape=circle, style=filled, fillcolor=\""<< colorHex(n.first) <<"\" ]\n";
    }
  }

 for ( Edge e : edges ) {
    myfile << e.first << " -- " << e.second <<"\n";
 }

// Add the legend : consider 12 colors maximum based on paired12
// std::string colormap[] = {"#a6cee3","#1f78b4","#b2df8a","#33a02c","#fb9a99","#e31a1c","#fdbf6f","#ff7f00","#cab2d6"};

myfile << "{ rank = sink;\n";
myfile << "Legend [shape=none, margin=0, label=<\n";
myfile << "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
myfile << "<TR>\n";
myfile << "<td> MGCC: </td>\n";
for ( uint64_t mgcc=0; mgcc<mgcc_records.size(); ++mgcc ) {
  myfile << "<td bgcolor=\"" << colorHex(mgcc) << "\">" << mgcc << "</td>\n";
}
myfile << "</TR>\n";
myfile << "</TABLE>\n";
myfile << ">];\n";
myfile << "}\n";

 myfile << "}";
 myfile.close();

  return 0;
}
