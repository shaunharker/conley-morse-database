/*
 *  Clutching_Graph_Job.hpp
 */

#include <boost/foreach.hpp>
#include <algorithm>
#include <stack>
#include <vector>
#include <ctime>
#include <set>
#include "boost/iterator_adaptors.hpp"
#include "boost/iterator/counting_iterator.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/set.hpp"

#include "database/program/Configuration.h"
#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "database/structures/Database.h"
#include "database/structures/Tree.h"

#include "database/algorithms/clutching.h"

#include "chomp/Rect.h"



/** Main function for clutching graph job.
 *
 *  This function is called from worker, and compare graph structure
 *  for each two adjacent boxes.
 */
template <class PhaseGrid>
void Clutching_Graph_Job ( Message * result , const Message & job ) {
  
  // Read Job Message

// ATLAS JOB BEGIN #1
  size_t job_number;
  std::vector < Grid::GridElement> box_names;
  std::vector < RectGeo > box_geometries;
  std::vector <std::pair<Grid::GridElement, Grid::GridElement> > box_adjacencies;
  int PHASE_SUBDIV_INIT;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Model model;
  //chomp::Rect PHASE_BOUNDS;  // HERE
  //std::vector < bool > PHASE_PERIODIC; // HERE
  job >> job_number;
  job >> box_names;
  job >> box_geometries;
  job >> box_adjacencies;
  job >> PHASE_SUBDIV_INIT;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  job >> model;
  //job >> PHASE_BOUNDS;     // HERE
  //job >> PHASE_PERIODIC;   // HERE
// ATLAS JOB END #1
  // Prepare data structures
  std::map < Grid::GridElement, boost::shared_ptr<PhaseGrid> > phase_space_grids;
  std::map < Grid::GridElement, MorseGraph> conley_morse_graphs;
  std::vector < BG_Data > clutching_graphs;

  // Compute Morse Graphs
  std::cout << "CLUTCHING JOB " << job_number << ": " << box_names . size () << " BEGINNING.\n";
  std::cout << "--------- 1. Compute Morse Graphs --------- " << job_number << "\n";
  for ( size_t i = 0; i < box_names . size (); ++ i ) {
    //std::cout << " Processing parameter box " << i << "/" << box_names . size () << "\n";
    //Prepare phase space and map
    Grid::GridElement box = box_names [ i ];
    // ATLAS JOB BEGIN #2
    phase_space_grids [ box ] = boost::dynamic_pointer_cast < PhaseGrid > 
      ( model . phaseSpace ( box_geometries [ i ] ) );
    if ( not phase_space_grids [ box ] ) {
      std::cout << "PHASE SPACE incorrectly chosen in makefile.\n";
    }
    // DEBUG
    //std::cout << "Parameter = " << box_geometries [ i ] << ", box = " << box << "\n";
    // END DEBUG
    boost::shared_ptr<GeometricMap> map = model . map ( box_geometries [ i ] );
    // ATLAS JOB END #2
    // perform computation
    Compute_Morse_Graph 
    ( & conley_morse_graphs  [ box ],
      phase_space_grids [ box ], 
      *map, 
      PHASE_SUBDIV_INIT,
      PHASE_SUBDIV_MIN, 
      PHASE_SUBDIV_MAX, 
      PHASE_SUBDIV_LIMIT);
    //std::cout << "Morse Graph computed.\n";
    if ( conley_morse_graphs [ box ] . NumVertices () == 0 )  { 
      std::cerr << "WARNING. Box # " << box << " Clutching Job #" << job_number 
      << ", box = " << box_geometries [ i ] << " had no morse sets.\n"; 
    }
  }
  
  // Compute Clutching Graphs
  std::cout << "--------- 2. Compute Clutching Graphs --------- " << job_number << "\n";
  typedef std::pair < Grid::GridElement, Grid::GridElement > Adjacency;
  BOOST_FOREACH ( const Adjacency & A, box_adjacencies ) {
    // Debug
    if ( conley_morse_graphs . count ( A . first ) == 0  ||
         conley_morse_graphs . count ( A . second ) == 0 ) {
      std::cout << "Clutching Job ordered that is not within patch.\n";
      abort (); // or continue?
    }
    // End Debug
    clutching_graphs . push_back ( BG_Data () );
    Clutching ( & clutching_graphs . back (),
                conley_morse_graphs [ A . first ],
                conley_morse_graphs [ A . second ]);
  }
  
  // Create Database
  std::cout << "--------- 3. Creating Database --------- " << job_number << "\n";

  Database database;
  
  // Create Parameter Box Records
  typedef std::pair < Grid::GridElement, MorseGraph > indexed_cmg_t;
  BOOST_FOREACH ( const indexed_cmg_t & cmg, conley_morse_graphs ) {
    database . insert ( cmg . first, DAG_Data ( cmg . second ) );
  }
  // Create Clutching Records
  for ( uint64_t i = 0; i < box_adjacencies . size (); ++ i ) {
    database . insert ( box_adjacencies [ i ] . first, box_adjacencies [ i ] . second, clutching_graphs [ i ] );
  }
  
  // Return Result
  std::cout << "CLUTCHING JOB " << job_number << ": " << box_names . size () << " COMPLETE.\n";

  *result << job_number;
  *result << database;
}
