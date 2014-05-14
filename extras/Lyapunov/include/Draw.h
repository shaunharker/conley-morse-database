
#ifndef LYAPUNOV_DRAW_H
#define LYAPUNOV_DRAW_H

#include <vector>
#include <queue>
#include "boost/shared_ptr.hpp"
#include "CImg.h"
#include "database/tools/Picture.h"
#include "database/structures/TreeGrid.h"
#include "database/structures/RectGeo.h"

using namespace cimg_library;

inline
void draw2Dimage ( const std::vector<double> & data, 
									 boost::shared_ptr<const TreeGrid> grid ) {
	int N = grid -> size ();
	RectGeo bounds = grid -> bounds ();
	RectGeo outer_bounds = grid -> bounds ();
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		bounds.lower_bounds[d] = outer_bounds.upper_bounds[d];
		bounds.upper_bounds[d] = outer_bounds.lower_bounds[d];
	}
	for ( int i = 0; i < N; ++ i ) {
		if ( data [ i ] != 0.0 ) {
			RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    		(grid -> geometry ( Grid::iterator(i) ));
    	for ( int d= 0 ;  d < r . dimension (); ++ d ) {
    		bounds . lower_bounds [ d ] = std::min(bounds.lower_bounds[d],r.lower_bounds[d]);
    		bounds . upper_bounds [ d ] = std::max(bounds.upper_bounds[d],r.upper_bounds[d]);
    	}
    }
  }
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		if ( bounds.upper_bounds[d] < bounds.lower_bounds[d] ) 
			bounds.upper_bounds[d] = bounds.lower_bounds[d];
	}

	std::cout << " Bounds = " << bounds << "\n";
	Picture pic ( 4096, 4096, 
		bounds . lower_bounds [ 0 ],
		bounds . upper_bounds [ 0 ],
		bounds . lower_bounds [ 1 ],
		bounds . upper_bounds [ 1 ]);
  
  double max_data = 0.0;
  for ( int i = 0; i < N; ++ i ) {
  	if ( data [ i ] > max_data ) max_data = data [ i ];
  }
  if ( max_data == 0.0 ) max_data = 1.0;
  for ( int i = 0; i < N; ++ i ) {
  	if ( data [ i ] == 0.0 ) continue;
    RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    	(grid -> geometry ( Grid::iterator(i) ));
    int color = (int)((data [ i ]/max_data) * 255.0);
    pic . draw_square ( color, color, color, 
    	r . lower_bounds [ 0 ],
			r . upper_bounds [ 0 ],
			r . lower_bounds [ 1 ],
			r . upper_bounds [ 1 ],
			false /* no transparency */);
  }
 	pic . saveAsPNG ( "lyapunov.png" );
  pic.bitmap.display("Lyapunov Function");       
  
}

inline
void draw2Dimage ( const std::vector<bool> & data, 
									 boost::shared_ptr<const TreeGrid> grid ) {
	std::vector<double> double_data ( data.size (), 0.0 );
	for ( int i = 0; i < data . size (); ++ i ) {
		if ( data [ i ] ) double_data [ i ] = 1.0;
	}	
	draw2Dimage ( double_data, grid );
}

