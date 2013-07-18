// picture.h

#ifndef CMPD_PICTURE
#define CMPD_PICTURE

#include "CImg.h"
using namespace cimg_library;

/// Draw to RGBA bitmap
struct Picture {
  typedef double Real; // TODO, worry about this
  CImg<unsigned char> bitmap;
  int Width;
  int Height;
  Real x_min;
  Real x_max;
  Real y_min;
  Real y_max;
  
  void white_bitmap ( const int Width, const int Height );
  Picture ( void );
  Picture ( int Width, int Height, Real x_min, Real x_max, Real y_min, Real y_max );
  
  void saveAsPNG ( const char * filename ) const;
  void saveAsBMP ( const char * filename ) const;
  
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
template < class Toplex, class CellContainer >
Picture * draw_picture (const int Width, const int Height,
                        unsigned char Red, unsigned char Green, unsigned char Blue,
                        const Toplex & my_toplex, const CellContainer & my_subset );

/// Copy the picture in "A" into "picture" by scaling it
void copy_scaled_picture ( Picture * picture, const Picture & A );

/// Determine bounds that fit both pictures A and B; then use
/// copy_scaled_picture to copy them into the same frame.
Picture * combine_pictures (const int Width, const int Height, 
                            const Picture & A, const Picture & B );


// DEFINITIONS


/// Create a white 32-bit RGBA bitmap image of width Width and height Height
/// Must free it later
inline void Picture::white_bitmap ( const int Width, const int Height ) {
  bitmap . assign (Width,Height,1,3,255);
}

// Picture Definitions

inline Picture::Picture ( void ) {}

inline Picture::Picture ( int Width, int Height, Real x_min, Real x_max, Real y_min, Real y_max ) :
Width(Width), Height(Height), x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
  white_bitmap ( Width, Height );
}

inline void Picture::saveAsPNG ( const char * filename ) const {
  // Rotate the picture to get the axes right
  bitmap . save_png ( filename );
}

inline void Picture::saveAsBMP ( const char * filename ) const {
  bitmap . save_bmp ( filename );
}


inline void Picture::draw_square ( unsigned char Red, unsigned char Green, unsigned char Blue,
                           Real draw_x_min, Real draw_x_max, Real draw_y_min, Real draw_y_max, bool transparent ) {
  int left = (int) (( (draw_x_min - x_min) / (x_max - x_min) ) * (Real) Width );
  int right = (int) (( (draw_x_max - x_min) / (x_max - x_min ) ) * (Real) Width );
  // Images are no longer upside-down
  // old code : 
  //int bottom = (int) ( ( (draw_y_min - y_min) / (y_max - y_min) ) * (Real) Height );
  //int top = (int) ( ( (draw_y_max - y_min) / (y_max - y_min ) )* (Real) Height );
  int bottom = Height - (int) ( ( (draw_y_max - y_min) / (y_max - y_min) ) * (Real) Height );
  int top = Height - (int) ( ( (draw_y_min - y_min) / (y_max - y_min ) )* (Real) Height );

  if ( (left == right) && (right + 1 < Width) ) ++ right;
  if ( (bottom == top) && (top  + 1 < Height) ) ++ top;
  for ( int i = left; i < right; ++ i ) {
    for ( int j = bottom; j < top; ++ j ) {
      //int offset = 4 * ( Width * (Height - j - 1) + i );
      // For transparency effect we take min
      if ( transparent ) {
        bitmap ( i, j, 0, 0 ) = std::min ( bitmap ( i, j, 0, 0 ), Red);
        bitmap ( i, j, 0, 1 ) = std::min ( bitmap ( i, j, 0, 1 ), Green);
        bitmap ( i, j, 0, 2 ) = std::min ( bitmap ( i, j, 0, 2 ), Blue);

      } else {
        bitmap ( i, j, 0, 0 ) = Red;
        bitmap ( i, j, 0, 1 ) = Green;
        bitmap ( i, j, 0, 2 ) = Blue;
      }
    }
  }
}

