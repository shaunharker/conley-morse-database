#ifndef CMDB_EUCLIDEANPARAMETERSPACE
#define CMDB_EUCLIDEANPARAMETERSPACE

#include "database/structures/Grid.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/RectGeo.h"

#include "boost/shared_ptr.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/unordered_map.hpp"
#include "boost/serialization/shared_ptr.hpp"
#include "boost/serialization/export.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class EuclideanParameter : public Parameter {
public:
	boost::shared_ptr<RectGeo> geo;
  EuclideanParameter ( void ) {}
	EuclideanParameter ( boost::shared_ptr<RectGeo> geo ) : geo(geo) {}
private:
	// Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<Parameter>(*this);
  	ar & geo;
  }
  /// derivation interface
  virtual void print ( std::ostream & outstream ) const {
  	outstream << * geo;
  }
};

BOOST_CLASS_EXPORT_KEY(EuclideanParameter);

class EuclideanParameterSpace : public ParameterSpace {
public:

	/// initialize
	///    Create the ParameterSpace given the configuration specified
  virtual void initialize ( const Configuration & config );

	void initialize ( const Configuration & config,
                    boost::shared_ptr<Grid> parameter_grid  );

	/// adjacencies
	///    Return a vector of adjacent vertices.
	virtual std::vector<uint64_t> adjacencies ( uint64_t v ) const;
	
	/// size
	///    Return the number of vertices
	virtual uint64_t size ( void ) const;

	/// parameter
	///    Return the parameter object associated with a vertex
	virtual boost::shared_ptr<Parameter> parameter ( uint64_t v ) const;
	
  /// search
  ///    Given a parameter, find the vertex associated with it
  ///    (This can be used to find a parameter which might contain the other)
  virtual uint64_t search ( boost::shared_ptr<Parameter> parameter ) const;
  
	/// patch
	///    Return a "ParameterPatch" object
	///    A sequence of calls to this function will return a sequence
	///    of patches that cover the entire parameter space.
	///    To do this the function must use mutable data (and so is not thread-safe)
	///    even though it is declared const.
	///    One the sequence is completed an empty patch is returned, and then the
	///    sequence will restart.
	///    The default implementation returns patches that consist of two vertices
	///    and the edge between them.
	virtual boost::shared_ptr<ParameterPatch> patch ( void ) const;


private:
	boost::shared_ptr<Grid> parameter_grid_;
  RectGeo bounds_;
  std::vector<bool> periodic_;
  int dimension_;

// EuclideanPatch construction variables
	mutable int patch_width_; 
  mutable std::vector<int> patches_across_;  
  mutable std::vector<int> coordinates_;
  mutable bool finished_;

  // Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<ParameterSpace>(*this);
    ar & parameter_grid_;
    ar & bounds_;
    ar & periodic_;
    ar & dimension_;
    ar & patch_width_;
    ar & patches_across_;
    ar & coordinates_;
    ar & finished_;
  }
};

BOOST_CLASS_EXPORT_KEY(EuclideanParameterSpace);

inline void
EuclideanParameterSpace::initialize ( const Configuration & config ) {
  boost::shared_ptr<Grid> parameter_grid ( new UniformGrid );
  initialize ( config, parameter_grid );
}

