#ifndef BOOLEANCLASSES_H
#define BOOLEANCLASSES_H

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <string>
#include <sstream>



// ----------------------------------------------
// Simple Boolean Box (subdomain of phase space )
// with the two sets of parameters
// ----------------------------------------------
class BooleanBox {
public:
	BooleanBox ( void ) {}
	BooleanBox ( chomp::Rect g, chomp::Rect s, chomp::Rect r ) : gamma(g),sigma(s),rect(r) { }

	chomp::Rect gamma;
	chomp::Rect sigma;
	chomp::Rect rect;
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & gamma;
    ar & sigma;
    ar & rect;
  }
};

inline std::ostream& operator<< (std::ostream& os, const BooleanBox& box) {
	os << "Boolean Box with bounds : " << box.rect << " and parameters (gi,si) : ";
  os << box.gamma << " , " << box.sigma;
	os << "\n";
  return os;
}

// -----------------------------------------------------
// Class for the codimension 1 "face" of the boolean box
// if it is a fixed point, it will be defined as a zero
// dimensional Rect 
// -----------------------------------------------------
class Face { // Will only work for cubes
public: 
  int direction; // means X_{direction} = cst. direction=-1 means a fixed point
  chomp::Rect rect; // will be n-dimensional, and degenerated along the dimension given by direction

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & direction;
    ar & rect;
  }
};

inline std::ostream& operator<< (std::ostream& os, const Face& face) {
  os << "Face for x=cst in the direction : " << face.direction << " , ";
  os << " with bounds : " << face.rect << "\n";
  return os;
}

// need to know when two faces overlap properly

inline bool intersect ( const chomp::Rect & rect1, const chomp::Rect & rect2 ) {
  int dim = rect1 . dimension();
  for ( unsigned int i=0; i<dim; ++i ) { 
    // if ( ( rect1.lower_bounds[i] >= rect2.upper_bounds[i] ) || 
    //      ( rect1.upper_bounds[i] <= rect2.lower_bounds[i] ) ) {
    //   return false;
    // } 
    // add a small epsilon to avoid some false positive in the intersection
    if ( rect1.lower_bounds[i] >= rect2.upper_bounds[i] - 1e-10 ) {
      return false;
    }
    if ( rect1.upper_bounds[i] <= rect2.lower_bounds[i] + 1e-10 ) {
      return false;
    }

  }
  return true;
}

// Check when two faces are identical
inline bool operator == ( const Face & face1, const Face & face2 ) {
	double eps_ = 1e-10;	
		if ( face1 . direction != face2 . direction ) {
			return false;
		}  
		std::vector < double > lb1, ub1, lb2, ub2;
		lb1 = face1 . rect . lower_bounds;
		ub1 = face1 . rect . upper_bounds;
		lb2 = face2 . rect . lower_bounds;
		ub2 = face2 . rect . upper_bounds;
		for ( unsigned int i=0; i<face1.rect.dimension(); ++i ) {
			if ( (std::abs(lb1[i]-lb2[i]) > eps_) || (std::abs(ub1[i]-ub2[i]) > eps_) ) { return false; }
		} 
		return true;
	}


// -------------------------------------------------------------------
// Class for the configuration of a Boolean network and its definition
// -------------------------------------------------------------------
class BooleanConfig { 

public:
	void load ( const char * inputfile, const RectGeo & param );

	std::vector < BooleanBox > listboxes ( void ) const { return booleanboxes_; }
  std::vector < BooleanBox > & listboxes ( void ) { return booleanboxes_; }

	chomp::Rect phasespace ( void ) const { return phasespace_; }

private:
	int dimension_;
	chomp::Rect phasespace_;
	std::vector < BooleanBox > booleanboxes_;
public:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & dimension_;
    ar & phasespace_;
    ar & booleanboxes_;
  }
};


