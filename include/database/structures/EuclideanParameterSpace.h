#ifndef CMDB_EUCLIDEANPARAMETERSPACE
#define CMDB_EUCLIDEANPARAMETERSPACE

#include "database/structures/ParameterSpace.h"
#include "boost/shared_ptr.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/unordered_map.hpp"
#include "boost/serialization/shared_ptr.hpp"

class EuclideanParameter : public Parameter {
public:
	boost::shared_ptr<RectGeo> geo;
	EuclideanParameter ( boost::shared_ptr<RectGeo> geo ) : geo(geo) {}
private:
	// Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
  	ar & geo;
  }
  /// derivation interface
  virtual void print ( std::ostream & outstream ) const {
  	outstream << * geo;
  }
};

class EuclideanParameterSpace : public ParameterSpace {
public:

	/// initialize
	///    Create the ParameterSpace given the configuration specified
	void initialize ( const Configuration & config );

	/// adjacencies
	///    Return a vector of adjacent vertices.
	virtual std::vector<uint64_t> adjacencies ( uint64_t v ) const;
	
	/// size
	///    Return the number of vertices
	virtual uint64_t size ( void ) const;

	/// parameter
	///    Return the parameter object associated with a vertex
	virtual boost::shared_ptr<Parameter> parameter ( uint64_t v ) const;
	
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

// EuclideanPatch construction variables
	mutable int patch_width_; 
  mutable std::vector<int> patches_across_;  
  mutable std::vector<int> coordinates_;
  mutable bool finished_;
};

inline void
EuclideanParameterSpace::initialize ( const Configuration & config ) {
  // CONSTUCT THE PARAMETER GRID
  parameter_grid_ = boost::shared_ptr<Grid> ( new PARAMETER_GRID );

  // Initialization for TreeGrid
  if ( boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid ) ) {
    boost::shared_ptr<TreeGrid> grid = 
      boost::dynamic_pointer_cast < TreeGrid > ( parameter_grid );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_PERIODIC );  
    for (int i = 0; i < config.PARAM_SUBDIV_DEPTH[0]; ++i) {
      for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
        parameter_grid -> subdivide (); // subdivide every top cell
      }
    }
    bounds_ = grid -> bounds ();
  }

  // Initialization for UniformGrid
  if ( boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid ) ) {
    boost::shared_ptr<UniformGrid> grid = 
      boost::dynamic_pointer_cast < UniformGrid > ( parameter_grid );
    grid -> initialize ( config.PARAM_BOUNDS, 
                         config.PARAM_SUBDIV_SIZES,
                         config.PARAM_PERIODIC );
    bounds_ = grid -> bounds ();
  }
  
  // Initialization for EdgeGrid
  if ( boost::dynamic_pointer_cast < EdgeGrid > ( parameter_grid ) ) {
    boost::shared_ptr<EdgeGrid> grid = 
      boost::dynamic_pointer_cast < EdgeGrid > ( parameter_grid );
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

  // Initialize patch coordinates
  coordinates_ . resize ( config.PARAM_DIM, 0);
  finished_ = false;

}

inline std::vector<uint64_t> 
EuclideanParameterSpace::adjacencies ( uint64_t v ) const {
	std::vector<uint64_t> neighbors;
	RectGeo geo = * boost::dynamic_pointer_cast<EuclideanParameter> ( parameter ( v ) ) -> geo;
  double tol = (bounds_.upper_bounds[d]-bounds_.lower_bounds[d])/(double)(1000000000.0);
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
  	geo . lower_bounds [ d ] -= tol;
  	geo . upper_bounds [ d ] += tol;
  }
  neighbors = parameter_grid -> cover ( geo );
  return neighbors;
}
	
inline uint64_t 
EuclideanParameterSpace::size ( void ) const {
	return parameter_grid_ -> size ();
}

inline boost::shared_ptr<Parameter> 
EuclideanParameterSpace::parameter ( uint64_t v ) const {
	boost::shared_ptr<RectGeo> geo = boost::dynamic_pointer_cast<RectGeo> 
      ( parameter_grid -> geometry ( vertex ) );
  return boost::shared_ptr<Parameter> ( new EuclideanParameter ( geo ) );
}

inline boost::shared_ptr<ParameterPatch> 
EuclideanParameterSpace::patch ( void ) const {
  
  boost::shared_ptr<ParameterPatch> result ( new ParameterPatch );
  if ( finished ) {
  	finished = false;
  	return result;
  }

  RectGeo geo ( config.PARAM_DIM );
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
	// tol included for robustness
  	double tol = (bounds_.upper_bounds[d] - bounds_.lower_bounds[d]) 
                   /(double)(1000000000.0);
    geo . lower_bounds [ d ] = 
        bounds_.lower_bounds[d]+((double)coordinates[d])*
        (bounds_.upper_bounds[d]-bounds_.lower_bounds[d]) 
        /(double)patches_across[d] - tol;
    geo . upper_bounds [ d ] = 
        bounds_.lower_bounds[d]+((double)(1+coordinates[d]))*
        (bounds_.upper_bounds[d]-bounds_.lower_bounds[d])
        /(double)patches_across[d] + tol;
      
    if ( not config.PARAM_PERIODIC [ d ] ) {
      if ( geo . lower_bounds [ d ] < bounds_ . lower_bounds [ d ] ) 
        geo . lower_bounds [ d ] = bounds_ . lower_bounds [ d ];
      if ( geo . upper_bounds [ d ] > bounds_. upper_bounds [ d ] ) 
        geo . upper_bounds [ d ] = bounds_ . upper_bounds [ d ];
    }
  }
   
  // Prepare ParameterPatch

  // Find Vertices in Patch
  std::vector<uint64_t> vertices = parameter_grid -> cover ( geo );
  boost::unordered_set<uint64_t> vertex_set ( vertices . begin (), vertices . end () );
 
  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( uint64_t vertex, vertices ) {
  	result . vertices . push_back ( (uint64_t) vertex );
    result . parameters [ (uint64_t) vertex ] = parameter ( vertex );
    std::vector<uint64_t> neighbors = adjacencies ( vertex );
    BOOST_FOREACH ( uint64_t other, neighbors ) {
      if (( vertex_set . count ( other ) != 0 ) && vertex < other ) {
        result . edges . push_back ( std::make_pair ( vertex, other ) );
      }
    }
  }

  // Odometer step (multidimensional loop iteration)
  finished = true;
  for ( int d = 0; d < config.PARAM_DIM; ++ d ) {
    ++ coordinates [ d ];
    if ( coordinates [ d ] == patches_across [ d ] ) {
      coordinates [ d ] = 0;
    } else {
      finished = false;
      break;
    }
  }

  return result;
}

#endif
