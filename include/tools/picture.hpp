// picture.hpp
// a tool for converting 2D adaptive cubical complexes into .png files

#include <algorithm>

template < class Toplex >
Picture * draw_picture (const int Width, const int Height,
			unsigned char Red, unsigned char Green, unsigned char Blue,
			const Toplex & my_toplex, const typename Toplex::Subset & my_subset ) {
  typename Toplex::Geometric_Description bounds = my_toplex . bounds ();
  // Prepare variables for bounds finding loop
  Picture::Real x_min = bounds . upper_bounds [ 0 ];
  Picture::Real x_max = bounds . lower_bounds [ 0 ];
  Picture::Real y_min = bounds . upper_bounds [ 1 ];
  Picture::Real y_max = bounds . lower_bounds [ 1 ];
  
  // Find bounds of picture
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
	it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    x_min = std::min ( x_min, box . lower_bounds [ 0 ] );
    x_max = std::max ( x_max, box . upper_bounds [ 0 ] );
    y_min = std::min ( y_min, box . lower_bounds [ 1 ] );
    y_max = std::max ( y_max, box . upper_bounds [ 1 ] );
  }
  
  // Create picture
  Picture * picture = new Picture( Width, Height, x_min, x_max, y_min, y_max );
  
  // Draw picture
  for ( typename Toplex::Subset::const_iterator it = my_subset . begin (); 
	it != my_subset . end (); ++ it ) {
    typename Toplex::Geometric_Description box = my_toplex . geometry ( my_toplex . find ( *it ) );
    picture -> draw_square (Red, Green, Blue,
                            box . lower_bounds [ 0 ],
                            box . upper_bounds [ 0 ],
                            box . lower_bounds [ 1 ],
                            box . upper_bounds [ 1 ]);
                           
  }
  
  return picture;
} /* draw_bitmap */

