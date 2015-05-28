
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

template <typename T>
std::string NumberToString ( T Number );

void saveJSON ( const std::vector<uint64_t> & pIndex, 
		const std::vector<std::string> & pInequalities,
		const std::unordered_map<uint64_t,std::string> & wallInformation,	
  		const std::vector< std::unordered_map< uint64_t, std::vector<uint64_t> > > & outedges,
		const std::vector<std::string> & variables);

template <typename T>
std::string NumberToString ( T Number )
{
  std::stringstream ss;
  ss << Number;
  return ss.str();
}

//arguments :

//path/database.mdb path mynetwork.txt mgcc_index incc_index

int main ( int argc, char * argv [] ) {
  Database database;
  database . load ( argv [ 1 ] );

  const AbstractParameterSpace & space =
  dynamic_cast<const AbstractParameterSpace&> (database . parameter_space ());

  boost::unordered_set < uint64_t > parameters;
  boost::unordered_set < uint64_t > mgr_indices;


  const std::vector < MGCC_Record > & mgcc_records =
  database . MGCC_Records ();

  // the index of interest comes from the last two arguments
  uint64_t mgcc;
  mgcc = atoi(argv[argc-2]);
  uint64_t incc;
  incc = atoi(argv[argc-1]);

  // To store the correspondence betwen the space dimensions and variable names
  std::vector<std::string> variables;
  // To store the parameter index
  std::vector<uint64_t> parameterIndex;
  // To store the parameter inequalities
  std::vector<std::string> parameterInequalities;
  // To store the information of all the walls
  // here switch the <key,value> from Model.h class, to allow search easier 
  std::unordered_map <size_t,Wall> walls;
  std::unordered_map <size_t,Wall>::iterator itw;
 
  // To store the wall information (independent of parameter)
  std::unordered_map<uint64_t,std::string> wallInformation;
  std::unordered_map<uint64_t,std::string>::iterator itWallInfo;

  // To store the outedges between walls for the given incc for each parameter
  std::vector< std::unordered_map< uint64_t, std::vector<uint64_t> > > outedges;

  // Initialize the model
  Model model;
  model . initialize ( argc-1, argv+1 );
  std::shared_ptr<Grid> phase_space = model . phaseSpace ();
  if ( not phase_space ) {
    throw std::logic_error ( "Clutching_Graph_Job. model.phaseSpace() failed"
                             " to return a valid pointer.\n");
  }
  //
  BooleanSwitchingParameterSpace & boolean_space = *
  std::dynamic_pointer_cast<BooleanSwitchingParameterSpace> ( model . parameterSpace () );

  walls = model.getWalls(); 

  // Extract the correspondence network node and variable names
  BooleanSwitching::Network network;
  std::string str1 = argv[2];
  std::string str2 = argv[3];
  std::string str3 = str1 + "/" + str2;
  network.load(str3.c_str());
  // index starts at 1
  for ( unsigned int i=1; i<=network.size(); ++i ) { variables . push_back ( network.name(i) ); }

//  std::string sfile = "walls.txt";
//  model . saveWalls ( sfile.c_str() );


  // To save all the parameter inequalities
//  std::ofstream parameterfile;
//  parameterfile . open ( "parametersInequalities.txt" );

// Loop through all the mgccp_records

  for ( unsigned int imgccp=0; imgccp<mgcc_records[mgcc].mgccp_indices.size(); ++imgccp ) {

    const MGCCP_Record & mgccp_record =
    database . MGCCP_Records () [ mgcc_records[mgcc].mgccp_indices[imgccp] ];

    uint64_t morsegraph_index = mgccp_record . morsegraph_index;
    mgr_indices . insert ( morsegraph_index );
    const MorseGraphRecord & morsegraph_record = database . morsegraphData () [ morsegraph_index ];

    uint64_t dag_index = morsegraph_record . dag_index;
    const DAG_Data & dag_data = database . dagData () [ dag_index ];

    std::vector < uint64_t > pindex = mgccp_record . parameter_indices;

    // Loop through all the parameters for a given mgccp
    for ( unsigned int ip=0; ip<pindex.size(); ++ip ) { 
      //
      // store the parameter index
      parameterIndex . push_back ( pindex[ip] );
      //
      std::shared_ptr<BooleanSwitchingParameter> p =
      std::dynamic_pointer_cast<BooleanSwitchingParameter> ( boolean_space . parameter ( pindex[ip] ) );
      //
      // store the parameter inequalities
      parameterInequalities . push_back ( boolean_space . prettyPrint(p) );

      // Get the wall maps (to be used for the outedges )
      std::vector < std::pair<int64_t,int64_t> > wallMaps = model . getWallMaps ( p );

 /*
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
*/
//
//
// Save the parameter inequalities 
//      parameterfile << boolean_space . prettyPrint ( p );
//      parameterfile << "\n\n";
      //
      std::shared_ptr<const Map> map = model . map ( p );
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

//      std::cout << "Computed a Morse graph with " << mg . NumVertices () << " nodes.\n";
      //
// DEBUG
// Save the morsesets information and gather the wall index that composed the morsesets
//std::ofstream ofile;
//ofile.open("morseSets_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".txt");
//
//std::set < Grid::GridElement > wallMSid; // to store the wall id of morsesets
      typedef std::vector<Grid::GridElement> CellContainer;
      typedef  MorseGraph::VertexIterator VI;
      VI it, stop;
      for (boost::tie ( it, stop ) = mg . Vertices (); it != stop;  ++ it ) {
        // get the incc
        CS_Data my_cs_data;
        my_cs_data . vertices . push_back ( *it );
        uint64_t my_cs_index = database . csIndex ( my_cs_data );
        // Now we make an INCCP record by hand:
        INCCP_Record my_inccp_record;
        my_inccp_record . cs_index = my_cs_index;
        my_inccp_record . mgccp_index = mgcc_records[mgcc].mgccp_indices[imgccp];
        uint64_t my_inccp_index = database . inccpIndex ( my_inccp_record );
        //
        // Now we want the INCC index:
        uint64_t my_incc_index = database . inccp_to_incc () [ my_inccp_index ];
        //
        // Check against the given incc 
        if (  my_incc_index == incc ) {
          std::shared_ptr<const Grid> my_subgrid ( mg . grid ( *it ) );
          //
          if ( not my_subgrid ) {
            std::cout << "Abort! This vertex does not have an associated grid!\n";
            abort ();
          }
          CellContainer my_subset = phase_space -> subset ( * my_subgrid );
          //
          std::set <uint64_t> wallsMSid;
          BOOST_FOREACH ( Grid::GridElement ge, my_subset ) {
            if ( not std::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) ) ) {
              std::cout << "Unexpected null response from geometry\n";
            }
            AtlasGeo geo = * std::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) );
            RectGeo box =  geo . rect ();
            int id = geo . id ();

	    // store the wall index for the morse set
	    wallsMSid . insert ( id );

	    // check if this wall was already encountered, if not add it to the wallInformation
	    itWallInfo = wallInformation . find ( id );
            if ( itWallInfo == wallInformation.end() ) { 
        	itw = walls . find ( id );
		if ( itw != walls.end() ) {
                  std::string wallInfoString;
                  if ( itw -> second . isFixedPoint() ) {
                    wallInfoString = "-1 ";
                  } else {
                    // check which direction is degenerated (should have only one)
                    int dir;
                    for ( unsigned int j=0; j<itw->second.rect().dimension(); ++j ) {
                      if ( std::abs( itw->second.rect().upper_bounds[j]-itw->second.rect().lower_bounds[j] ) < 1e-12 ) {
                        dir = j;
                      }
                    }
                    wallInfoString = NumberToString(dir) + " ";
                  }
		  // print the rect 
		  std::string ss;
		  ss = "";
                  int mydim = itw->second.rect().dimension();
		  std::vector<double> lb = itw->second.rect().lower_bounds;
		  std::vector<double> ub = itw->second.rect().upper_bounds;
		  for ( unsigned int i=0; i<mydim-1; ++i )  { 
		    ss += "[" + NumberToString(lb[i]) + ", " + NumberToString(ub[i]) + "]x";
		  }
                  ss += "[" + NumberToString(lb[mydim-1]) + ", " + NumberToString(ub[mydim-1]) + "]";
                  wallInfoString += ss;
                  // add the wall information to WallInformation
                  wallInformation [ itw->first ] = wallInfoString;
                } else { 
                  std::cout << "Error: Could not find wall\n"; 
                  abort(); 
                }
	    }

//    ofile << *it << " " << id << " " << my_incc_index ;
//    ofile << "\n";
//    wallMSid . insert ( id );   
          } // end loop my_subset

	// Construct the outedges of the morse sets for the given parameter 
	std::unordered_map< uint64_t, std::vector<uint64_t> > myoutedges;
	// loop through all the wall maps and find the relevant one. 
	// Would be better to start for the morse set walls (smaller set)
	for ( unsigned int i=0; i<wallMaps.size(); ++i ) { 
	  uint64_t id1 = wallMaps[i].first;
	  uint64_t id2 = wallMaps[i].second;
	  std::set<uint64_t>::iterator it1, it2;	
	  it1 = wallsMSid . find ( id1 );
	  it2 = wallsMSid . find ( id2 );
	  if ( it1 != wallsMSid.end() && it2 != wallsMSid.end() ) { 
		myoutedges [ id1 ] . push_back  ( id2 );
	  } 
	
	} 	

outedges . push_back ( myoutedges );

        } // end condition on incc
      } // end loop over mg.Vertices
