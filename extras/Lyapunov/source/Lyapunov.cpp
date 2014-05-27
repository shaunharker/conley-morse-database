// Calculate Lyapunov function
// Use Succinct Grid and Modified Tarjan method
// Keep track of memory usage

#define MEMORYBOOKKEEPING
#define CMG_VERBOSE

#include <iostream>
#include <fstream>
#include <exception>

//#include "Draw.h"
#include "Model.h"

#include "ComputeLyapunov.h"

#include "database/structures/RectGeo.h"
#include "database/structures/TreeGrid.h"
#include "database/structures/MorseGraph.h"
#include "database/program/Configuration.h"
#include "database/maps/Map.h"

#ifndef MISSING_SDSL
#include "database/structures/SuccinctGrid.h"
#endif
#include "database/structures/PointerGrid.h"

#include "boost/shared_ptr.hpp"
#include <boost/serialization/export.hpp>
#ifndef MISSING_SDSL
BOOST_CLASS_EXPORT_IMPLEMENT(SuccinctGrid);
#endif
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);


/// Load phase space from .mg or .cmg file
boost::shared_ptr<TreeGrid> 
phaseSpace ( int argc, char * argv [] );

/// Load ModelMap with appropriate parameters from config.xml file
boost::shared_ptr<const Map> 
modelMap ( int argc, char * argv [] );

/// Save result of Lyapunov function calculation to a file (in argv[2])
void 
saveLyapunov ( std::vector<double> global_lyapunov, 
               int argc, char * argv [] );


/// main
///   load .mg or .cmg file in first command line argument
///   save lyapunov function to file indicated in second command line argument
int main ( int argc, char * argv [] ) {

  if ( argc < 3 ) {
    std::cout << "Please supply command line arguments. The first should be "
                 " the data file created by SingleCMG holding the Morse decomposition"
                 " information (.mg/.cmg file). The second should be the name of the"
                 " output file (to be created) that will contain the Lyapunov function. "
                 " The third should be the path to the folder containing the config.xml file."
                 " The remaining should be floating point numbers specifying "
                 " the parameter value to compute at. These should be the same "
                 " values that were passed to SingleCMG. Example: \n"
                 " ./computeLyapunov ./data.cmg ./lyapunov.txt ./ 19.0 20.0 \n";
    return 0;
  }

  // Obtain phase space  
  boost::shared_ptr<TreeGrid> grid = phaseSpace ( argc, argv );

  // Obtain model map
  boost::shared_ptr<const Map> map = modelMap ( argc - 2, argv + 2 );

  // Compute Lyapunov function
  std::vector<double> global_lyapunov = ComputeLyapunov ( grid, map );

  // Save Lyapunov function
  std::cout << "Saving Lyanpunov function.\n";
  saveLyapunov ( global_lyapunov, argc, argv );

  return 0;
}

/// Load phase space from .mg or .cmg file
boost::shared_ptr<TreeGrid> phaseSpace ( int argc, char * argv [] ) {
  MorseGraph mg;
  mg . load ( argv[1] );
  boost::shared_ptr<TreeGrid> result = 
    boost::dynamic_pointer_cast<TreeGrid> (mg . phaseSpace ());
  if ( not result ) {
    throw std::logic_error ( "Error. data.mg file does not store " 
                             "a phase space of type TreeGrid\n");
  }
  return result;
}

/// Load ModelMap with appropriate parameters from config.xml file
boost::shared_ptr<const Map> modelMap ( int argc, char * argv [] ) {
  Model model;
  model . initialize ( argc, argv );
  return model . map (); // passing null parameter triggers default response
}

void saveLyapunov ( std::vector<double> global_lyapunov, 
                    int argc, char * argv [] ) {
  std::ofstream outfile ( argv [ 2 ] );
  if ( not outfile . good () ) {
    throw std::logic_error ( "Unable to create output file.\n");
  }
  outfile << std::setprecision( std::numeric_limits<double>::digits10+2);
  BOOST_FOREACH ( double value, global_lyapunov ) {
    outfile << value << "\n";
  }
  outfile . close ();
}
