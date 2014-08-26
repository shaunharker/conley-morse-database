#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include <vector>
#include <utility>
#include <exception>
#include <sstream>

#include <unordered_map>
#include <unordered_set>

#include "boost/shared_ptr.hpp"

#include "database/structures/RectGeo.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/Grid.h"
#include "database/structures/MorseGraph.h"
#include "database/program/Configuration.h"
#include "database/maps/Map.h"

#include "ModelMap.h"

#include "Phase/BooleanSwitchingMaps.h"
#include "Phase/MultiDimensionalIndices.h"
#include "Phase/Wall.h"
#include "Parameter/BooleanSwitchingParameterSpace.h"


class Model {
public:
  /// initialize
  ///   Given command line arguments, load necessary files 
  ///   required for initializtion.
  void initialize ( int argc, char * argv [] ); 

  /// parameterSpace
  ///   return a shared ptr to the parameter space
  boost::shared_ptr < ParameterSpace > parameterSpace ( void ) const;
  
  /// phaseSpace
  ///   return a shared ptr to the phase space
  boost::shared_ptr < Grid > phaseSpace ( void ) const;

  /// map
  ///   return a shared ptr to a map function object corresponding to 
  ///   parameter p
  boost::shared_ptr < const Map > map ( boost::shared_ptr<Parameter> p ) const;

  /// annotate
  ///   Given a MorseGraph, provide annotations.
  void annotate ( MorseGraph * mg_in ) const;

private:
  Configuration config_;
  boost::shared_ptr < BooleanSwitchingParameterSpace > parameter_space_;
  int phase_space_dimension_;
  MultiDimensionalIndices domains_;
  std::unordered_map<Wall, size_t> walls_;
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
  // Initialize parameter space
  std::cout << "Model::initialize. Initialize parameter space.\n";
  parameter_space_ . reset ( new BooleanSwitchingParameterSpace );
  parameter_space_  -> initialize ( argc, argv );
  // Initialize phase space
  std::cout << "Model::initialize. Initialize phase space.\n";
  domains_ . assign  ( parameter_space_ -> domainLimits () );
  phase_space_dimension_ = parameter_space_ -> dimension ();
  // Loop through domains and create walls and interior point.
  std::cout << "Model::initialize. Build walls.\n";
  size_t num_walls = 0;
  for ( std::vector<size_t> const& domain : domains_ ) {
    for ( int cface = -phase_space_dimension_; 
          cface <= phase_space_dimension_; ++ cface ) {
      Wall wall ( cface, domain );
      if ( walls_ . count ( wall ) == 0 ) {
        walls_ [ wall ] = num_walls ++;
      }
    }
  }   
  std::cout << "Model::initialize. Initialization complete.\n";
}

inline boost::shared_ptr < ParameterSpace > 
Model::parameterSpace ( void ) const {
  return boost::dynamic_pointer_cast<ParameterSpace> ( parameter_space_ );
}

inline boost::shared_ptr < Grid > 
Model::phaseSpace ( void ) const {
  boost::shared_ptr < Atlas > space ( new Atlas );
  typedef std::pair<Wall, size_t> WallIndexPair;
  for ( WallIndexPair const& wall_index_pair : walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int wall_id = wall_index_pair . second;
    RectGeo rect = wall . reducedRect (); 
    space -> add_chart ( wall_id, rect );
  }
  return boost::dynamic_pointer_cast<Grid> ( space );
}

inline boost::shared_ptr < const Map > 
Model::map ( boost::shared_ptr<Parameter> p) const { 
  boost::shared_ptr < ModelMap > atlasmap ( new ModelMap );
  // Loop through domains and add wall maps
#ifdef BS_DEBUG_MODELMAP
  std::ofstream outfile ("map.gv");
  outfile << "digraph G {\n";
  typedef std::pair<Wall, size_t> WallIndexPair;
  for ( WallIndexPair const& wall_index_pair : walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int wall_id = wall_index_pair . second;
    outfile << wall_id << "[label=\"" << wall . rect () << "\"]\n";
  }
  std::unordered_set<uint64_t> mapped_in, mapped_out;
#endif
  for ( std::vector<size_t> const& domain : domains_ ) {
    typedef std::pair<int, int> intPair;
    std::vector < intPair > listofmaps = 
      BooleanSwitchingMaps ( parameter_space_ -> closestFace ( p, domain ) );
    for ( intPair const& cface_pair : listofmaps ) {
      Wall wall1 ( cface_pair . first, domain );
      Wall wall2 ( cface_pair . second, domain );
      int id1 = walls_ . find ( wall1 ) -> second;
      int id2 = walls_ . find ( wall2 ) -> second;
      BooleanChartMap map ( wall1 . reducedRect (), 
                            wall2 . reducedRect () );
      atlasmap -> addMap ( id1, id2, map );
#ifdef BS_DEBUG_MODELMAP
      mapped_out . insert ( id1 );
      mapped_in . insert ( id2 );
      outfile << id1 << " -> " << id2 << "\n";
#endif
    }
  }
#ifdef BS_DEBUG_MODELMAP
  outfile << "}\n\n";
  outfile . close ();
  for ( uint64_t in : mapped_in ) {
    if ( mapped_out . count ( in ) == 0 ) {
      abort ();
    }
  }
#endif
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