//ofile.close();
// END DEBUG
//
/*
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
*/
// DEBUG
// output the graph of the walls from the morsesets only (Graphviz format)
//   std::ofstream mapFile;
//   mapFile . open ( "maps_"+NumberToString(imgccp)+"_"+NumberToString(ip)+".gv" );
//   mapFile << "digraph{ \n";
/*
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
*/
  } // end of loop through parameters

} // end loop through mgccp


  saveJSON ( parameterIndex, parameterInequalities, wallInformation, outedges, variables );



  return 0;
}


void saveJSON ( const std::vector<uint64_t> & pIndex, 
		const std::vector<std::string> & pInequalities,
		const std::unordered_map<uint64_t,std::string> & wallInformation,
		const std::vector< std::unordered_map< uint64_t, std::vector<uint64_t> > > & outedges,
		const std::vector<std::string> & variables ) {

  std::ofstream ofile; 
  ofile . open ( "output.json" );
  //
  ofile << "{ \n";
// Parameter fields  
  ofile << "\"parameters\": {\n";
  ofile << "\"index\": [ ";
  for ( unsigned int i=0; i<pIndex.size()-1; ++i ) { ofile << pIndex[i] << ", "; }
  ofile << pIndex[pIndex.size()-1];
  ofile << " ],\n"; // end of index
  ofile << "\"inequalities\": [ ";
  for ( unsigned int i=0; i<pInequalities.size(); ++i ) { ofile << "\"" << pInequalities[i] << "\", "; } 
  ofile << "\" " <<pInequalities[pInequalities.size()-1] << " \"";
  ofile << " ]\n"; // end of inequalities
  ofile << "},\n"; // end of parameters
// OutEdges information
  ofile << "\"outedges\": {\n";
  ofile << "\"data\": [\n";
  // loop over different parameter
  for ( unsigned int i=0; i<outedges.size(); ++i ) {   
    // get the list of outedges for a given parameter
    std::unordered_map<uint64_t, std::vector<uint64_t> > myoutedges = outedges[i];
    std::unordered_map<uint64_t, std::vector<uint64_t> >::iterator it;
    ofile << "[\n{\n";
    for ( it=myoutedges.begin(); it!=myoutedges.end(); ++it ) { 
      uint64_t id1 = it->first;
      ofile << "\"" << id1 << "\": [";
      std::vector<uint64_t> mylist = it -> second;
      for ( unsigned int j=0; j<mylist.size()-1; ++j ) { 
	ofile << mylist[j] << " ,";
      }
      ofile << mylist[mylist.size()-1];
      if ( std::distance(myoutedges.begin(),it) < myoutedges.size()-1 ) { 
        ofile << " ],\n";
      } else { 
        ofile << " ]\n";
      }
    }
    if ( i < outedges.size()-1 ) {
      ofile << "}],\n"; // end of the parameter field
    } else { 
      ofile << "}]\n";
    }
  } // end loop over different parameter
  ofile <<"]\n},\n"; // end of outedges
// Wall information field  
  ofile << "\"walls\": {\n";
  ofile << "\"info\": {\n";
  std::unordered_map<uint64_t,std::string>::const_iterator itwi;
  for ( itwi=wallInformation.begin(); itwi!=wallInformation.end(); ++itwi ) { 
    ofile << "\"" << itwi->first << "\": " << "\""<< itwi->second;
    if ( std::distance(wallInformation.begin(), itwi) < wallInformation.size()-1 ) { 
      ofile << "\",\n";
    } else { 
      ofile << "\"\n";
    }
  }  
  ofile << "}\n";
  ofile << "},\n"; // end of walls
// Variables information
  ofile << "\"variables\": { \n";
  ofile << "\"info\": [ ";
  for ( unsigned int i=0; i<variables.size()-1; ++i ) { ofile << "\"" << variables[i] << "\", "; }
  ofile << "\"" << variables[variables.size()-1] << "\" ]\n";
  ofile << "}\n"; // end of variables 
  ofile << "}"; // end of file
  ofile . close ( );
}

