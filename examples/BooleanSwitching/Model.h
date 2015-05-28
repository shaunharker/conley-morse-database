#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include <vector>
#include <utility>
#include <exception>
#include <sstream>

#include <unordered_map>
#include <unordered_set>

#include <memory>

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
  std::shared_ptr < ParameterSpace > parameterSpace ( void ) const;
  
  /// phaseSpace
  ///   return a shared ptr to the phase space
  std::shared_ptr < Grid > phaseSpace ( void ) const;

  /// map
  ///   return a shared ptr to a map function object corresponding to 
  ///   parameter p
  std::shared_ptr < const Map > map ( std::shared_ptr<Parameter> p ) const;

  /// annotate
  ///   Given a MorseGraph, provide annotations.
  void annotate ( MorseGraph * mg_in ) const;

  /// Save the walls ( phase space )
  void saveWalls ( const char *filename ) const;
  
  /// Get the wall information for a given id
  std::unordered_map <size_t, Wall >  getWalls ( void ) const;

  /// get the maps between walls
  std::vector< std::pair<int64_t,int64_t> > getWallMaps ( std::shared_ptr<Parameter> p ) const;

private:
  Configuration config_;
  std::shared_ptr < BooleanSwitchingParameterSpace > parameter_space_;
  int64_t phase_space_dimension_;
  MultiDimensionalIndices domains_;
  std::unordered_map<Wall, size_t> walls_;
 
  bool validateMorseGraph ( MorseGraph * mg_in ) const;
 
  std::vector < std::string > constructAnnotationsMorseSet (
                                  const std::shared_ptr<Grid> & phasespace,
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

inline std::shared_ptr < ParameterSpace > 
Model::parameterSpace ( void ) const {
  return std::dynamic_pointer_cast<ParameterSpace> ( parameter_space_ );
}

inline std::shared_ptr < Grid > 
Model::phaseSpace ( void ) const {
  std::shared_ptr < Atlas > space ( new Atlas );
  typedef std::pair<Wall, size_t> WallIndexPair;
  for ( WallIndexPair const& wall_index_pair : walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int64_t wall_id = wall_index_pair . second;
    RectGeo rect = wall . reducedRect (); 
    space -> add_chart ( wall_id, rect );
  }
  space -> finalize ();
  return std::dynamic_pointer_cast<Grid> ( space );
}

inline std::shared_ptr < const Map > 
Model::map ( std::shared_ptr<Parameter> p) const { 
  std::shared_ptr < ModelMap > atlasmap ( new ModelMap );
  // Loop through domains and add wall maps
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
    }
  }
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

inline 
void Model::saveWalls ( const char *filename ) const {
  //
  std::vector < Wall > mywalls;
  mywalls . resize ( walls_.size() );
  std::ofstream ofile ( filename );
  //
  typedef std::pair<Wall, size_t> WallIndexPair;
  for ( WallIndexPair const& wall_index_pair : walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int64_t wall_id = wall_index_pair . second;
    // just to order them (not the best way)
    mywalls[wall_id] = wall;
  }
  // Save the wall information sorted
  for ( unsigned int i=0; i<mywalls.size(); ++i ) {
    ofile << i << " ";
    if ( mywalls[i] . isFixedPoint() ) {
      ofile << "-1 ";
    } else {
      // check which direction is degenerated (should have only one)
      int dir;
      for ( unsigned int j=0; j<mywalls[i].rect().dimension(); ++j ) {
        if ( std::abs(mywalls[i].rect().upper_bounds[j]-mywalls[i].rect().lower_bounds[j] )<1e-12 ) {
          dir = j;
        }
      }
      ofile << dir;
    }
    ofile << " " << mywalls[i].rect() << "\n";
  }
  ofile . close();
}

inline
std::unordered_map <size_t,Wall>  Model::getWalls ( void ) const { 
  std::unordered_map <size_t,Wall> output;
  //
  typedef std::pair<Wall, size_t> WallIndexPair;
  for ( WallIndexPair const& wall_index_pair : walls_ ) {
    const Wall & wall = wall_index_pair . first;
    int64_t wall_id = wall_index_pair . second;
    output[wall_id] = wall;
  }
  return output; 
} 

inline
std::vector< std::pair<int64_t,int64_t> > Model::getWallMaps ( std::shared_ptr<Parameter> p ) const {
  //  similar method to map
  std::vector< std::pair<int64_t,int64_t> > output;
  //
  for ( std::vector<size_t> const& domain : domains_ ) {
    typedef std::pair<int64_t, int64_t> intPair;
    std::vector < intPair > listofmaps = 
      BooleanSwitchingMaps ( parameter_space_ -> closestFace ( p, domain ) );
    for ( intPair const& cface_pair : listofmaps ) {
      Wall wall1 ( cface_pair . first, domain );
      Wall wall2 ( cface_pair . second, domain );
      int64_t id1 = walls_ . find ( wall1 ) -> second;
      int64_t id2 = walls_ . find ( wall2 ) -> second;
      //
      output . push_back ( std::pair<int64_t,int64_t>(id1,id2) );
    }
  }    
  return output;
}


inline bool Model::validateMorseGraph ( MorseGraph * mg_in ) const {
  MorseGraph & mg = *mg_in;
  std::shared_ptr < Grid > atlas = phaseSpace ( );
  // To validate a Morse Graph we need :
  // condition1 && !condition2 && condition3
  bool c1, c2, c3;
  c1 = false;
  c2 = false;
  c3 = false;
  for ( int v = 0; v < mg.NumVertices(); ++ v ) {
    std::shared_ptr<const Grid> my_subgrid ( mg . grid ( v ) );
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
                                    const std::shared_ptr<Grid> & phasespace,
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
    if ( not std::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) ) ) {
      std::cout << "Unexpected null response from geometry\n";
    }
    AtlasGeo geo = * std::dynamic_pointer_cast < AtlasGeo > ( phasespace -> geometry ( ge ) );
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
