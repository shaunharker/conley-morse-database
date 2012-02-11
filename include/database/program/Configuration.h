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

/* Map Choice */
#include "ModelMap.h"
typedef ModelMap GeometricMap;

#include "chomp/Rect.h"

class Configuration {
public:
  // Model, name and description
  std::string MODEL_NAME;
  std::string MODEL_DESC;
  
  /* Parameter Space */
  int PARAM_DIM;
  int PARAM_SUBDIV_DEPTH;
  Rect PARAM_BOUNDS;
  
  /* Phase Space */
  int PHASE_DIM;
  int PHASE_SUBDIV_MIN;
  int PHASE_SUBDIV_MAX;
  int PHASE_SUBDIV_LIMIT;
  Rect PHASE_BOUNDS; 
  
  // Loading
  void loadFromFile ( const char * filename ) {
    std::string filestring ( filename );
    std::string appendstring ( "/config.xml" );
    std::string loadstring = filestring + appendstring;
    
    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;
    
    // Load the XML file into the property tree. If reading fails
    // (cannot open file, parse error), an exception is thrown.
    char current [ 100 ];
    getcwd ( current, 100 );
    std::cout << "Current directory:\n" << current << "\n";
    std::cout << "Loading from file:\n " << loadstring << "\n";
    read_xml(loadstring, pt);
    std::cout << "Success.\n";

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
    ar & PHASE_SUBDIV_MIN;
    ar & PHASE_SUBDIV_MAX;
    ar & PHASE_SUBDIV_LIMIT;
    ar & PHASE_BOUNDS; 
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