inline void
EuclideanParameterSpace::initialize ( const Configuration & config, 
                                      boost::shared_ptr<Grid> parameter_grid ) {
  parameter_grid_ = parameter_grid;

  // Initialization for TreeGrid
  if ( boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid_ ) ) {
    boost::shared_ptr<TreeGrid> grid = 
      boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid_ );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_PERIODIC );  
    for (int i = 0; i < config.PARAM_SUBDIV_DEPTH[0]; ++i) {
      for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
        parameter_grid_ -> subdivide (); // subdivide every top cell
      }
    }
    bounds_ = grid -> bounds ();
  }

  // Initialization for UniformGrid
  if ( boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid_ ) ) {
    boost::shared_ptr<UniformGrid> grid = 
      boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid_ );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_SUBDIV_SIZES,
                         config.PARAM_PERIODIC );
    bounds_ = grid -> bounds ();
  }
  
  // Initialization for EdgeGrid
  if ( boost::dynamic_pointer_cast < EdgeGrid > ( parameter_grid_ ) ) {
    boost::shared_ptr<EdgeGrid> grid = 
      boost::dynamic_pointer_cast < EdgeGrid > ( parameter_grid_ );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_SUBDIV_SIZES,
                         config.PARAM_PERIODIC );
		bounds_ = grid -> bounds ();
  }

  // Initialize Patch creation
    // In order to accommodate periodicity, 
    //    we let the patches overhang from the outer bounds slightly.
  patch_width_ = 4; // try to use (patch_width +- 1)^d boxes per patch 
  
    // Create patches_across:
    //    The distance between patch centers in each dimension.
    // EXAMPLE: num_across = 64, patch_width = 4 ---> patches_across = 9 

  patches_across_ . resize ( config.PARAM_DIM );
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
    patches_across_[d] = 1 + config.PARAM_SUBDIV_SIZES[d] / patch_width_; 
  }

  // Dimension
  dimension_ = config.PARAM_DIM;
  // Periodicity
  periodic_ = config.PARAM_PERIODIC;

  // Initialize patch coordinates
  coordinates_ . resize ( config.PARAM_DIM, 0);
  finished_ = false;

}

inline std::vector<uint64_t> 
EuclideanParameterSpace::adjacencies ( uint64_t v ) const {
	std::vector<uint64_t> neighbors;
	RectGeo geo = * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ( v ) ) -> geo;
  for ( int d = 0; d < dimension_; ++ d ) {
  	double tol = (bounds_.upper_bounds[d]-bounds_.lower_bounds[d])
  	  /(double)(1000000000.0);
  	geo . lower_bounds [ d ] -= tol;
  	geo . upper_bounds [ d ] += tol;
  }
  neighbors = parameter_grid_ -> cover ( geo );
  return neighbors;
}
	
inline uint64_t 
EuclideanParameterSpace::size ( void ) const {
	return parameter_grid_ -> size ();
}

inline boost::shared_ptr<Parameter> 
EuclideanParameterSpace::parameter ( uint64_t v ) const {
	boost::shared_ptr<RectGeo> geo = boost::dynamic_pointer_cast<RectGeo> 
      ( parameter_grid_ -> geometry ( v ) );
  return boost::shared_ptr<Parameter> ( new EuclideanParameter ( geo ) );
}

inline uint64_t 
EuclideanParameterSpace::search ( boost::shared_ptr<Parameter> parameter ) const {
  RectGeo geo = * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ) -> geo;
  std::vector<uint64_t> vertices = parameter_grid_ -> cover ( geo );
  if ( vertices . size () != 1 ) return * end ();
  return vertices [ 0 ];
}

