// picture.hpp
// a tool for converting 2D adaptive cubical complexes into .png files

#include <algorithm>
#include <iterator>
#include "boost/foreach.hpp"
#include "boost/tuple/tuple.hpp"
#include "chomp/Rect.h"
#include "database/structures/TreeGrid.h"

template < class CellContainer >
Picture * draw_picture (const int Width, const int Height,
                        unsigned char Red, unsigned char Green, unsigned char Blue,
                        const TreeGrid & my_grid, const CellContainer & my_subset ) {
  RectGeo bounds = my_grid . bounds ();
  // Prepare variables for bounds finding loop
  Real x_min = bounds . upper_bounds [ 0 ];
  Real x_max = bounds . lower_bounds [ 0 ];
  Real y_min = bounds . upper_bounds [ 1 ];
  Real y_max = bounds . lower_bounds [ 1 ];
  
  // Find bounds of picture
  BOOST_FOREACH ( Grid::GridElement cell, my_subset ) {
    RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( cell ) ) );
    x_min = std::min ( x_min, box . lower_bounds [ 0 ] );
    x_max = std::max ( x_max, box . upper_bounds [ 0 ] );
    y_min = std::min ( y_min, box . lower_bounds [ 1 ] );
    y_max = std::max ( y_max, box . upper_bounds [ 1 ] );
  }
  if ( x_min == x_max || y_min == y_max ) std::cout << "draw_picture: \n" << x_min << " " << x_max << " " << y_min << " " << y_max << "\n";
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  std::cout << "x_min = " << x_min << "\n";
  std::cout << "x_max = " << x_max << "\n";
  std::cout << "y_min = " << y_min << "\n";
  std::cout << "y_max = " << y_max << "\n";

  // Draw picture
  BOOST_FOREACH ( Grid::GridElement cell, my_subset ) {
    RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( cell ) ) );
    picture -> draw_square (Red, Green, Blue,
                            box . lower_bounds [ 0 ],
                            box . upper_bounds [ 0 ],
                            box . lower_bounds [ 1 ],
                            box . upper_bounds [ 1 ]);
  }
  
  return picture;
} /* draw_bitmap */

#include <cmath>
#include "database/structures/MorseGraph.h"
#include "database/structures/Grid.h"
#include "chomp/ConleyIndex.h"

inline
Picture * draw_morse_sets (const int Width,
                           const int Height,
                           const TreeGrid & my_grid,
                           const MorseGraph & conley_morse_graph ) {
  std::cout << "draw_morse_sets\n";
  typedef std::vector<Grid::GridElement> CellContainer;
  using namespace chomp;
  typedef MorseGraph CMG;
  RectGeo bounds = my_grid . bounds ();
  // Prepare variables for bounds finding loop
  Real x_min = bounds . upper_bounds [ 0 ];
  Real x_max = bounds . lower_bounds [ 0 ];
  Real y_min = bounds . upper_bounds [ 1 ];
  Real y_max = bounds . lower_bounds [ 1 ];
  

  // Loop Through Morse Sets to determine bounds
  typedef  CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    // Draw the Morse Set
    boost::shared_ptr<const Grid> my_subgrid ( conley_morse_graph . grid ( *it ) );
    CellContainer my_subset = my_grid . subset ( * my_subgrid );
    for ( CellContainer::const_iterator cellit = my_subset . begin (); 
         cellit != my_subset . end (); ++ cellit ) {
      RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( *cellit ) ) );
      x_min = std::min ( x_min, box . lower_bounds [ 0 ] );
      x_max = std::max ( x_max, box . upper_bounds [ 0 ] );
      y_min = std::min ( y_min, box . lower_bounds [ 1 ] );
      y_max = std::max ( y_max, box . upper_bounds [ 1 ] );

    }
  }
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Loop through Morse Sets to draw them
  int count = 0;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    // Draw the Morse Set
    boost::shared_ptr<const Grid> my_subgrid ( conley_morse_graph . grid ( *it ) );
    CellContainer my_subset = my_grid . subset ( * my_subgrid );
    if ( my_subset . size () < 10 ) std::cout << "Small Morse Set " << count << ": ";
    ++ count;

    for ( CellContainer::const_iterator cellit = my_subset . begin (); 
         cellit != my_subset . end (); ++ cellit ) {

      RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( *cellit ) ) );

      // DEBUG
      if ( my_subset . size () < 10 ) std::cout << box << " ";
      
      picture -> draw_square (Red, Green, Blue,
                              box . lower_bounds [ 0 ],
                              box . upper_bounds [ 0 ],
                              box . lower_bounds [ 1 ],
                              box . upper_bounds [ 1 ], 
                              true);
    }
    if ( my_subset . size () < 10 ) std::cout << "\n";

  }

  return picture;
} /* draw_morse_sets */

