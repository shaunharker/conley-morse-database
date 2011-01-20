// picture.cpp  
// tool for making 2D adaptive cubical complexes into .png files

#include "tools/picture.h"

/// Create a white 32-bit RGBA bitmap image of width Width and height Height
/// Must free it later
void white_bitmap (unsigned char * * bitmap, const int Width, const int Height ) {
  int size = 4 * Width * Height;
  *bitmap = new unsigned char [ size ];
  for ( int i = 0; i < size; ++ i ) ( *bitmap ) [ i ] = 255;
}

// Picture Definitions

Picture::Picture ( void ) { bitmap = NULL; }

Picture::Picture ( int Width, int Height, Real x_min, Real x_max, Real y_min, Real y_max ) :
Width(Width), Height(Height), x_min(x_min), x_max(x_max), y_min(y_min), y_max(y_max) {
  white_bitmap ( & bitmap, Width, Height );
}

Picture::~Picture ( void ) { if ( bitmap != NULL ) delete bitmap; }

void Picture::draw_square ( unsigned char Red, unsigned char Green, unsigned char Blue, 
                  Real draw_x_min, Real draw_x_max, Real draw_y_min, Real draw_y_max ) {
  int left = ( (draw_x_min - x_min) / (x_max - x_min) ) * (Real) Width;
  int right = ( (draw_x_max - x_min) / (x_max - x_min ) ) * (Real) Width;
  int bottom = ( (draw_y_min - y_min) / (y_max - y_min) ) * (Real) Height;
  int top = ( (draw_y_max - y_min) / (y_max - y_min ) )* (Real) Height;
  for ( int i = left; i < right; ++ i ) {
    for ( int j = bottom; j < top; ++ j ) {
      int offset = 4 * ( Width * (Height - j - 1) + i ); 
      // For transparency effect we take min
      bitmap [ offset + 0 ] = std::min ( bitmap [ offset + 0 ], Red);
      bitmap [ offset + 1 ] = std::min ( bitmap [ offset + 1 ], Green);
      bitmap [ offset + 2 ] = std::min ( bitmap [ offset + 2 ], Blue);
      bitmap [ offset + 3 ] = 255;
    }
  }
}

// copy a scaled version of A into output (which is initialized to some Height and Width
// and axes scales)
void copy_scaled_picture ( Picture * picture, const Picture & A ) {
  Picture & output = * picture;
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
      int offset = 4 * ( A . Width * ( A . Height - j - 1) + i );
      unsigned char Red = A . bitmap [ offset + 0 ];
      unsigned char Green = A . bitmap [ offset + 1 ]; 
      unsigned char Blue = A . bitmap [ offset + 2 ];
      picture -> draw_square ( Red, Green, Blue, x_left, x_right, y_left, y_right );
    } /* for j */
  } /* for i */
}

Picture * combine_pictures (const int Width, const int Height, 
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
  if ( B . x_min != B . x_max && B . y_min != B . y_max ) copy_scaled_picture ( picture, B );
  return picture;
} /* combine_pictures */

