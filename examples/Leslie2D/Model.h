#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include "boost/shared_ptr.hpp"
#include "database/ParameterSpace.h"

#include "ModelMap.h"
#include "database/structures/MorseGraph.h"
#include "database/program/Configuration.h"

class Model {
public:
  /// initialize
  ///   Given command line arguments, load necessary files 
  ///   required for initializtion.
  void initialize ( int argc, char * argv [] ); 

  /// parameterSpace
  ///   return a shared ptr to the parameter space
  boost::shared_ptr < ParameterSpace > parameterSpace ( void );
  
  /// phaseSpace
  /// return a shared ptr to the phase space corresponding to
  /// parameter p
  boost::shared_ptr < Grid > phaseSpace ( void );

  /// map
  ///   return a shared ptr to a map function object corresponding to 
  ///   parameter p
  boost::shared_ptr < ModelMap > map ( boost::shared_ptr<Parameter> p );

  /// annotate
  ///   Given a MorseGraph, provide annotations.
  void annotate ( MorseGraph * mg_in ) const;

private:
  Configuration config_;
  boost::shared_ptr < ParameterSpace > parameter_space_;
public:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & config_;
  }
};

inline void 
Model::initialize ( int argc, char * argv [] ) { 
  config_ . loadFromFile ( argv[1] );
}

inline boost::shared_ptr < ParameterSpace > 
Model::parameterSpace ( void ) {
  if ( not parameter_space_ ) {
    boost::shared_ptr<Grid> parameter_grid ( new PARAMETER_GRID );
    parameter_space_ = boost::shared_ptr<ParameterSpace> 
      ( new EuclideanParameterSpace );
    boost::dynamic_pointer_cast<EuclideanParameterSpace> ( parameter_space_ ) 
      -> initialize ( config, parameter_grid );
  }
  return parameter_space_;
}

inline boost::shared_ptr < Grid > 
Model::phaseSpace ( boost::shared_ptr<Parameter> p ) {
  boost::shared_ptr < TreeGrid > space ( new PHASE_GRID );
  space -> initialize ( config_.PHASE_BOUNDS, config_.PHASE_PERIODIC );
  return boost::dynamic_pointer_cast<Grid> ( space );
}

inline boost::shared_ptr < ModelMap > 
Model::map ( boost::shared_ptr<Parameter> p) { 
  return boost::shared_ptr < ModelMap > ( new ModelMap ( p ) );
}

inline void 
Model::annotate( MorseGraph * mg_in ) const {
/*
  MorseGraph & mg = *mg_in;
  mg . annotation () . insert ( "annotation_A" );
  mg . annotation () . insert ( "annotation_B" );
  for ( int v = 0; v < mg.NumVertices(); ++ v ) {
    mg . annotation ( v ) . insert ( std::string ( "annotation_C" ) );
  }
*/
}

#endif