inline
void draw2DAttractorRepeller ( const std::vector<bool> & mis,
															 const std::vector<bool> & attractor,
 												       const std::vector<bool> & repeller,
									 boost::shared_ptr<const TreeGrid> grid ) {
	int N = grid -> size ();
	RectGeo bounds = grid -> bounds ();
	RectGeo outer_bounds = grid -> bounds ();
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		bounds.lower_bounds[d] = outer_bounds.upper_bounds[d];
		bounds.upper_bounds[d] = outer_bounds.lower_bounds[d];
	}
	for ( int i = 0; i < N; ++ i ) {
		//if ( repeller [ i ] || mis [ i ] || attractor [ i ] ) {
			RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    		(grid -> geometry ( Grid::iterator(i) ));
    	for ( int d= 0 ;  d < r . dimension (); ++ d ) {
    		bounds . lower_bounds [ d ] = std::min(bounds.lower_bounds[d],r.lower_bounds[d]);
    		bounds . upper_bounds [ d ] = std::max(bounds.upper_bounds[d],r.upper_bounds[d]);
    	}
    
  }
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		if ( bounds.upper_bounds[d] < bounds.lower_bounds[d] ) 
			bounds.upper_bounds[d] = bounds.lower_bounds[d];
	}

	std::cout << " Bounds = " << bounds << "\n";

	Picture pic ( 4096, 4096, 
		bounds . lower_bounds [ 0 ],
		bounds . upper_bounds [ 0 ],
		bounds . lower_bounds [ 1 ],
		bounds . upper_bounds [ 1 ]);
  
  for ( int i = 0; i < N; ++ i ) {
    RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    	(grid -> geometry ( Grid::iterator(i) ));
    int color = 0;
    int red = 0;
    int green = 0;
    int blue = 0;
    if ( not mis [ i ] && repeller [ i ] ) color = 1;
    if ( mis [ i ] && repeller [ i ] ) color = 2;
    if ( mis [ i ] && not repeller [ i ] ) color = 3;
    if ( mis [ i ] && attractor [ i ] ) color = 4;
 
    switch ( color ) {
    	case 0:
    		break;
    	case 1:
    		red = 128;
    		break;
    	case 2:
    		red = 255;
    		break;
    	case 3:
    		blue = 255;
    		break;
      case 4:
        green = 255;
    }
    pic . draw_square ( red, green, blue, 
    	r . lower_bounds [ 0 ],
			r . upper_bounds [ 0 ],
			r . lower_bounds [ 1 ],
			r . upper_bounds [ 1 ],
			false /* no transparency */);
  }
  pic.bitmap.display("Attractor/Repeller Pair");       
  
}

inline
void draw2DMorseSets ( const std::vector < std::deque < Grid::GridElement > > & components,
									 		 boost::shared_ptr<const TreeGrid> grid ) {
	int N = grid -> size ();
	RectGeo bounds = grid -> bounds ();
	RectGeo outer_bounds = grid -> bounds ();
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		bounds.lower_bounds[d] = outer_bounds.upper_bounds[d];
		bounds.upper_bounds[d] = outer_bounds.lower_bounds[d];
	}
	for ( int ms = 0; ms < components . size (); ++ ms ) {
		for ( int i = 0; i < components[ms].size(); ++ i ) {
			Grid::GridElement ge = components[ms][i];
			RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    		(grid -> geometry ( Grid::iterator(ge) ));
    	for ( int d= 0 ;  d < r . dimension (); ++ d ) {
    		bounds . lower_bounds [ d ] = std::min(bounds.lower_bounds[d],r.lower_bounds[d]);
    		bounds . upper_bounds [ d ] = std::max(bounds.upper_bounds[d],r.upper_bounds[d]);
    	}
    }
  }
  
	for ( int d = 0; d < bounds . dimension (); ++ d ) {
		if ( bounds.upper_bounds[d] < bounds.lower_bounds[d] ) 
			bounds.upper_bounds[d] = bounds.lower_bounds[d];
	}

	std::cout << " Bounds = " << bounds << "\n";

	Picture pic ( 4096, 4096, 
		bounds . lower_bounds [ 0 ],
		bounds . upper_bounds [ 0 ],
		bounds . lower_bounds [ 1 ],
		bounds . upper_bounds [ 1 ]);
  
// Loop through Morse Sets to draw them
  for ( int ms = 0; ms < components.size(); ++ ms ) {
  	unsigned char Red = rand () % 255;
   	unsigned char Green = rand () % 255;
    unsigned char Blue = rand () % 255;
  	for ( int i = 0; i < components[ms].size(); ++ i ) {
			Grid::GridElement ge = components[ms][i];
      RectGeo r = * boost::dynamic_pointer_cast<RectGeo> 
    			(grid -> geometry ( Grid::iterator(ge) ));

      pic . draw_square (Red, Green, Blue,
                              r . lower_bounds [ 0 ],
                              r . upper_bounds [ 0 ],
                              r . lower_bounds [ 1 ],
                              r . upper_bounds [ 1 ], 
                              false);
    }
  }
  pic.bitmap.display("Morse Sets");  
}

#endif

