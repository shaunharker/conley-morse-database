/*
 *  Configuration.h
 */


#ifndef _CMDP_CONFIGURATION_
#define _CMDP_CONFIGURATION_

#include <string>
#include <sstream>

#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <unistd.h>
#include <fstream>

/* Map Choice */
#include "ModelMap.h"
typedef ModelMap GeometricMap;

#include "chomp/Rect.h"

class Configuration {
public:
  typedef chomp::Rect Rect;
  // Model, name and description
  std::string MODEL_NAME;
  std::string MODEL_DESC;
  
  /* Parameter Space */
  int PARAM_DIM;
  int PARAM_SUBDIV_DEPTH;
  Rect PARAM_BOUNDS;
  
  /* Phase Space */
  int PHASE_DIM;
  int PHASE_SUBDIV_INIT;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Rect PHASE_BOUNDS; 
  std::vector<bool> PERIODIC;
  
  // Loading
  void loadFromFile ( const char * filename ) {
    std::string filestring ( filename );
    std::string appendstring ( "/config.xml" );
    std::string loadstring = filestring + appendstring;
    char current [ 100 ];
    std::ifstream input(loadstring.c_str());
    
    getcwd ( current, 100 );
    std::cout << "Current directory:\n" << current << "\n";
    std::cout << "Loading from file:\n " << loadstring << "\n";
    LoadFromStream(&input);
    std::cout << "Success.\n";
  }

  void LoadFromString(const std::string& input) {
    std::istringstream stream(input);
    LoadFromStream(&stream);
  }
  
  void LoadFromStream(std::istream* input) {
    
    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;
    
    // Load the XML file into the property tree. If reading fails
    // (cannot open file, parse error), an exception is thrown.
    
    read_xml(*input, pt);
    
    MODEL_NAME = pt.get<std::string>("config.model.name");
    MODEL_DESC = pt.get<std::string>("config.model.desc");
    
    /* Parameter Space */
    PARAM_DIM = pt.get<int>("config.param.dim");
    PARAM_SUBDIV_DEPTH = pt.get<int>("config.param.subdiv.depth");
    
    PARAM_BOUNDS . lower_bounds . resize ( PARAM_DIM );
    PARAM_BOUNDS . upper_bounds . resize ( PARAM_DIM );
    std::string param_lower_bounds = pt.get<std::string>("config.param.bounds.lower");
    std::string param_upper_bounds = pt.get<std::string>("config.param.bounds.upper");
    std::stringstream param_lbss ( param_lower_bounds );
    std::stringstream param_ubss ( param_upper_bounds );
    for ( int d = 0; d < PARAM_DIM; ++ d ) {
      param_lbss >> PARAM_BOUNDS . lower_bounds [ d ];
      param_ubss >> PARAM_BOUNDS . upper_bounds [ d ];
    }
    
    /* Phase Space */
    PHASE_DIM = pt.get<int>("config.phase.dim");
    PHASE_SUBDIV_MIN = pt.get<int>("config.phase.subdiv.min");
    PHASE_SUBDIV_MAX = pt.get<int>("config.phase.subdiv.max");
    PHASE_SUBDIV_LIMIT = pt.get<int>("config.phase.subdiv.limit");
 
    boost::optional<int> opt_phase_subdiv_init = pt.get_optional<int>("config.phase.subdiv.init");
    PHASE_SUBDIV_INIT = 0;
    if ( opt_phase_subdiv_init ) PHASE_SUBDIV_INIT = opt_phase_subdiv_init . get ();
    
    PHASE_BOUNDS . lower_bounds . resize ( PHASE_DIM );
    PHASE_BOUNDS . upper_bounds . resize ( PHASE_DIM );
    std::string phase_lower_bounds = pt.get<std::string>("config.phase.bounds.lower");
    std::string phase_upper_bounds = pt.get<std::string>("config.phase.bounds.upper");
    std::stringstream phase_lbss ( phase_lower_bounds );
    std::stringstream phase_ubss ( phase_upper_bounds );
    for ( int d = 0; d < PHASE_DIM; ++ d ) {
      phase_lbss >> PHASE_BOUNDS . lower_bounds [ d ];
      phase_ubss >> PHASE_BOUNDS . upper_bounds [ d ];
    }
    
    PERIODIC . resize ( PHASE_DIM, false );
    boost::optional<std::string> phase_periodic = pt.get_optional<std::string>("config.phase.periodic");
    if ( phase_periodic ) {
      std::cout << "Found periodic field\n";
      std::stringstream phase_periodic_ss ( *phase_periodic );
      for ( int d = 0; d < PHASE_DIM; ++ d ) {
        int x;
        phase_periodic_ss >> x;
        PERIODIC [ d ] = (bool) x;
      }

    }else {
      std::cout << "Did not find a periodic field.\n";
    }
    
    
  }
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    // Model, name and description
    ar & MODEL_NAME;
    ar & MODEL_DESC;
    
    /* Parameter Space */
    ar & PARAM_DIM;
    ar & PARAM_SUBDIV_DEPTH;
    ar & PARAM_BOUNDS;
    
    /* Phase Space */
    ar & PHASE_DIM;
    ar & PHASE_SUBDIV_INIT;
    ar & PHASE_SUBDIV_MIN;
    ar & PHASE_SUBDIV_MAX;
    ar & PHASE_SUBDIV_LIMIT;
    ar & PHASE_BOUNDS; 
    ar & PERIODIC;
  }
  
};


/* Parameter Space */
//int PARAM_DIMENSION; //2
//int PARAM_SUBDIVISIONS; //6
/* Phase Space */
//int SPACE_DIMENSION;// 2
//int MIN_PHASE_SUBDIVISIONS;// 12
//int MAX_PHASE_SUBDIVISIONS;// 15
//int COMPLEXITY_LIMIT;// 10000
//const Real param_lower_bounds [PARAM_DIMENSION] = { 8.0, 3.0 }; // 8 3
//const Real param_upper_bounds [PARAM_DIMENSION] = { 37.0, 50.0 }; // 37 50
//const Real space_lower_bounds [SPACE_DIMENSION] = { 0.0, 0.0 };
//const Real space_upper_bounds [SPACE_DIMENSION] = { 320.056, 224.040 };

#endif