inline void Picture::draw_square_outline ( unsigned char Red, unsigned char Green, unsigned char Blue,
                                   Real draw_x_min, Real draw_x_max, Real draw_y_min, Real draw_y_max ) {
  int left = (int) ( ( (draw_x_min - x_min) / (x_max - x_min) ) * (Real) Width );
  int right = (int) ( ( (draw_x_max - x_min) / (x_max - x_min ) ) * (Real) Width );
  // Images are no longer upside-down
  // old code : 
  //int bottom = (int) ( ( (draw_y_min - y_min) / (y_max - y_min) ) * (Real) Height );
  //int top = (int) ( ( (draw_y_max - y_min) / (y_max - y_min ) )* (Real) Height );
  int bottom = Height - (int) ( ( (draw_y_max - y_min) / (y_max - y_min) ) * (Real) Height );
  int top = Height - (int) ( ( (draw_y_min - y_min) / (y_max - y_min ) )* (Real) Height );

  if ( right + 1 < Width ) ++ right;
  if ( top  + 1 < Height ) ++ top;
  for ( int i = left; i < right; ++ i ) {
    for ( int j = bottom; j < top; ++ j ) {
      // if interior, skip to the end
      if ( i > left && i < (right - 1 ) ) if ( j > bottom ) j = top - 1;
      //int offset = 4 * ( Width * (Height - j - 1) + i );
      // For transparency effect we take min
      bitmap ( i, j, 0, 0 ) = std::min ( bitmap ( i, j, 0, 0 ), Red);
      bitmap ( i, j, 0, 1 ) = std::min ( bitmap ( i, j, 0, 1 ), Green);
      bitmap ( i, j, 0, 2 ) = std::min ( bitmap ( i, j, 0, 2 ), Blue);
    }
  }
}

// copy a scaled version of A into output (which is initialized to some Height and Width
// and axes scales)
inline void copy_scaled_picture ( Picture * picture, const Picture & A ) {
  /*
   std::cout << "scaling size " <<
   "[" << A . x_min << ", " << A . x_max << "] x ["
   << A . y_min << ", " << A . y_max << "] \n into          ["
   "[" << picture -> x_min << ", " << picture -> x_max << "] x ["
   << picture -> y_min << ", " << picture -> y_max << "]\n ";
   */
  for ( int i = 0; i < A . Width; ++ i ) {
    Picture::Real x_left = A . x_min + (A . x_max - A . x_min ) * ( (Picture::Real) i / (Picture::Real) A . Width );
    Picture::Real x_right = A . x_min + (A . x_max - A . x_min ) * ( (Picture::Real) (i+1) / (Picture::Real) A . Width );
    for ( int j = 0; j < A . Height; ++ j ) {
      Picture::Real y_left = A . y_min + (A . y_max - A . y_min ) * ( (Picture::Real) j / (Picture::Real) A . Height );
      Picture::Real y_right = A . y_min + (A . y_max - A . y_min ) * ( (Picture::Real) (j+1) / (Picture::Real) A . Height );
      //int offset = 4 * ( A . Width * ( A . Height - j - 1) + i );
      unsigned char Red = A . bitmap ( i, j, 0, 0 ); //bitmap [ offset + 0 ];
      unsigned char Green = A . bitmap ( i, j, 0, 1 ); //bitmap [ offset + 1 ];
      unsigned char Blue = A . bitmap ( i, j, 0, 2 ); //bitmap [ offset + 2 ];
      picture -> draw_square ( Red, Green, Blue, x_left, x_right, y_left, y_right );
    } /* for j */
  } /* for i */
}

inline Picture * combine_pictures (const int Width, const int Height,
                            const Picture & A, const Picture & B ) {
  Picture * picture = new Picture (Width, Height,
                                   std::min ( A . x_min, B . x_min ),
                                   std::max ( A . x_max, B . x_max ),
                                   std::min ( A . y_min, B . y_min ),
                                   std::max ( A . y_max, B . y_max ) );
  /*
   std::cout << "combining sizes " <<
   "[" << A . x_min << ", " << A . x_max << "] x ["
   << A . y_min << ", " << A . y_max << "] \n and          "
   "[" << B . x_min << ", " << B . x_max << "] x ["
   << B . y_min << ", " << B . y_max << "]\n ";
   */
  if ( A . x_min != A . x_max && A . y_min != A . y_max ) copy_scaled_picture ( picture, A );
  else std::cout << "skipping picture\n";
  if ( B . x_min != B . x_max && B . y_min != B . y_max ) copy_scaled_picture ( picture, B );
  else std::cout << "skipping picture\n";
  
  return picture;
} /* combine_pictures */




#include "database/tools/picture.hpp"

#endif
