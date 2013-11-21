#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include "ModelMap.h"
#include "database/program/Configuration.h"

class Model {
 public:
  typedef RectGeo Parameter;

  void initialize ( int argc, char * argv [] ); 
    // Load stuff from a file necessary for the methods
    // "map" and "phaseSpace" below to work properly.

  boost::shared_ptr < ModelMap > map ( const Parameter & p );
  // return a shared ptr to a map function object corresponding to 
  // parameter p

  boost::shared_ptr < Grid > phaseSpace ( const Parameter & p );
  // return a shared ptr to the phase space corresponding to
  // parameter p


private:
  Configuration config_;
public:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & config_;
  }
};

inline void Model::initialize ( int argc, char * argv [] ) { 
  config_ . loadFromFile ( argv[1] );
}

inline boost::shared_ptr < ModelMap > Model::map ( const Parameter & param ) { 
  return boost::shared_ptr < ModelMap > ( new ModelMap ( param ) );
}

inline boost::shared_ptr < Grid > Model::phaseSpace ( const Parameter & p ) {
  boost::shared_ptr < TreeGrid > space ( new PHASE_GRID );
  space -> initialize ( config_.PHASE_BOUNDS, config_.PHASE_PERIODIC );
  return boost::dynamic_pointer_cast<Grid> ( space );
}


#endif