// Valid in Higher Dimension
inline void BooleanConfig::load ( const char * inputfile, const RectGeo & param ) {
  using boost::property_tree::ptree;
  ptree pt;

  std::vector < BooleanBox > booleanboxesloaded;

  PyGILState_STATE gstate = PyGILState_Ensure();

  //  // Retrieve the main module.
  boost::python::object main = boost::python::import("__main__");
  
  // // Retrieve the main module's namespace
  boost::python::object global(main.attr("__dict__"));

  // run the python script and get back the string with xml configuration
  boost::python::object result = exec_file( inputfile, global, global );

  std::vector < double > lb = param . lower_bounds;
  std::vector < double > ub = param . upper_bounds;

  std::vector < double > v;
  for ( unsigned int i=0; i<lb.size(); ++i ) {
    v . push_back ( 0.5*(lb[i]+ub[i]) );
  }
  // append the upperbounds for the parameters values
  for ( unsigned int i=0; i<ub.size(); ++i ) {
    v . push_back ( ub[i] );
  }

  global["pyvector"]=v;

  boost::python::object ignored = boost::python::exec("result = config(pyvector)", global, global);

  std::string message = boost::python::extract<std::string>(global["result"]);

  PyGILState_Release(gstate);

  std::stringstream ss ( message );
  read_xml(ss, pt);

  dimension_ = pt.get<int>("atlas.dimension");
//
  std::vector < double > lbgamma, ubgamma, lbsigma, ubsigma;
  std::vector < chomp::Rect > boxes;
//
  // Phase space information
  std::vector < double > lbounds, ubounds;
  lbounds . resize ( dimension_ );
  ubounds . resize ( dimension_ );
  std::string lower_bounds = pt.get<std::string>("atlas.phasespace.bounds.lower");
  std::string upper_bounds = pt.get<std::string>("atlas.phasespace.bounds.upper");
  std::stringstream lbss ( lower_bounds );
  std::stringstream ubss ( upper_bounds );
  for ( int d = 0; d < dimension_; ++ d ) {
    lbss >> lbounds [ d ];
    ubss >> ubounds [ d ];
  }
  phasespace_ = chomp::Rect ( dimension_, lbounds, ubounds );

  lbgamma . resize ( dimension_ );
  ubgamma . resize ( dimension_ );
  lbsigma . resize ( dimension_ );
  ubsigma . resize ( dimension_ );

  std::string gammastr = pt.get<std::string>("atlas.gamma.lower");
  std::stringstream gammass ( gammastr );
  std::string gammastr2 = pt.get<std::string>("atlas.gamma.upper");
  std::stringstream gammass2 ( gammastr2 );
 
  for ( int d = 0; d < dimension_; ++ d ) {
  	gammass >> lbgamma [ d ];
    gammass2 >> ubgamma [ d ];
  }

  chomp::Rect gamma ( dimension_, lbgamma, ubgamma );

  //
  BOOST_FOREACH ( ptree::value_type & v, pt.get_child("atlas.listboxes") ) {
		std::string sigmastr = v . second . get_child ( "sigma.lower" ) . data ( );
    std::string sigmastr2 = v . second . get_child ( "sigma.upper" ) . data ( );

    std::string lbstr = v . second . get_child ( "bounds.lower" ) . data ( );
    std::string ubstr = v . second . get_child ( "bounds.upper" ) . data ( );
    //
		std::stringstream sigmass ( sigmastr );
    std::stringstream sigmass2 ( sigmastr2 );

    std::stringstream lbss ( lbstr );
    std::stringstream ubss ( ubstr );
    //
		for ( int d = 0; d < dimension_; ++ d ) {
  		sigmass >> lbsigma [ d ];
      sigmass2 >> ubsigma [ d ];
  	}
  	//
    for ( unsigned int d = 0; d < dimension_; ++ d ) {
      lbss >> lbounds [ d ];
      ubss >> ubounds [ d ];
    }
    //
    chomp::Rect sigma ( dimension_, lbsigma, ubsigma );
    booleanboxesloaded . push_back ( BooleanBox ( gamma, sigma, chomp::Rect(dimension_, lbounds, ubounds) ) );
  }

  booleanboxes_ = booleanboxesloaded;

}


// --------------------------------------------------------
// Due to some limitations in Boost::python, we need to run 
// each python call into their own thread
// 
// --------------------------------------------------------
class RunPythonThread { 
public:
  // constructor
  RunPythonThread ( BooleanConfig *bc, std::string inputfile, const RectGeo parameters ) : 
              booleanconfig_(bc),
              inputfile_(inputfile), 
              parameters_(parameters) {}

  void operator ( ) ( void ) {
    booleanconfig_ -> load ( inputfile_.c_str(), parameters_ );
  }

private:
  BooleanConfig * booleanconfig_;
  std::string inputfile_;
  const RectGeo parameters_;
};


#endif
