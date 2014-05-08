#ifndef NETWORKIO_H
#define NETWORKIO_H

#include <fstream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/unordered_map.hpp>

#include "Network.h"

//=============
// Declarations
//=============

// Load a network from an .xml file
Network loadNetwork ( const char * inputfile );
// Save the network into a Graphviz file
void saveNetworkGraphviz ( const Network & network );
// Print on the screen the network information
void printNetwork ( const Network & network );


//============
// Definitions
//============

Network loadNetwork ( const char * inputfile ) { 
	using boost::property_tree::ptree;
  ptree pt;
  std::ifstream input ( inputfile );
  read_xml(input, pt);
//
  Network network;
  network.numVariables = pt.get<int>("network.numberofnodes");
// we don't know if the list of nodes in network.wml will be 
// in the right order. So we are using a map to store the information
// where the key is the node id
// unordered_map could be changed to map
  boost::unordered_map < int, std::vector < int > > networkInputs;
  boost::unordered_map < int, std::vector < bool > > networkRegulations;
  boost::unordered_map < int, int > networkThresholds;

	BOOST_FOREACH ( ptree::value_type & v, pt.get_child("network.listofnodes") ) {

		// extract the node id
		int id;
		std::string idstr = v . second . get_child ( "id" ) . data ( );
		std::stringstream idss ( idstr );
		idss >> id;
		// should check that id is not already defined
		if ( networkInputs . find ( id ) !=  networkInputs . end ( ) ) {
			std::cout << "Fatal Error : two nodes have the same id\n";
			abort();
		}

		// extract the in-edges
		int numInEdges;
		std::string numInEdgestr = v . second . get_child ( "numinedges" ) . data ( );
		std::stringstream numInEdgess ( numInEdgestr );
		numInEdgess >> numInEdges;

		std::string inEdgestr = v . second . get_child ( "inedgefrom" ) . data ( );
		std::stringstream inEdgess ( inEdgestr );
		std::vector < int > myInputs;
		myInputs . assign ( numInEdges, 0 );
		for ( int i=0; i<numInEdges; ++i ) {
			inEdgess >> myInputs [ i ];
		}
		networkInputs [ id ] = myInputs;

		// extract the regulations of the in-edges
		std::string regulationstr = v . second . get_child ( "regulations" ) . data ( );
		std::stringstream regulationss ( regulationstr );
		std::vector < bool > myregulations;
		myregulations . assign ( numInEdges, 0 );
		for ( int i=0; i<numInEdges; ++i ) {
			int value;
			regulationss >> value;
			if ( value==0 ) { 
				myregulations [ i ]=false;
			} else {
				myregulations [ i ]=true;
			}
		}
		networkRegulations [ id ] = myregulations;

		// Extract the number of thresholds
		int numthreshold;
		std::string numthresholdstr = v . second . get_child ( "numthresholds" ) . data ( );
		std::stringstream numthresholdss ( numthresholdstr );
		numthresholdss >> numthreshold;
		networkThresholds [ id ] = numthreshold;
  }


  // check the sizes
  // otherwise some variables won't have an equation 
  if ( network.numVariables != networkInputs.size() ) {
  	std::cout << network.numVariables << "  " << networkInputs.size() <<"\n";
  	std::cout << "Fatal Error in xml file : number of nodes != number of nodes listed\n";
  	abort();
  }

// Go through the data and construct the object network
  // we need to construct network.outputs
	std::vector < std::vector < int > > networkOutputs;
	boost::unordered_map < int, std::vector < int > >::iterator itinputs;
	
	// will store temporaily the out-edges of each node
	// we use a set so we can search for a node quickly
	boost::unordered_map < int, std::set < int > > outputs;
	boost::unordered_map < int, std::set < int > >::iterator itoutputs;
	std::vector < int >::iterator itvi;

	for ( int i=0; i<network.numVariables; ++i ) {
		// store the outputs for node i
		// go through the in edges and count the outer-edges of node i
		for ( itinputs=networkInputs.begin(); 
					itinputs!=networkInputs.end();
					++itinputs ) {
			// extract the in-edges for node
			std::vector < int > inEdges = itinputs -> second;
			for ( itvi=inEdges.begin(); itvi!=inEdges.end(); ++itvi )  {
				itoutputs = outputs . find ( *itvi );
				if ( itoutputs!=outputs.end() ) {
					itoutputs -> second . insert ( itinputs -> first );
				} else { 
					std::set < int > newset;
					newset . insert ( itinputs->first );
					outputs [ *itvi ] = newset; 
				}
			}
		}
	}

	for ( int i=0; i<network.numVariables; ++i ) { 
		// construct the network
		network . inputs . push_back ( networkInputs [ i ] );
		network . regulation . push_back ( networkRegulations [ i ] );
		network . threshold_count . push_back ( networkThresholds [ i ] );
		// here convert the set to a vector
		std::vector < int > voutputs;
		std::set < int > tmpoutputs = outputs[i];
		for ( std::set<int>::iterator it=tmpoutputs.begin();
				  it!=tmpoutputs.end(); ++it ) {
			voutputs . push_back ( *it );
		}
		network . outputs . push_back ( voutputs );
	}

	return network;
}

void saveNetworkGraphviz ( const Network & network ) { 
	std::ofstream file;
	file . open ( "network.gv" );
	file << "digraph {\n";
	for ( int j=0; j<network.inputs.size(); ++j ) {
		for ( int k=0; k<network.inputs[j].size(); ++k ) {
			if ( network.regulation[j][k] == false ) {
				file << network.inputs[j][k] << " -> " << j;
				file << " [arrowhead=tee]\n";
			} else {
				file << network.inputs[j][k] << " -> " << j << "\n";
			}
		}
	}
	file << "}";
}

void printNetwork ( const Network & network ) {

	std::cout << "=========\n";
	std::cout << "Network : \n";
	std::cout << "Number of variables : " << network.numVariables << "\n";
	for ( int i=0; i<network.inputs.size(); ++i ) {
		std::cout << "------------\n";
		std::cout << "Variable : " << i << "\n";
		std::cout << "Number of thresholds : " << network.threshold_count[i] <<"\n";
		std::cout << "Inputs : \n";
		for ( int j=0; j<network.inputs[i].size(); ++j ) {
			std::cout << network.inputs[i][j] << " ";
		}
		std::cout << "\n";
		std::cout << "Regulation : \n";
		for ( int j=0; j<network.regulation[i].size(); ++j ) {
			std::cout << network.regulation[i][j] << " ";
		}
		std::cout << "\n";
		std::cout << "Outputs : \n";
		for ( int j=0; j<network.outputs[i].size(); ++j ) {
			std::cout << network.outputs[i][j] << " ";
		}
		std::cout << "\n";
	}
}

#endif