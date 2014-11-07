#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include <vector>
#include <utility>
#include <exception>
#include <sstream>

#include <unordered_map>
#include <unordered_set>

#include "boost/shared_ptr.hpp"

#include "database/structures/Atlas.h"

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

// Header file containing the condition string for annotations
#include "AnnotationConditions.h"


class Model {
  typedef std::vector<Grid::GridElement> CellContainer;
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
  int64_t phase_space_dimension_;
  MultiDimensionalIndices domains_;
  std::unordered_map<Wall, size_t> walls_;
 
  bool validateMorseGraph ( MorseGraph * mg_in ) const;
 
  std::vector < std::string > constructAnnotationsMorseSet (
                                  const boost::shared_ptr<Grid> & phasespace,
                                  const CellContainer & subset ) const;

  bool hasAllStatesOn ( const Wall & wall ) const;
  bool hasAllStatesOff ( const Wall & wall ) const;
  
  // Correspondence between the chart id and the wall
  // restriction : we cannot subdivide the charts
  boost::unordered_map<size_t, Wall> chartIdToWall_;     
    
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
    for ( int64_t cface = -phase_space_dimension_; 
          cface <= phase_space_dimension_; ++ cface ) {
      Wall wall ( cface, domain );
      if ( walls_ . count ( wall ) == 0 ) {
        // needed to be able to check conditions on morse sets
        chartIdToWall_ [ num_walls ] = wall;
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
    int64_t wall_id = wall_index_pair . second;
    RectGeo rect = wall . reducedRect (); 
    space -> add_chart ( wall_id, rect );
  }
  space -> finalize ();
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
    int64_t wall_id = wall_index_pair . second;
    outfile << wall_id << "[label=\"" << wall_id << " &#92;n " << wall . rect () << "\"]\n";
  }
  std::unordered_set<uint64_t> mapped_in, mapped_out;
#endif
  for ( std::vector<size_t> const& domain : domains_ ) {
    typedef std::pair<int64_t, int64_t> intPair;
    std::vector < intPair > listofmaps = 
      BooleanSwitchingMaps ( parameter_space_ -> closestFace ( p, domain ) );
    for ( intPair const& cface_pair : listofmaps ) {
      Wall wall1 ( cface_pair . first, domain );
      Wall wall2 ( cface_pair . second, domain );
      int64_t id1 = walls_ . find ( wall1 ) -> second;
      int64_t id2 = walls_ . find ( wall2 ) -> second;
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
  MorseGraph & mg = *mg_in;
  
  if ( validateMorseGraph ( & mg ) ) {
      mg . annotation () . insert ( "GOOD" );
  } else {
      mg . annotation () . insert ( "BAD" );
  }
}

inline bool Model::validateMorseGraph ( MorseGraph * mg_in ) const {
  MorseGraph & mg = *mg_in;
  boost::shared_ptr < Grid > atlas = phaseSpace ( );
  // To validate a Morse Graph we need :
  // condition1 && !condition2 && condition3
  bool c1, c2, c3;
  c1 = false;
  c2 = false;
  c3 = false;
  for ( int v = 0; v < mg.NumVertices(); ++ v ) {
    boost::shared_ptr<const Grid> my_subgrid ( mg . grid ( v ) );
    if ( not my_subgrid ) {
      std::cout << "Abort! This vertex does not have an associated grid!\n";
      abort ();
    }
    CellContainer my_subset = atlas -> subset ( * my_subgrid );
    // construct the annotation of the morse set
    std::vector < std::string > vertexAnnotation =
    constructAnnotationsMorseSet ( atlas, my_subset );
    // Check the annotations to know which conditions are satisfied
    for ( unsigned int i=0; i<vertexAnnotation.size(); ++i ) {
      // annotate the vertex of the morsegraph
      mg . annotation ( v ) . insert ( vertexAnnotation[i] );

      if ( vertexAnnotation[i] == CONDITION1STRING ) { c1 = true; }
      if ( vertexAnnotation[i] == CONDITION2STRING ) { c2 = true; }
      if ( vertexAnnotation[i] == CONDITION3STRING ) { c3 = true; }
    }
  }
  // we want a morse graph with
  // 1) a fixed point with all states off
  // 2) no fixed point with all states on
  // 3) a morse set where every variable makes a transition
  return (c1 && !c2 && c3);
}

inline
std::vector < std::string > Model::constructAnnotationsMorseSet (
                                    const boost::shared_ptr<Grid> & phasespace,
                                    const CellContainer & subset ) const {
  std::vector < std::string > annotation;
  bool condition0, condition1, condition2, condition3, condition4;
  condition0 = false;
  condition1 = false;
  condition2 = false;
  condition3 = false;
  condition4 = false;
  // to keep track of the variables making a transition
  std::set < int > wallVariables;
  // Loop through the grid elements of the morse set
  BOOST_FOREACH ( Grid::GridElement ge, subset ) {
    if ( not boost::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) ) ) {
      std::cout << "Unexpected null response from geometry\n";
    }
    AtlasGeo geo = * boost::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) );
    size_t id = geo . id ( );      
    Wall wall = chartIdToWall_ . find ( id ) -> second;
    if ( wall . isFixedPoint() ) {
      if ( hasAllStatesOff ( wall ) ) { condition1 = true; }
      if ( hasAllStatesOn ( wall ) ) { condition2 = true; }
      if ( !condition1 && !condition2 ) { condition0 = true; }
    } else {
      condition4 = true;
      RectGeo box = wall . rect();
      // check which dimension is degenerated
      bool checkstatus;
      checkstatus = false;
      for ( unsigned int i=0; i<box.dimension(); ++i ) {
        if ( std::abs(box.upper_bounds[i]-box.lower_bounds[i]) < 1e-12 ) {
          wallVariables . insert ( i );
        }
      }
      if ( wallVariables . size() == wall.rect().dimension() ) {
        condition3 = true;
      }
    }
  }
  //
  if ( condition0 ) {
    annotation . push_back ( CONDITION0STRING );
  }
  if ( !condition0 && !condition1 && !condition2  && !condition3 ) {
//    std::cout << "NOT A FULL CYCLE : " << wallVariables.size() << "\n";
    std::string str;
    str = "";
    std::set < int >::iterator it;
    for ( it=wallVariables.begin(); it!=wallVariables.end(); ++it ) {
      std::stringstream ss;
      ss << *it;
      str += ss.str() + " ";
//      std::cout << "MYSTRING = " << *it <<"\n";
    }
//    std::cout << "STRING =" << str << "\n";
    std::string conditionstring(CONDITION4STRING);
    str = conditionstring + ":" + str;
    annotation . push_back ( str );
  }
  if ( condition1 ) {
    annotation . push_back ( CONDITION1STRING );
  }
  if ( condition2 ) {
    annotation . push_back ( CONDITION2STRING );
  }
  if ( condition3 ) {
    annotation . push_back ( CONDITION3STRING );
#ifdef BS_DEBUG_MODELMAP
    std::ofstream ofile;
    ofile . open ( "morseset.txt" );
    BOOST_FOREACH ( Grid::GridElement ge, subset ) {
      if ( not boost::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) ) ) {
        std::cout << "Unexpected null response from geometry\n";
      }
      AtlasGeo geo = * boost::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) );
      size_t id = geo . id ( );
      Wall wall = chartIdToWall_ . find ( id ) -> second;
      ofile << id << " " << wall.rect() << "\n";
    }
    ofile . close ();