inline boost::shared_ptr<ParameterPatch> 
EuclideanParameterSpace::patch ( void ) const {

#ifdef EDGEPATCHMETHOD
  // DEBUG
  std::cout << "EDGEPATCHMETHOD\n";
  // END DEBUG
  boost::shared_ptr<ParameterPatch> result;
  while ( 1 ) {
    std::cout << "Calling parent patch method.\n";
    result = ParameterSpace::patch ();
    if ( result -> vertices . empty () ) break;
    uint64_t u = result -> vertices [ 0 ];
    uint64_t v = result -> vertices [ 1 ];
    RectGeo u_geo = * boost::dynamic_pointer_cast<EuclideanParameter> 
      ( result -> parameter [ u ] ) -> geo;
    RectGeo v_geo = * boost::dynamic_pointer_cast<EuclideanParameter> 
      ( result -> parameter [ v ] ) -> geo;
    // DEBUG
    std::cout << "Examining candidate patch " << u_geo << " -- " << v_geo << "\n";
    // END DEBUG
    int codimension = 0;
    for ( int d = 0; d < dimension_; ++ d ) {
      if ( ( u_geo . lower_bounds [ d ] == v_geo . upper_bounds [ d ] )
      || ( u_geo . upper_bounds [ d ] == v_geo . lower_bounds [ d ] ) ) {
        ++ codimension;
      }
    }
    // DEBUG
    std::cout << " Calculated the intersection codimension of " << codimension << "\n";
    // END DEBUG
    if ( codimension == 1 ) break;
  }
  return result;
#else
  //std::cout << "EuclideanParameterSpace::patch dimension_ = " << dimension_ << "\n";

  boost::shared_ptr<ParameterPatch> result ( new ParameterPatch );
  if ( finished_ ) {
  	finished_ = false;
  	return result;
  }

  // Determine a rectangle based on "coordinates"
  RectGeo geo ( dimension_ );
  for ( int d = 0; d < dimension_; ++ d ) {
	 // tol included for robustness
  	double tol = (bounds_.upper_bounds[d] - bounds_.lower_bounds[d]) 
                   /(double)(1000000000.0);
    geo . lower_bounds [ d ] = 
        bounds_.lower_bounds[d]+((double)coordinates_[d])*
        (bounds_.upper_bounds[d]-bounds_.lower_bounds[d]) 
        /(double)patches_across_[d] - tol;
    geo . upper_bounds [ d ] = 
        bounds_.lower_bounds[d]+((double)(1+coordinates_[d]))*
        (bounds_.upper_bounds[d]-bounds_.lower_bounds[d])
        /(double)patches_across_[d] + tol;
      
    if ( not periodic_ [ d ] ) {
      if ( geo . lower_bounds [ d ] < bounds_ . lower_bounds [ d ] ) 
        geo . lower_bounds [ d ] = bounds_ . lower_bounds [ d ];
      if ( geo . upper_bounds [ d ] > bounds_. upper_bounds [ d ] ) 
        geo . upper_bounds [ d ] = bounds_ . upper_bounds [ d ];
    }
  }
   
  //std::cout << "EuclideanParameterSpace::patch geo = " << geo << "\n";
  // Prepare ParameterPatch

  // Find Vertices in Patch
  std::vector<uint64_t> vertices = parameter_grid_ -> cover ( geo );
  boost::unordered_set<uint64_t> vertex_set ( vertices . begin (), vertices . end () );
 
  //std::cout << "EuclideanParameterSpace::patch vertices in patch = " 
  //  << vertices . size () << "\n";

  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( uint64_t vertex, vertices ) {
    //std::cout << "EuclideanParameterSpace::patch vertex = " << vertex << "\n"; 
  	result -> vertices . push_back ( (uint64_t) vertex );
    result -> parameter [ (uint64_t) vertex ] = parameter ( vertex );
    //std::cout << "EuclideanParameterSpace::patch parameter = " << * parameter(vertex) << "\n"; 
    std::vector<uint64_t> neighbors = adjacencies ( vertex );
    //std::cout << "EuclideanParameterSpace::patch number of neighbors = " 
    //  << neighbors . size () << "\n";
    BOOST_FOREACH ( uint64_t other, neighbors ) {
      //std::cout << "EuclideanParameterSpace::patch adjacent vertex = " << other << "\n"; 
      if (( vertex_set . count ( other ) != 0 ) && vertex < other ) {
        result -> edges . push_back ( std::make_pair ( vertex, other ) );
      }
    }
  }

  // Odometer step (multidimensional loop iteration)
  finished_ = true;
  for ( int d = 0; d < dimension_; ++ d ) {
    ++ coordinates_ [ d ];
    if ( coordinates_ [ d ] == patches_across_ [ d ] ) {
      coordinates_ [ d ] = 0;
    } else {
      finished_ = false;
      break;
    }
  }

  return result;
#endif
}

#endif
