/*
 *  Clutching_Graph_Job.h
 */

#ifndef _CMDP_CLUTCHING_GRAPH_JOB_
#define _CMDP_CLUTCHING_GRAPH_JOB_

#include "delegator/Message.h"
#include "Model.h"

void Clutching_Graph_Job ( Message * result, 
                           const Message & job,
                           const Model & model ); 

/////////////////
// Definitions //
/////////////////

#include <boost/foreach.hpp>
#include <exception>
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
#include "database/algorithms/clutching.h"
#include "database/maps/Map.h"

/** Main function for clutching graph job.
 *
 *  This function is called from worker, and compare graph structure
 *  for each two adjacent boxes.
 */
inline void 
Clutching_Graph_Job ( Message * result, 
                      const Message & job,
                      const Model & model ) {
  
  // Read Job Message
  std::shared_ptr<ParameterPatch> patch;
  int PHASE_SUBDIV_INIT;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  
  std::cout << "Clutching_Graph_Job. About to read patch and phase space info.\n";
  job >> patch;
  std::cout << "Clutching_Graph_Job. About to read phase space info.\n";
  job >> PHASE_SUBDIV_INIT;
  job >> PHASE_SUBDIV_MIN;
  job >> PHASE_SUBDIV_MAX;
  job >> PHASE_SUBDIV_LIMIT;
  std::cout << "Clutching_Graph_Job. About to do computation.\n";

  // Prepare data structures
  Database database;
  boost::unordered_map < uint64_t, MorseGraph> morse_graphs;

  // Compute Morse Graphs
  size_t num_parameters = patch -> vertices . size ();
  std::cout << "Clutching_Graph_Job. Starting analysis of " << num_parameters << " parameter boxes.\n";
  std::cout << "--------- 1. Compute Morse Graphs --------- " << "\n";

  size_t count = 0;
  BOOST_FOREACH ( uint64_t vertex, patch -> vertices ) {
    // Obtain parameter associated with vertex
    std::shared_ptr<Parameter> parameter = patch -> parameter [ vertex ];
    
    // Debug output
    std::cout << "Clutching_Graph_Job. Processing parameter " << *parameter 
              << ", which is " << ++count << "/" << num_parameters << ".\n "; 

    // Prepare dynamical map
    std::shared_ptr<const Map> map = model . map ( parameter );
    if ( not map ) {
      std::cout << "Clutching_Graph_Job. No map associated with parameter " <<
        *parameter << "; continuing.\n";
      continue;
    }
    // Prepare phase space
    std::shared_ptr<Grid> phase_space = model . phaseSpace ();    
    if ( not phase_space ) {
      throw std::logic_error ( "Clutching_Graph_Job. model.phaseSpace() failed" 
                               " to return a valid pointer.\n");
    }

    // Perform Morse Graph computation
    Compute_Morse_Graph 
    ( & morse_graphs [ vertex ],
      phase_space, 
      map, 
      PHASE_SUBDIV_INIT,
      PHASE_SUBDIV_MIN, 
      PHASE_SUBDIV_MAX, 
      PHASE_SUBDIV_LIMIT );

    std::cout << "Clutching_Graph_Job. Successfully computed " 
      << "Morse Graph for parameter " << *parameter << ".\n";

    // Check for warnings
    if ( morse_graphs [ vertex ] . NumVertices () == 0 )  { 
      std::cerr << "Clutching_Graph_Job. WARNING. Vertex # " << vertex << ", parameter = " 
      << *parameter << " yielded no morse sets.\n"; 
    }

    // Annotate the morse graph
    std::cout << "Clutching_Graph_Job. Annotating " 
      << "Morse Graph for parameter " << *parameter << ".\n";
    model . annotate ( & morse_graphs [ vertex ] );

    // Insert Morse graph into database
    std::cout << "Clutching_Graph_Job. Inserting " 
      << "Morse Graph for parameter " << *parameter << " into local database.\n";
    database . insert ( vertex, morse_graphs [ vertex ] );
  }
  
  // Compute Clutching Graphs
  std::cout << "--------- 2. Compute Clutching Graphs --------- " << "\n";
  typedef std::pair < uint64_t, uint64_t > Adjacency;
  BOOST_FOREACH ( const Adjacency & A, patch -> edges ) {
    uint64_t u = A . first;
    uint64_t v = A . second;
    // If adjacency between uncomputed Morse sets, continue.
    if ( morse_graphs . count ( u ) == 0  ||
         morse_graphs . count ( v ) == 0 ) {
      continue;
    }
    // Compute clutching graph
    BG_Data clutching_graph;
    Clutching ( & clutching_graph,
                morse_graphs [ u ],
                morse_graphs [ v ]);

    // Insert clutching graph into database
    database . insert ( u, v, clutching_graph );
  }
  
  // Return Result
  std::cout << "CLUTCHING JOB with " << num_parameters << " parameters COMPLETE.\n";
  *result << database;
}
#endif