inline
Picture * draw_grid (const int Width,
                       const int Height,
                       const TreeGrid & my_grid ) {
  std::cout << "draw_grid\n";

  typedef std::vector<Grid::GridElement> CellContainer;

  RectGeo bounds = my_grid . bounds ();
  // Prepare variables for bounds finding loop
  Real x_min = bounds . lower_bounds [ 0 ];
  Real x_max = bounds . upper_bounds [ 0 ];
  Real y_min = bounds . lower_bounds [ 1 ];
  Real y_max = bounds . upper_bounds [ 1 ];
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Loop through top cells to draw them
  CellContainer my_subset = my_grid . cover ( my_grid . bounds () );
  for ( CellContainer::const_iterator cellit = my_subset . begin ();
       cellit != my_subset . end (); ++ cellit ) {
    // Draw the outline of the cell
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( *cellit ) ) );
    Real volume = ( box . upper_bounds [ 0 ] - box . lower_bounds [ 0 ] )*( box . upper_bounds [ 1 ] - box . lower_bounds [ 1 ] ) ;
    Real total_volume = ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] )*( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) ;
    Real ratio = total_volume / volume;
    Real log_of_ratio = (16.0f - log ( ratio ) / log ( 4.0f) ) * 16.0f;
    Red = Green = Blue = (unsigned char) log_of_ratio;
    picture -> draw_square (Red, Green, Blue,
                                    box . lower_bounds [ 0 ],
                                    box . upper_bounds [ 0 ],
                                    box . lower_bounds [ 1 ],
                                    box . upper_bounds [ 1 ],
                            false);
  }


  return picture;
} /* draw_grid */

inline
Picture * draw_grid_and_morse_sets (const int Width, const int Height,
                       const TreeGrid & my_grid,
                       const MorseGraph  & conley_morse_graph ) {
  std::cout << "draw_grid_and_morse_sets\n";

  typedef std::vector<Grid::GridElement> CellContainer;
  using namespace chomp;
  typedef MorseGraph  CMG;
  RectGeo bounds = my_grid . bounds ();
  // Prepare variables for bounds finding loop
  Real x_min = bounds . lower_bounds [ 0 ];
  Real x_max = bounds . upper_bounds [ 0 ];
  Real y_min = bounds . lower_bounds [ 1 ];
  Real y_max = bounds . upper_bounds [ 1 ];
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Loop through top cells to draw them
  CellContainer my_subset = my_grid . cover ( my_grid . bounds () );
  
  BOOST_FOREACH ( Grid::GridElement cell, my_subset ) {
    // Draw the outline of the cell
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( cell ) ) );
    Real volume = ( box . upper_bounds [ 0 ] - box . lower_bounds [ 0 ] )*( box . upper_bounds [ 1 ] - box . lower_bounds [ 1 ] ) ;
    Real total_volume = ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] )*( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) ;
    Real ratio = total_volume / volume;
    Real log_of_ratio = (32.0 - log ( ratio ) / log ( 2.0) ) * 8.0f;
    Red = Green = Blue = (unsigned char) log_of_ratio;
    picture -> draw_square (Red, Green, Blue,
                            box . lower_bounds [ 0 ],
                            box . upper_bounds [ 0 ],
                            box . lower_bounds [ 1 ],
                            box . upper_bounds [ 1 ],
                            false /* not transparent */);
  }
  
  // Loop through Morse Sets to draw them
  
  // first white them out
  typedef CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    unsigned char Red = 255; // rand () % 255;
    unsigned char Green = 255; // rand () % 255;
    unsigned char Blue = 255; //rand () % 255;
    // Draw the Morse Set
    boost::shared_ptr<const Grid> my_subgrid ( conley_morse_graph . grid ( *it ) );
    CellContainer my_subset = my_grid . subset ( * my_subgrid );
    BOOST_FOREACH ( Grid::GridElement cell, my_subset ) {
      RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( cell ) ) );
      picture -> draw_square (Red, Green, Blue,
                              box . lower_bounds [ 0 ],
                              box . upper_bounds [ 0 ],
                              box . lower_bounds [ 1 ],
                              box . upper_bounds [ 1 ], false);
    }
  }
  
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    unsigned char Red = 0;
    unsigned char Green = 0;
    unsigned char Blue = 0; // initialize to zero to stop compiler warning
    switch ( rand() % 6 ) {
      case 0 : Red = 0; 
        Blue = 255; Green = rand()%255; break;
      case 1 : Blue = 0; Green = 255; Red = rand()%255; break;
      case 2 : Green = 0; Red = 255; Blue = rand()%255; break;
      case 3 : Red = 0; Green = 255; Blue = rand()%255; break;
      case 4 : Green = 0; Blue = 255; Red = rand()%255; break;
      case 5 : Blue = 0; Red = 255; Green = rand()%255; break;
      default : break;
    }
    // Draw the Morse Set
    boost::shared_ptr<const Grid> my_subgrid ( conley_morse_graph . grid ( *it ) );
    CellContainer my_subset = my_grid . subset ( * my_subgrid );
    BOOST_FOREACH ( Grid::GridElement cell, my_subset ) {
      RectGeo box = * boost::dynamic_pointer_cast < RectGeo > ( my_grid . geometry ( my_grid . find ( cell ) ) );
      picture -> draw_square (Red, Green, Blue,
                              box . lower_bounds [ 0 ],
                              box . upper_bounds [ 0 ],
                              box . lower_bounds [ 1 ],
                              box . upper_bounds [ 1 ]);
    }
  }  
  return picture;
} /* draw_grid_and_morse_sets */
