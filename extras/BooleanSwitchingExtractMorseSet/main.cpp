
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
#include "boost/shared_ptr.hpp"
#include "Parameter/BooleanSwitchingParameterSpace.h"
#include "Parameter/FactorGraph.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/Database.h"

#include <boost/algorithm/string.hpp>

template <typename T>
std::string NumberToString ( T Number );


template <typename T>
std::string NumberToString ( T Number )
{
  std::stringstream ss;
  ss << Number;
  return ss.str();
}

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

  // std::cout << "Number of records : " << mgcc_records.size() << "\n";

  // the index of interest comes from the last argument
  uint64_t mgcc;
  mgcc = atoi(argv[argc-1]);

  // Initialize the model
  Model model;
  model . initialize ( argc-1, argv+1 );
  boost::shared_ptr<Grid> phase_space = model . phaseSpace ();
  if ( not phase_space ) {
    throw std::logic_error ( "Clutching_Graph_Job. model.phaseSpace() failed"
                             " to return a valid pointer.\n");
  }
  //
  BooleanSwitchingParameterSpace & boolean_space = *
  boost::dynamic_pointer_cast<BooleanSwitchingParameterSpace> ( model . parameterSpace () );

std::string sfile = "walls.txt";
model . saveWalls ( sfile.c_str() );


  // To save all the parameter inequalities
  std::ofstream parameterfile;
  parameterfile . open ( "parametersInequalities.txt" );

