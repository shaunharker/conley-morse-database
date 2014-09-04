
// Serialization CPP code
#include <boost/serialization/export.hpp>
#include "database/structures/Grid.h"
#include "database/structures/PointerGrid.h"
#include "database/structures/SuccinctGrid.h"
#include "database/structures/UniformGrid.h"
#include "database/structures/EdgeGrid.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/EuclideanParameterSpace.h"
#include "database/structures/AbstractParameterSpace.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(UniformGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EdgeGrid);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameter);
BOOST_CLASS_EXPORT_IMPLEMENT(EuclideanParameterSpace);
BOOST_CLASS_EXPORT_IMPLEMENT(AbstractParameterSpace);

#define BS_DEBUG_MODELMAP
#include <iostream>
#include "Model.h"
#include "database/structures/MorseGraph.h"
#include "database/program/jobs/Compute_Morse_Graph.h"
#include "boost/foreach.hpp"
#include "boost/shared_ptr.hpp"
#include "Parameter/BooleanSwitchingParameterSpace.h"
#include "Parameter/FactorGraph.h"
#include "database/structures/ParameterSpace.h"
#include "database/structures/Database.h"

#include <boost/algorithm/string.hpp>


//arguments :

//path/database.mdb path path/networks/mynetwork.txt mgcc_index

int main ( int argc, char * argv [] ) {
  Database database;
  database . load ( argv [ 1 ] );

  const AbstractParameterSpace & space = 
    dynamic_cast<const AbstractParameterSpace&> (database . parameter_space ());

  boost::unordered_set < uint64_t > parameters;
  boost::unordered_set < uint64_t > mgr_indices;
  
  // the index of interest comes from the last argument
  uint64_t mgccp_index;
  mgccp_index = atoi(argv[argc-1]);
  
  const MGCCP_Record & mgccp_record =
  database . MGCCP_Records () [ mgccp_index ];
  uint64_t morsegraph_index = mgccp_record . morsegraph_index;
  mgr_indices . insert ( morsegraph_index );
  const MorseGraphRecord & morsegraph_record =
  database . morsegraphData () [ morsegraph_index ];
  
  uint64_t dag_index = morsegraph_record . dag_index;
  const DAG_Data & dag_data = database . dagData () [ dag_index ];
  
  std::cout << "Number of vertices : " << dag_data . num_vertices << "\n";
  
  std::vector < uint64_t > pindex = mgccp_record . parameter_indices;

//
  Model model;
  model . initialize ( argc-1, argv+1 );
//
  BooleanSwitchingParameterSpace & boolean_space = *
  boost::dynamic_pointer_cast<BooleanSwitchingParameterSpace> (
                                                    model . parameterSpace () );
//
  std::cout << "Parameters inequalities : \n";
  for ( uint64_t pi : pindex ) {
    std::cout << boolean_space . prettyPrint ( boolean_space . parameter ( pi ) );
  }
// pick one paramater index to construct p
  boost::shared_ptr<BooleanSwitchingParameter> p =
  boost::dynamic_pointer_cast<BooleanSwitchingParameter> (
                                      boolean_space . parameter ( pindex[0] ) );
//
  boost::shared_ptr<const Map> map = model . map ( p );
  if ( not map ) {
    std::cout << "No map associated with parameter " <<
    *p << "!.\n";
  }
  boost::shared_ptr<Grid> phase_space = model . phaseSpace ();
  if ( not phase_space ) {
    throw std::logic_error ( "Clutching_Graph_Job. model.phaseSpace() failed"
                            " to return a valid pointer.\n");
  }
  
  // Perform Morse Graph computation
  MorseGraph mg;
  Compute_Morse_Graph
  ( & mg,
   phase_space,
   map,
   0,
   0,
   0,
   0 );
  
  
  model . annotate ( & mg );
  
  std::cout << "Computed a Morse graph with "
  << mg . NumVertices () << " nodes.\n";
  
  
//  
//  std::ofstream ofile;
//  ofile.open("morse_sets.dat");
//  
//  typedef std::vector<Grid::GridElement> CellContainer;
//  typedef  MorseGraph::VertexIterator VI;
//  VI it, stop;
//  for (boost::tie ( it, stop ) = mg . Vertices (); it != stop;  ++ it ) {
//    
//    
//    std::set < std::string > annotations = mg . annotation ( *it );
//    // should have only one annotation
//    BOOST_FOREACH ( std::string str, annotations ) {
//      std::vector<std::string> fields;
//      boost::split ( fields, str, boost::is_any_of (":") );
//      if ( fields[0] == "FC " ) {
//        std::cout << "found FC\n";
//        
//        
//        
//      }
//    }
//    
//    // std::cout << "new vertex \n";
//    
//    boost::shared_ptr<const Grid> my_subgrid ( mg . grid ( *it ) );
//    
//    if ( not my_subgrid ) {
//      std::cout << "Abort! This vertex does not have an associated grid!\n";
//      abort ();
//    }
//    CellContainer my_subset = phase_space -> subset ( * my_subgrid );
//    
//    BOOST_FOREACH ( Grid::GridElement ge, my_subset ) {
//      if ( not boost::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) ) ) {
//        std::cout << "Unexpected null response from geometry\n";
//      }
//      AtlasGeo geo = * boost::dynamic_pointer_cast < AtlasGeo > ( phase_space -> geometry ( ge ) );
//      RectGeo box =  geo . rect ();
//      int id = geo . id ();
//      // std::cout << "(Chart, Rect) = (" << id << ", " << box << ")\n";
//      
//      ofile << std::setprecision(15) << *it << " " << id << " ";
//      
//      
//      
//      for ( unsigned int i=0; i<box.dimension(); ++i ) {
//        ofile << std::setprecision(15) << box.lower_bounds[i] << " " << box.upper_bounds[i] << " ";
//      }
//      ofile << "\n";
//      
//    }
//  }
//  
//  ofile.close();
  
 
 
  return 0;
}
