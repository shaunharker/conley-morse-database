#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include "boost/shared_ptr.hpp"
#include "database/ParameterSpace.h"
#include "database/structures/MorseGraph.h"
#include "database/program/Configuration.h"
#include "database/maps/AtlasMap.h"
#include "ConstructBooleanMaps.h"
#include "BooleanMap.h"
#include "MultiDimensionalIndices.h"
#include "LookUpTable.h"
#include "BooleanSwitchingClasses.h"
#include "Wall.h"

typedef AtlasMap<BooleanChartMap> ModelMap;

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
  ///   return a shared ptr to the phase space corresponding to
  ///   parameter p
  boost::shared_ptr < Grid > phaseSpace ( boost::shared_ptr<Parameter> p );

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
  int phase_space_dimension_;
  MultiDimensionalIndices domains_;
  boost::unordered_map<Wall, size_t> walls_;
  LUT lut_;
public:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & config_;
    // TODO: complete serialization
  }
};

inline void 
Model::initialize ( int argc, char * argv [] ) { 
  config_ . loadFromFile ( argv[1] );
  // Construct Lookup Table
  lut_ = constructLookUpTable ( dimension );
  // Initialize parameter space
  parameter_space_ = boost::shared_ptr<ParameterSpace> 
    ( new BooleanSwitchingParameterSpace );
  boost::dynamic_pointer_cast<BooleanSwitchingParameterSpace> 
    ( parameter_space_ ) -> initialize ( argc, argv );
  // Initialize phase space
  phase_space_dimension_ = parametergraph . network ( ) . threshold_count . size ( ); // FIX
  domains_ . assign  ( parametergraph . network ( ) . threshold_count );
  // Loop through domains and create walls and interior point.
  size_t num_walls = 0;
  BOOST_FOREACH ( const std::vector<size_t> & domain, domains ) {
    for ( CFace cface = -phase_space_dimension_; 
          cface <= phase_space_dimension_; ++ cface ) {
      Wall wall ( cface, domain );
      if ( walls_ . count ( wall ) == 0 ) {
        walls_ [ wall ] = num_walls ++;
      }
    }
  }   
}

inline boost::shared_ptr < ParameterSpace > 
Model::parameterSpace ( void ) {
  return parameter_space_;
}

inline boost::shared_ptr < Grid > 
Model::phaseSpace ( boost::shared_ptr<Parameter> p ) {
  boost::shared_ptr < Atlas > space ( new Atlas );
  typedef std::pair<Wall, size_t> WallIndexPair;
  BOOST_FOREACH ( const WallIndexPair & wall_index_pair, walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int wall_id = wall_index_pair . second;
    RectGeo rect = wall . reducedRect (); 
    space -> add_chart ( wall_id, rect );
  }
  return boost::dynamic_pointer_cast<Grid> ( space );
}

inline boost::shared_ptr < ModelMap > 
Model::map ( boost::shared_ptr<Parameter> p) { 
  boost::shared_ptr < ModelMap > atlasmap ( new ModelMap );
  // Loop through domains and add wall maps
  BOOST_FOREACH ( const std::vector<size_t> & domain, domains_ ) {
    std::vector < std::pair < CFace, CFace > > listofmaps = 
      BooleanPairFacesMaps ( domain,
                             p.closestFace(domain), 
                             lut_,
                             domains_ . limits () );
    typedef std::pair<CFace, CFace> WallPair;
    BOOST_FOREACH ( const WallPair & wall_pair, listofmaps ) {
      Wall wall1 ( wall_pair . first, domain );
      Wall wall2 ( wall_pair . second, domain );
      int id1 = walls_ [ wall1 ];
      int id2 = walls_ [ wall2 ];
      BooleanChartMap map ( wall1 . rect (), wall2 . rect () );
      atlasmap -> addMap ( id1, id2, map );
    }
  }
  return atlasmap;
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
