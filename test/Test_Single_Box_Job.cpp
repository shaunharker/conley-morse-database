/// Test the single box job.
/// This simple test program runs the function Compute_Conley_Morse_Graph
/// for a sample parameter box for a sample map.

#include "program/Strategies.h"
#include "program/jobs/Compute_Conley_Morse_Graph.h"
#include "data_structures/Cached_Box_Information.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "tools/picture.h"
#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "algorithms/Homology.h"
#include "maps/leslie.h"

#include "tools/lodepng/lodepng.h"

template < class Toplex >
void draw_ascii_subset ( const Toplex & my_toplex, const typename Toplex::Subset & my_subset ) {
  int Width = 128;
  int Height = 128;
  int i_max = 0;
  int j_max = 0;
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  std::vector < std::vector < bool > > data ( Height + 1 );
  for ( int j = 0; j <= Height; ++ j ) data [ j ] . resize ( Width + 1, false );
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
       it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    int i_left = ( box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (Picture::Real) Width;
    int j_left = ( box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (Picture::Real) Height;
    int i_right = ( box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (Picture::Real) Width;
    int j_right = ( box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (Picture::Real) Height;
    
    i_max = std::max ( i_right, i_max );
    j_max = std::max ( j_right, j_max );
    
    for ( int i = i_left; i < i_right; ++ i )
      for ( int j = j_left; j < j_right; ++ j ) 
        data [ j ] [ i ] = true;
  }
  /*
   for ( int j = Height; j >= 0; -- j ) {
   for ( int i = 0; i <= Width; ++ i ) {
   if ( data [ j ] [ i ] ) std::cout << "#";
   else std::cout << ".";
   }
   std::cout << "\n";
   } 
   */
  j_max += 5;
  i_max += 5;
  data . clear ();
  data . resize ( Height + 1);
  for ( int j = 0; j <= Height; ++ j ) data [ j ] . resize ( Width + 1, false );
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
       it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    int i_left = ( box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (Picture::Real) Width
    * ((Picture::Real) Width / (Picture::Real) i_max );
    int j_left = ( box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (Picture::Real) Height
    * ( (Picture::Real) Height / (Picture::Real) j_max );
    int i_right = ( box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) /
    ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] ) * (Picture::Real) Width
    * ((Picture::Real) Width / (Picture::Real) i_max );
    int j_right = ( box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) /
    ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) * (Picture::Real) Height
    * ( (Picture::Real) Height / (Picture::Real) j_max );
    
    for ( int i = i_left; i < i_right; ++ i )
      for ( int j = j_left; j < j_right; ++ j ) { 
        if ( j > Height || i > Width ) {
          std::cout << " i = " << i << " and j = " << j << "\n";
        } else {
          data [ j ] [ i ] = true;
        }
      }
  }
  for ( int j = Height; j >= 0; -- j ) {
    for ( int i = 0; i <= Width; ++ i ) {
      if ( data [ j ] [ i ] ) std::cout << "#";
      else std::cout << ".";
    } /* for */
    std::cout << "\n";
  } /* for */
} /* draw_ascii_subset */

int main ( int argc, char * argv [] ) 
{
  typedef Decide_Subdiv_Level_Size< Adaptive_Cubical::Toplex > Decide_Subdiv;
  Decide_Subdiv decide_subdiv ( 12 , 100000 );
  typedef Decide_Conley_Index_Size< Adaptive_Cubical::Toplex > Decide_Conley_Index;
  Decide_Conley_Index decide_conley_index ( 2 , 0, false , 0 ); //Decide_Conley_Index::MaxValue );

  typedef ConleyMorseGraph < Adaptive_Cubical::Toplex::Subset , Conley_Index_t > Conley_Morse_Graph;
  Conley_Morse_Graph conley_morse_graph;
  
  if ( argc != 3 ) {
    std::cout << "Usage: Supply 2 (not " << argc << ") arguments:\n";
    std::cout << "Input two integers in [0, 64)\n";
    return 0;
  }
  Adaptive_Cubical::Real bx = ( Adaptive_Cubical::Real ) atoi ( argv [ 1 ] ); //37.0;
  Adaptive_Cubical::Real by = ( Adaptive_Cubical::Real ) atoi ( argv [ 2 ] ); //17.0;
  
  Adaptive_Cubical::Geometric_Description phase_space_box ( 2 , 0 , 300 );
  phase_space_box . lower_bounds [ 0 ] = -0.001;
  phase_space_box . upper_bounds [ 0 ] = 320.056;
  phase_space_box . lower_bounds [ 1 ] = -0.001;
  phase_space_box . upper_bounds [ 1 ] = 224.040;
  
  Adaptive_Cubical::Geometric_Description parameter_space_limits ( 2 , 0 , 300 );
  parameter_space_limits . lower_bounds [ 0 ] = 8.0;
  parameter_space_limits . upper_bounds [ 0 ] = 37.0;
  parameter_space_limits . lower_bounds [ 1 ] = 3.0;
  parameter_space_limits . upper_bounds [ 1 ] = 50.0;
  
  
  Adaptive_Cubical::Geometric_Description parameter_box ( 2 , 20.01 , 20.02 );
  parameter_box . lower_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * bx / 64.0;
  parameter_box . upper_bounds [ 0 ] = parameter_space_limits . lower_bounds [ 0 ] + 
  ( parameter_space_limits . upper_bounds [ 0 ] - parameter_space_limits . lower_bounds [ 0 ] ) * ( bx + 1.0 ) / 64.0;
  parameter_box . lower_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * by / 64.0;
  parameter_box . upper_bounds [ 1 ] = parameter_space_limits . lower_bounds [ 1 ] + 
  ( parameter_space_limits . upper_bounds [ 1 ] - parameter_space_limits . lower_bounds [ 1 ] ) * ( by + 1.0 ) / 64.0;

  
  Adaptive_Cubical::Toplex phase_space;
  phase_space . initialize ( phase_space_box );
  Cached_Box_Information cached_box_information;


  Compute_Conley_Morse_Graph < Conley_Morse_Graph , Adaptive_Cubical::Toplex ,
    Adaptive_Cubical::Toplex , LeslieMap , Decide_Subdiv , Decide_Conley_Index ,
    Cached_Box_Information > ( & conley_morse_graph , & phase_space ,
    parameter_box , decide_subdiv , decide_conley_index ,
    & cached_box_information );

  // Create a Picture
  
  int Width = 512;
  int Height = 512;
  Picture * picture = new Picture( Width, Height, 0.0, 0.0, 0.0, 0.0 ); // this assures I have the origin in current imp
  
  typedef Conley_Morse_Graph::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    //draw_ascii_subset ( phase_space, conley_morse_graph . CubeSet ( * it ) );
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    Picture * morse_picture = draw_picture ( Width, Height, 
                                            Red, Green, Blue, 
                                            phase_space, conley_morse_graph . CubeSet ( *it ) );
    Picture * combination = combine_pictures ( Width, Height, *picture, *morse_picture );
    std::swap ( picture, combination );
    delete morse_picture;
    delete combination;
  }

  LodePNG_encode32_file( "image.png", picture -> bitmap, picture -> Width, picture -> Height);

  return 0;
} /* main */