#endif
  }
  return annotation;
}

inline
bool Model::hasAllStatesOn ( const Wall & wall ) const {
  bool AllStatesOn = true;
  //
  const std::vector < double > & lbounds = wall . rect() . lower_bounds;
  const std::vector < double > & ubounds = wall . rect() . upper_bounds;
  int dim = lbounds . size();
  //
  // for the cube [a_i,b_i]^n
  // need only to check if b_i is not less than 1.0
  for ( unsigned int i=0; i<dim; ++i ) {
    if ( ubounds[i] <= 1.0 + 1e-10 ) {
      AllStatesOn = false;
    }
  }
  return AllStatesOn;
}

inline
bool Model::hasAllStatesOff ( const Wall & wall ) const {
  bool AllStatesOff = true;
  //
  const std::vector < double > & lbounds = wall . rect() . lower_bounds;
  const std::vector < double > & ubounds = wall . rect() . upper_bounds;
  int dim = lbounds . size();
  //
  // for the cube [a_i,b_i]^n
  // need only to check if b_i is not greater than 1.0
  for ( unsigned int i=0; i<dim; ++i ) {
      if ( lbounds[i] >= 1.0 - 1e-10 ) {
          AllStatesOff = false;
      }
  }
  return AllStatesOff;
}

#endif
