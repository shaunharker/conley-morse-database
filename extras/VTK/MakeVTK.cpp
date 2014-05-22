
#include "database/structures/MorseGraph.h"
#include "database/structures/PointerGrid.h"
BOOST_CLASS_EXPORT_IMPLEMENT(PointerGrid);

#include <fstream>
#include <exception>

struct Point {
  double x;
  double y;
  double z;
  Point ( double x, double y, double z ) : x(x), y(y), z(z) {}
  bool operator == ( const Point & rhs ) const {
    if ( (float) x != (float) rhs.x ) return false;
    if ( (float) y != (float) rhs.y ) return false;
    if ( (float) z != (float) rhs.z ) return false;
    return true;
  }
};

inline std::size_t hash_value(Point const& p) {
  std::size_t seed = 0;
  boost::hash_combine(seed, (float) p.x );
  boost::hash_combine(seed, (float) p.y );
  boost::hash_combine(seed, (float) p.z );
  return seed;
}


int main ( int argc, char * argv [] ) {

  std::ifstream lyapunov_file ( "lyapunov.txt" );
  std::ofstream vtk_file ( "lyapunov.vtk" );
  MorseGraph cmg;
  cmg . load ( "cushing.cmg" );
  uint64_t N = cmg . phaseSpace () -> size ();
  // N is also the number of values in lyapunov.txt

  std::cout << "There are " << N << " grid elements.\n";
  boost::unordered_map < Point, uint64_t > points;
  std::vector< Point > point_by_index;
  uint64_t num_points = 0;
  std::cout << "Computing points\n";
  for ( uint64_t i = 0; i < N; ++ i ) {
    int progresspercent = 0;
    if ( (100*i)/N > progresspercent) {
      progresspercent = (100*i)/N;
      std::cout << "\r" << progresspercent << "%    ";
      std::cout . flush ();
    }
    RectGeo rect = * boost::dynamic_pointer_cast<RectGeo> 
      ( cmg . phaseSpace () -> geometry ( i ) ); 
    for ( int k = 0; k < 8; ++ k ) {
      double x = (k&1)?rect.upper_bounds[0]:rect.lower_bounds[0];
      double y = (k&2)?rect.upper_bounds[1]:rect.lower_bounds[1];
      double z = (k&4)?rect.upper_bounds[2]:rect.lower_bounds[2];
      Point p ( x, y, z );
      if ( points . count ( p ) ) continue;
      point_by_index . push_back ( p );
      points [ p ] = num_points ++;
    }
  }
  std::cout << "\r";

  uint64_t M = points . size ();

  std::cout << "There are " << M << " distinct points (i.e. vertices of grid elements).\n";
  // OUTPUT PREAMBLE
  vtk_file << "# vtk Datafile version 3.0\n";
  vtk_file << "vtk output\n";
  vtk_file << "ASCII\n";
  vtk_file << "DATASET UNSTRUCTURED_GRID\n";

  // OUTPUT POINTS
  std::cout << "Outputting points\n";
  vtk_file << "POINTS " << M << " float\n";
  for ( uint64_t i = 0; i < M; ++ i ) {
    int progresspercent = 0;
    if ( (100*i)/M > progresspercent) {
      progresspercent = (100*i)/M;
      std::cout << "\r" << progresspercent << "%    ";
      std::cout . flush ();
    }
    const Point & p = point_by_index [ i ];
    vtk_file << p.x << " " << p.y << " " << p.z << "\n";
  }
  std::cout << "\r";

  // OUTPUT CELLS
  std::cout << "Outputting cells\n";
  vtk_file << "CELLS " << N << " " << 9*N << "\n";
  for ( uint64_t i = 0; i < N; ++ i ) {
    int progresspercent = 0;
    if ( (100*i)/N > progresspercent) {
      progresspercent = (100*i)/N;
      std::cout << "\r" << progresspercent << "%    ";
      std::cout . flush ();
    }
    RectGeo rect = * boost::dynamic_pointer_cast<RectGeo> 
     ( cmg . phaseSpace () -> geometry ( i ) ); 
    vtk_file << "8";
    for ( int k = 0; k < 8; ++ k ) {
      double x = (k&1)?rect.upper_bounds[0]:rect.lower_bounds[0];
      double y = (k&2)?rect.upper_bounds[1]:rect.lower_bounds[1];
      double z = (k&4)?rect.upper_bounds[2]:rect.lower_bounds[2];
      Point p ( x, y, z );
      uint64_t pi = points [ p ];
      vtk_file << " " << pi;
    }
    vtk_file << "\n";
  }
  std::cout << "\r";


  // OUTPUT CELL TYPES (all VTK_VOXEL == 11 )
  //std::cout << "Outputting cell types\n";
  vtk_file << "CELL_TYPES " << N << "\n";
  for ( uint64_t i = 0; i < N; ++ i ) {
    vtk_file << "11\n";
  }

  // OUTPUT SCALAR FIELD INFORMATION
  std::cout << "Outputting scalars\n";
  vtk_file << "CELL_DATA " << N << "\n";
  vtk_file << "SCALARS cell_scalars float 1\n";
  vtk_file << "LOOKUP_TABLE default\n";
  for ( uint64_t i = 0; i < N; ++ i ) {
    int progresspercent = 0;
    if ( (100*i)/N > progresspercent) {
      progresspercent = (100*i)/N;
      std::cout << "\r" << progresspercent << "%    ";
      std::cout . flush ();
    }
     RectGeo rect = * boost::dynamic_pointer_cast<RectGeo> 
      ( cmg . phaseSpace () -> geometry ( i ) );
    double intensity;
    lyapunov_file >> intensity;
    if ( not lyapunov_file . good () ) {
      throw std::logic_error ( "Not enough data points in lyapunov.txt to correspond to MorseGraph.\n");
    }
    // PUT CODE HERE TO PUT IN A VTK ENTRY CORRESPONDING TO
    vtk_file << intensity << "\n";
    //   RECTGEO AND INTENSITY
  }
  std::cout << "\r               \n";

  vtk_file . close ();
  lyapunov_file . close ();
  return 0;
}