// Loop through all the mgccp_records

  for ( unsigned int imgccp=0; imgccp<mgcc_records[mgcc].mgccp_indices.size(); ++imgccp ) {

    const MGCCP_Record & mgccp_record =
    database . MGCCP_Records () [ mgcc_records[mgcc].mgccp_indices[imgccp] ];

    // std::cout << "Number of records for the given MGCC : " << mgcc_records[mgcc].mgccp_indices.size() << "\n";

    uint64_t morsegraph_index = mgccp_record . morsegraph_index;
    mgr_indices . insert ( morsegraph_index );
    const MorseGraphRecord & morsegraph_record = database . morsegraphData () [ morsegraph_index ];

    uint64_t dag_index = morsegraph_record . dag_index;
    const DAG_Data & dag_data = database . dagData () [ dag_index ];

    // std::cout << "Number of vertices : " << dag_data . num_vertices << "\n";

    std::vector < uint64_t > pindex = mgccp_record . parameter_indices;

    // Loop through all the parameters for a given mgccp
    for ( unsigned int ip=0; ip<pindex.size(); ++ip ) { 
      //
      boost::shared_ptr<BooleanSwitchingParameter> p =
      boost::dynamic_pointer_cast<BooleanSwitchingParameter> (
                                                              boolean_space . parameter ( pindex[ip] ) );
 
// DEBUG
// Get the wall Maps 
std::vector < std::pair<int64_t,int64_t> > wallMaps = model . getWallMaps ( p );
// Save the OutEdges 
std::unordered_map < int64_t, std::set<int64_t> > outEdges;
//
for ( unsigned int iw=0; iw<wallMaps.size(); ++iw ) { outEdges [ wallMaps[iw].first ] . insert ( wallMaps[iw].second ); }
std::ofstream oofile ( "outEdges_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".txt" );
for ( std::unordered_map < int64_t, std::set<int64_t> >::iterator it1=outEdges.begin(); it1!=outEdges.end(); ++it1 ) { 
  oofile << it1->first << " ";
  std::set<int64_t> newset ( it1->second );
  for ( std::set<int64_t>::iterator it=newset.begin(); it!=newset.end(); ++it ) {
    oofile << *it << " ";
  }
  oofile << "\n";
}
oofile . close();
// END DEBUG
//
//
// Save the parameter inequalities 
      parameterfile << boolean_space . prettyPrint ( p );
      parameterfile << "\n\n";
      //
      boost::shared_ptr<const Map> map = model . map ( p );
      if ( not map ) {
        std::cout << "No map with mgccp index " << imgccp << "and with parameter " <<
        *p << "!.\n";
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

// MAY NEED TO PUT BACK MODEL INIT HERE, MAY HAVE OVERLAP IN THE ANNOTATION
      model . annotate ( & mg );

      std::cout << "Computed a Morse graph with " << mg . NumVertices () << " nodes.\n";
      //
// DEBUG
// Save the morsesets information and gather the wall index that composed the morsesets
std::ofstream ofile;
ofile.open("morseSets_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".txt");
//
std::set < Grid::GridElement > wallMSid; // to store the wall id of morsesets
typedef std::vector<Grid::GridElement> CellContainer;
typedef  MorseGraph::VertexIterator VI;
VI it, stop;
for (boost::tie ( it, stop ) = mg . Vertices (); it != stop;  ++ it ) {
  boost::shared_ptr<const Grid> my_subgrid ( mg . grid ( *it ) );
  //
  if ( not my_subgrid ) {
    std::cout << "Abort! This vertex does not have an associated grid!\n";
    abort ();
  }
  CellContainer my_subset = phase_space -> subset ( * my_subgrid );
  //
  BOOST_FOREACH ( Grid::GridElement ge, my_subset ) {
    if ( not boost::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) ) ) {
      std::cout << "Unexpected null response from geometry\n";
    }
    AtlasGeo geo = * boost::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) );
    RectGeo box =  geo . rect ();
    int id = geo . id ();
    ofile << *it << " " << id ;
    ofile << "\n";
    wallMSid . insert ( id );   
  }
}
ofile.close();
// END DEBUG
//

// DEBUG
// From the wall id of the morsesets, output their information
std::ofstream wallFile;
wallFile . open ( "wallsMS_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".txt" );

std::set < Grid::GridElement >::iterator itms, it1, it2;
std::unordered_map <size_t,Wall> walls = model.getWalls(); // here switch the <key,value> from Model.h class 
std::unordered_map <size_t,Wall>::iterator itw;
    
// go through the list of grid element for the morseset and save the wall information and corresponding outedges
for ( itms=wallMSid.begin(); itms!=wallMSid.end(); ++itms ) { 
  // find the wall information 
  itw = walls . find ( *itms );
  if ( itw != walls.end() ) { 
    wallFile << *itms << " " << itw->first << " ";
    if ( itw -> second . isFixedPoint() ) {
      wallFile << "-1 ";
    } else {
      // check which direction is degenerated (should have only one)
      int dir;
      for ( unsigned int j=0; j<itw->second.rect().dimension(); ++j ) {
        if ( std::abs( itw->second.rect().upper_bounds[j]-itw->second.rect().lower_bounds[j] ) < 1e-12 ) {
          dir = j;
        }
      }
      wallFile << dir;
    }
    wallFile << " " << itw->second.rect() << "\n";
  } else { 
    std::cout << "Error could not find wall\n";
    abort;
  } 
}
wallFile . close();
// END DEBUG
//
// DEBUG
// output the graph of the walls from the morsesets only (Graphviz format)
   std::ofstream mapFile;
   mapFile . open ( "maps_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".gv" );
   mapFile << "digraph{ \n";

// Construct the wall graph for the moresets
   // not the best way to do it.
   for ( unsigned int iii=0; iii<wallMaps.size(); ++iii ) { 
       // wallMaps  vector < pair <int,int> > : <id1,id2> means id1 -> id2
       int64_t id1 = wallMaps[iii].first;
       int64_t id2 = wallMaps[iii].second;

       it1 = wallMSid . find ( id1 );
       it2 = wallMSid . find ( id2 );

       if ( it1 != wallMSid.end() && it2 != wallMSid.end() ) { 
	 mapFile << id1 << " -> " << id2 << "\n"; 
       }
   } 
   mapFile << "}";
   mapFile . close();


    // Extract the correspondence network node and variable names
    BooleanSwitching::Network network;
    std::string str1 = argv[2];
    std::string str2 = argv[3];
    std::string str3 = str1 + "/" + str2;
    network.load(str3.c_str());

    std::ofstream variablefile;
    variablefile . open ( "variables_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".txt" );
    // index starts at 1
    for ( unsigned int i=1; i<=network.size(); ++i ) {
      variablefile << i-1 << " " << network.name(i) << "\n";
    }
    variablefile.close();

  } // end of loop through parameters

} // end loop through mgccp






  return 0;
}
