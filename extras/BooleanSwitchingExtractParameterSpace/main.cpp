
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


//arguments :

//path/database.mdb path path/networks/mynetwork.txt mgcc_index

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

  // the index of interest comes from the last argument
  uint64_t mgcc;
  mgcc = atoi(argv[argc-1]);


  Model model;
  model . initialize ( argc-1, argv+1 );
  //
  BooleanSwitchingParameterSpace & boolean_space = *
  std::dynamic_pointer_cast<BooleanSwitchingParameterSpace> (
  model . parameterSpace () );


  typedef std::pair<uint64_t, uint64_t> Edge;
  std::vector<Edge> edges;
  std::unordered_set < uint64_t > nodes;

  std::ofstream infoFile;
  infoFile . open ( "parameterGraphInequalities.txt" );
  infoFile << "{\n\"data\":[";

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
    // Extract a subpace of the parameter space
    std::unordered_set<uint64_t> subgraph;
    //
   for ( uint64_t p=0; p<pindex.size()-1; ++p ) {
      subgraph.insert(pindex[p]);
      nodes.insert(pindex[p]);
      infoFile << "{\"" << pindex[p] << "\":\"";
      infoFile << boolean_space . prettyPrint ( boolean_space . parameter ( pindex[p] ) );
      infoFile << "\"},\n";
    }
    subgraph.insert(pindex[pindex.size()-1]);
    nodes.insert(pindex[pindex.size()-1]);
    infoFile << "{\"" << pindex[pindex.size()-1] << "\":\"";
    infoFile << boolean_space . prettyPrint ( boolean_space . parameter ( pindex[pindex.size()-1] ) );
    infoFile << "\n\"}\n]\n}";
    //
    for ( uint64_t p : subgraph ) {
      std::vector<uint64_t> adj = boolean_space . adjacencies ( p );
      for ( uint64_t q : adj ) {
        if ( subgraph . count ( q ) ) {
          if ( p < q ) { edges . push_back ( std::make_pair ( p, q ) ); }
        }
      }
    }
  }


  // std::cout << "Number of records for the given MGCC : " << mgcc_records[mgcc].mgccp_indices.size() << "\n";

  // std::cout << "Number of vertices : " << dag_data . num_vertices << "\n";

  //

  infoFile . close ( );

// std::cout << "# edges : " << edges.size() << "\n";
// for ( Edge e : edges ) {
//   std::cout << e.first << " -> " << e.second <<"\n";
// }

// Output the parameter graph for the given MGCC number

// std::cout << "Number of parameters : " << pindex.size() << "\n";

  std::ofstream myfile;
  myfile . open ( "parameterGraphForaGivenMGCC.gv" );
  myfile << "graph {\n";
 for ( uint64_t n : nodes ) {
  //  std::cout << "i= " << i << " : " << nodes[i] << "\n";
   myfile << n <<";\n";
 }

 for ( Edge e : edges ) {
    myfile << e.first << " -- " << e.second <<"\n";
 }

 myfile << "}";
 myfile.close();

 //
/*
  // pick one paramater index to construct p
  std::shared_ptr<BooleanSwitchingParameter> p =
  std::dynamic_pointer_cast<BooleanSwitchingParameter> (
                                                          boolean_space . parameter ( pindex[0] ) );
  //  std::cout << "Parameters inequalities : \n";
  std::ofstream parameterfile;
  parameterfile . open ( "parametersInequalities.txt" );
  parameterfile << boolean_space . prettyPrint ( boolean_space . parameter ( pindex[0] ) );
  parameterfile . close();

  //
  std::shared_ptr<const Map> map = model . map ( p );
  if ( not map ) {
    std::cout << "No map associated with parameter " <<
    *p << "!.\n";
  }
  std::shared_ptr<Grid> phase_space = model . phaseSpace ();
  if ( not phase_space ) {
    throw std::logic_error ( "Clutching_Graph_Job. model.phaseSpace() failed"
                            " to return a valid pointer.\n");
  }

  // Perform Morse Graph computation
  MorseGraph mg;
  Compute_Morse_Graph
  ( & mg,
   phase_space,
   map,
   0,
   0,
   0,
   0 );


  model . annotate ( & mg );

  std::cout << "Computed a Morse graph with "
  << mg . NumVertices () << " nodes.\n";

  //
  std::ofstream ofile;
  ofile.open("morseSets.txt");

  typedef std::vector<Grid::GridElement> CellContainer;
  typedef  MorseGraph::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = mg . Vertices (); it != stop;  ++ it ) {

    std::shared_ptr<const Grid> my_subgrid ( mg . grid ( *it ) );

    if ( not my_subgrid ) {
      std::cout << "Abort! This vertex does not have an associated grid!\n";
      abort ();
    }
    CellContainer my_subset = phase_space -> subset ( * my_subgrid );

    BOOST_FOREACH ( Grid::GridElement ge, my_subset ) {
      if ( not std::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) ) ) {
        std::cout << "Unexpected null response from geometry\n";
      }
      AtlasGeo geo = * std::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) );
      RectGeo box =  geo . rect ();
      int id = geo . id ();
      // std::cout << "(Chart, Rect) = (" << id << ", " << box << ")\n";

      ofile << *it << " " << id ;//  << " ): " << box << "\n";

      ofile << "\n";

    }
  }

  ofile.close();


  // Extract the coorespondence network node and variable names
  BooleanSwitching::Network network;
  std::string str1 = argv[2];
  std::string str2 = argv[3];
  std::string str3 = str1 + "/" + str2;
  network.load(str3.c_str());

  std::ofstream variablefile;
  variablefile . open ( "variables.txt" );
  // index starts at 1
  for ( unsigned int i=1; i<=network.size(); ++i ) {
    variablefile << i-1 << " " << network.name(i) << "\n";
  }
  variablefile.close();
*/
  return 0;
}
