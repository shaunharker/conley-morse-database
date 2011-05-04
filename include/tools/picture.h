// picture.h

#ifndef CMPD_PICTURE
#define CMPD_PICTURE

/// Draw to RGBA bitmap
struct Picture {
  typedef double Real; // TODO, worry about this
  unsigned char * bitmap;
  int Width;
  int Height;
  Real x_min;
  Real x_max;
  Real y_min;
  Real y_max;
  Picture ( void );
  Picture ( int Width, int Height, Real x_min, Real x_max, Real y_min, Real y_max );
  ~Picture ( void );
  /// Draw a square around the specified locations of the given color
  ///    Does not overwrite -- rather, it takes the min of the color value of what
  ///    is being written and what is already there. This allows us to superpose images.
  void draw_square ( unsigned char Red, unsigned char Green, unsigned char Blue, 
                    Real draw_x_min, Real draw_x_max, Real draw_y_min, Real draw_y_max, bool transparent = true );
  void draw_square_outline ( unsigned char Red, unsigned char Green, unsigned char Blue, 
                    Real draw_x_min, Real draw_x_max, Real draw_y_min, Real draw_y_max );
};

/// Product a RGBA image of a toplex subset so that the top cells are colored
/// according to the values in Red, Green, and Blue. (Alpha is set to 255)
template < class Toplex >
Picture * draw_picture (const int Width, const int Height,
                        unsigned char Red, unsigned char Green, unsigned char Blue,
                        const Toplex & my_toplex, const typename Toplex::Subset & my_subset );

/// Copy the picture in "A" into "picture" by scaling it
void copy_scaled_picture ( Picture * picture, const Picture & A );

/// Determine bounds that fit both pictures A and B; then use
/// copy_scaled_picture to copy them into the same frame.
Picture * combine_pictures (const int Width, const int Height, 
                            const Picture & A, const Picture & B );




#include "tools/picture.hpp"

#endif
