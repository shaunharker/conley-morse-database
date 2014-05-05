#ifndef CMDB_MODEL_H
#define CMDB_MODEL_H

#include "boost/timer.hpp"

#include "Boolean.h"
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
  // return a shared ptr to the phase space Atlas corresponding to
  // parameter p

  void saveCharts ( int argc, char * argv [] );

private:
  BooleanConfig booleanconfig_;
  std::vector < Face > faces_;
  int dim_;
  std::string path_;

public:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & booleanconfig_;
    ar & faces_;
    ar & dim_;
    ar & path_;
  }
};

inline void Model::initialize ( int argc, char * argv [] ) { 
  //
  Configuration config;
  config . loadFromFile ( argv[1] );
  
  path_ = std::string(argv[1]);

  dim_ = config . PHASE_DIM; 

}

inline boost::shared_ptr < ModelMap > Model::map ( const Parameter & param ) { 
  //
  //
  // boost::timer time_start;
  //
  phaseSpace ( param ); 
  //
  boost::shared_ptr < ModelMap > atlasmap = constructMaps ( booleanconfig_ . phasespace ( ), 
                                                            booleanconfig_ . listboxes ( ), 
                                                            faces_ );
  //
  // double time_elapsed = time_start . elapsed();

  // std::cout << "Model::Map took " << time_elapsed << " s (note: one phaseSpace call is included)\n";

  return atlasmap;
}



inline boost::shared_ptr < Grid > Model::phaseSpace ( const Parameter & p ) {

  // boost::timer time_start;

  boost::shared_ptr < Atlas > myatlas ( new Atlas );

// Initialize the python interpreter on the first call only
  static bool firstcall=true;
  if ( firstcall ) {
    std::cout << "Initialize python\n";
// Initialize python 
    Py_Initialize ( );

    PyEval_InitThreads();
    PyEval_ReleaseLock();
  // To be able to pass std::vector<double> to python function 
    boost::python::class_<std::vector<double> >("PyVec")
      .def(boost::python::vector_indexing_suite<std::vector<double> >());
      firstcall = false;
  }

// Execute the initialization of booleanconfig through a thread
  // std::string directory ( path_ );
  std::string filename ("/configspace.py");
  // RunPythonThread ( BooleanConfig *bc, std::string inputfile, const RectGeo parameters )
  RunPythonThread pythonthread ( &booleanconfig_, path_+filename, p );

  boost::thread t ( pythonthread );

  t.join();

  std::vector < Face > faces;
  std::vector < Face > * faces_ptr;
  faces_ptr = & faces;

  constructFaces ( faces_ptr, booleanconfig_ . listboxes ( ) );
  faces_ = faces;

  // for each face, add a chart to the atlas
  // This will keep the ordering between faces and charts consistent
  // Atlas atlas;
  //
  for ( unsigned int i=0; i<faces_.size(); ++i ) {

    std::vector < double > lb, ub;
    int dir;
    // pure convention. for a fixed point we assume spatial direction = 0
    if ( faces_ [ i ] . direction == -1 ) {
      dir = 0;
    } else { 
      dir = faces_ [ i ] . direction;
    }
    // the charts are codimension-1;
    for ( unsigned int j=0; j<dim_; ++j ) {
      if ( j != dir ) { 
        lb . push_back ( faces_ [ i ] . rect . lower_bounds [ j ] );
        ub . push_back ( faces_ [ i ] . rect . upper_bounds [ j ] );
      }
    }
    // check if it is a fixed point
    if ( faces_ [ i ]  . direction != - 1) {      
      myatlas -> add_chart ( i, RectGeo ( dim_ - 1, lb, ub ) );
    } else { 
      // myatlas -> add_chart ( i, 0, RectGeo ( dim_ - 1, lb, ub ) );
      myatlas -> add_chart ( i, RectGeo ( 0 ) );
    }
  }

  // double time_elapsed = time_start . elapsed();

  // std::cout << "Model::PhaseSpace took " << time_elapsed << " s\n";

  //
  return boost::dynamic_pointer_cast<Grid> ( myatlas );
}

inline void Model::saveCharts ( int argc, char * argv [] ) { 
  //
  std::string directory ( path_ );
  std::string filename ("/charts.dat");
  //
  exportCharts ( (directory+filename).c_str(), faces_ );
}



#endif
