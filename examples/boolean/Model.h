#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

class Model {
 public:
  typedef RectGeo Parameter;

  void initialize ( int argc, char * argv [] ) {
    // Load stuff from a file necessary for the methods
    // "map" and "phaseSpace" below to work properly.
  } 
  boost::shared_ptr<ModelMap> map ( const Parameter & p ) const {
    // return a shared ptr to a map function object corresponding to 
    // parameter p
  }
  boost::shared_ptr<Grid> phaseSpace ( const Parameter & p ) const {
    // return a shared ptr to the phase space Atlas corresponding to
    // parameter p
  }
};
#endif
