// picture.hpp
// a tool for converting 2D adaptive cubical complexes into .png files

#include <algorithm>
#include <iterator>
#include "boost/foreach.hpp"
#include "boost/tuple/tuple.hpp"

template < class Toplex, class CellContainer >
Picture * draw_picture (const int Width, const int Height,
                        unsigned char Red, unsigned char Green, unsigned char Blue,
                        const Toplex & my_toplex, const CellContainer & my_subset ) {
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  // Prepare variables for bounds finding loop
  Picture::Real x_min = bounds . upper_bounds [ 0 ];
  Picture::Real x_max = bounds . lower_bounds [ 0 ];
  Picture::Real y_min = bounds . upper_bounds [ 1 ];
  Picture::Real y_max = bounds . lower_bounds [ 1 ];
  
  // Find bounds of picture
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( cell ) );
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
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( cell ) );
    picture -> draw_square (Red, Green, Blue,
                            box . lower_bounds [ 0 ],
                            box . upper_bounds [ 0 ],
                            box . lower_bounds [ 1 ],
                            box . upper_bounds [ 1 ]);
  }
  
  return picture;
} /* draw_bitmap */

#include <cmath>
#include "structures/Conley_Morse_Graph.h"
#include "algorithms/Homology.h"

template < class Toplex, class CellContainer >
Picture * draw_morse_sets (const int Width, const int Height,
                           const Toplex & my_toplex, 
                           const ConleyMorseGraph < CellContainer , Conley_Index_t > & conley_morse_graph ) {
  typedef ConleyMorseGraph < CellContainer , Conley_Index_t > CMG;
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  // Prepare variables for bounds finding loop
  Picture::Real x_min = bounds . upper_bounds [ 0 ];
  Picture::Real x_max = bounds . lower_bounds [ 0 ];
  Picture::Real y_min = bounds . upper_bounds [ 1 ];
  Picture::Real y_max = bounds . lower_bounds [ 1 ];
  

  // Loop Through Morse Sets to determine bounds
  typedef typename CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    // Draw the Morse Set
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    for ( typename CellContainer::const_iterator cellit = my_subset . begin (); 
         cellit != my_subset . end (); ++ cellit ) {
      typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *cellit ) );
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
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    if ( my_subset . size () < 10 ) std::cout << "Small Morse Set " << count << ": ";
    ++ count;

    for ( typename CellContainer::const_iterator cellit = my_subset . begin (); 
         cellit != my_subset . end (); ++ cellit ) {

      typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *cellit ) );

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

template < class Toplex, class CellContainer >
Picture * draw_toplex (const int Width, const int Height,
                       const Toplex & my_toplex ) {
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  // Prepare variables for bounds finding loop
  Picture::Real x_min = bounds . lower_bounds [ 0 ];
  Picture::Real x_max = bounds . upper_bounds [ 0 ];
  Picture::Real y_min = bounds . lower_bounds [ 1 ];
  Picture::Real y_max = bounds . upper_bounds [ 1 ];
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Loop through top cells to draw them
  CellContainer my_subset;
  std::insert_iterator<CellContainer> ii ( my_subset, my_subset . begin () );
  my_toplex . cover ( ii, my_toplex . bounds () );
  for ( typename CellContainer::const_iterator cellit = my_subset . begin (); 
       cellit != my_subset . end (); ++ cellit ) {
    // Draw the outline of the cell
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *cellit ) );
    Picture::Real volume = ( box . upper_bounds [ 0 ] - box . lower_bounds [ 0 ] )*( box . upper_bounds [ 1 ] - box . lower_bounds [ 1 ] ) ;
    Picture::Real total_volume = ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] )*( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) ;
    Picture::Real ratio = total_volume / volume;
    Picture::Real log_of_ratio = (16.0f - log ( ratio ) / log ( 4.0f) ) * 16.0f;
    Red = Green = Blue = (unsigned char) log_of_ratio;
    picture -> draw_square (Red, Green, Blue,
                                    box . lower_bounds [ 0 ],
                                    box . upper_bounds [ 0 ],
                                    box . lower_bounds [ 1 ],
                                    box . upper_bounds [ 1 ],
                            false);
  }


  return picture;
} /* draw_toplex */

template < class Toplex, class CellContainer >
Picture * draw_toplex_and_morse_sets (const int Width, const int Height,
                       const Toplex & my_toplex,
                      const ConleyMorseGraph < CellContainer, Conley_Index_t > & conley_morse_graph ) {
  typedef ConleyMorseGraph < CellContainer , Conley_Index_t > CMG;
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  // Prepare variables for bounds finding loop
  Picture::Real x_min = bounds . lower_bounds [ 0 ];
  Picture::Real x_max = bounds . upper_bounds [ 0 ];
  Picture::Real y_min = bounds . lower_bounds [ 1 ];
  Picture::Real y_max = bounds . upper_bounds [ 1 ];
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Loop through top cells to draw them
  CellContainer my_subset;
  std::insert_iterator<CellContainer> ii ( my_subset, my_subset . begin () );
  my_toplex . cover ( ii, my_toplex . bounds () );
  
  BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
    // Draw the outline of the cell
    unsigned char Red = rand () % 255;
    unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( cell ) );
    Picture::Real volume = ( box . upper_bounds [ 0 ] - box . lower_bounds [ 0 ] )*( box . upper_bounds [ 1 ] - box . lower_bounds [ 1 ] ) ;
    Picture::Real total_volume = ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] )*( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] ) ;
    Picture::Real ratio = total_volume / volume;
    Picture::Real log_of_ratio = (16.0f - log ( ratio ) / log ( 4.0f) ) * 16.0f;
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
  typedef typename CMG::VertexIterator VI;
  VI it, stop;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices ();
       it != stop;
       ++ it ) {
    unsigned char Red = 255; // rand () % 255;
    unsigned char Green = 255; // rand () % 255;
    unsigned char Blue = 255; //rand () % 255;
    // Draw the Morse Set
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
      typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( cell ) );
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
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    BOOST_FOREACH ( typename Toplex::Top_Cell cell, my_subset ) {
      typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( cell ) );
      picture -> draw_square (Red, Green, Blue,
                              box . lower_bounds [ 0 ],
                              box . upper_bounds [ 0 ],
                              box . lower_bounds [ 1 ],
                              box . upper_bounds [ 1 ]);
    }
  }  
  return picture;
} /* draw_toplex_and_morse_sets */
