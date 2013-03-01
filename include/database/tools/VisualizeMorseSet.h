#ifndef CMDB_VISUALIZEMORSESET_H
#define CMDB_VISUALIZEMORSESET_H

// Declaration
template < class Grid, class GridElement >
visualizeMorseSet 
  (const std::vector < GridElement > & set,
   const Grid & grid );

// Definition
template < class Grid, class GridElement >
visualizeMorseSet 
  (const std::vector < GridElement > & set,
   const Grid & grid) {
  //TODO MAKE BELOW WORK
    
    
    // Construct uniform measure
    boost::unordered_map < typename Toplex::Top_Cell, double > measure;
    double uniform = 1.0 / (double) newset . size ();
    BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
      measure [ t ] = uniform;
    }
    
    // Random "bleed-test" choice
#if 0
    size_t N = newset . size ();
    size_t i = 0;
    size_t choice = rand () % N;
    BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
      measure [ t ] = 0.0;//uniform;
      if ( i ++ == choice ) measure [ t ] = 1.0;
    }
#endif
    
    // Iterate through out-edges several times
    CImgDisplay display(512,512);
    //display . wait ();
    for ( int iterate = 0; iterate < 10; ++ iterate ) {
      std::cout << "iterate " << iterate << "\n";
      boost::unordered_map < typename Toplex::Top_Cell, double > newmeasure;
      BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
        chomp::Rect box = phase_space -> geometry ( t );
        std::vector < typename Toplex::Top_Cell > cover_of_box;
        std::insert_iterator < std::vector < typename Toplex::Top_Cell > > ii ( cover_of_box, cover_of_box . begin () );
        phase_space -> cover ( ii, box );
        std::vector < typename Toplex::Top_Cell > restricted_cover_of_box; // restrict to newset
        BOOST_FOREACH ( typename Toplex::Top_Cell s, cover_of_box ) {
          if ( measure . count ( s ) ) restricted_cover_of_box . push_back ( s );
        }
        double factor = 1.0 / (double) restricted_cover_of_box . size ();
        BOOST_FOREACH ( typename Toplex::Top_Cell s, restricted_cover_of_box ) {
          newmeasure [ s ] += measure [ t ] * factor;
        }
      }
      std::swap ( measure, newmeasure );
      // }
      
      // Get bounds.
      chomp::Rect outerbounds = phase_space -> bounds ();
      chomp::Rect bounds ( 2 );
      bounds . lower_bounds [ 0 ] = outerbounds . upper_bounds [ 0 ];
      bounds . upper_bounds [ 0 ] = outerbounds . lower_bounds [ 0 ];
      bounds . lower_bounds [ 1 ] = outerbounds . upper_bounds [ 1 ];
      bounds . upper_bounds [ 1 ] = outerbounds . lower_bounds [ 1 ];
      BOOST_FOREACH ( typename Toplex::Top_Cell t, newset ) {
        chomp::Rect box = phase_space -> geometry ( t );
        bounds . lower_bounds [ 0 ] = std::min ( bounds . lower_bounds [ 0 ], box . lower_bounds [ 0 ]);
        bounds . upper_bounds [ 0 ] = std::max ( bounds . upper_bounds [ 0 ], box . upper_bounds [ 0 ]);
        bounds . lower_bounds [ 1 ] = std::min ( bounds . lower_bounds [ 1 ], box . lower_bounds [ 1 ]);
        bounds . upper_bounds [ 1 ] = std::max ( bounds . upper_bounds [ 1 ], box . upper_bounds [ 1 ]);
      }
      
      // Draw Grid Elements
      int Width = 512;//2560;//1024;
      int Height = 512;//1600;//1024;
      CImg<unsigned char> visual(Width,Height,1,3,0);
      typedef std::pair < typename Toplex::Top_Cell, double > value_t;
      double max_bright = 0.0;
      BOOST_FOREACH ( value_t val, measure ) {
        max_bright = std::max ( max_bright, val . second );
      }
      BOOST_FOREACH ( value_t val, measure ) {
        typename Toplex::Top_Cell t = val . first;
        double brightness = val . second;
        chomp::Rect box = phase_space -> geometry ( t );
        double x0, x1, y0, y1;
        x0 = ((double)Width) * (box . lower_bounds [ 0 ] - bounds . lower_bounds [ 0 ]) / ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] );
        x1 = ((double)Width) * (box . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ]) / ( bounds . upper_bounds [ 0 ] - bounds . lower_bounds [ 0 ] );
        y0 = ((double)Height) * (box . lower_bounds [ 1 ] - bounds . lower_bounds [ 1 ]) / ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] );
        y1 = ((double)Height) * (box . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ]) / ( bounds . upper_bounds [ 1 ] - bounds . lower_bounds [ 1 ] );
        int X0, X1, Y0, Y1;
        X0 = x0; X1 = x1; Y0 = y0; Y1 = y1;
        if ( X1 == X0 ) ++ X1;
        if ( Y1 == Y0 ) ++ Y1;
        if ( X0 > Width ) X0 = Width;
        if ( X1 > Width ) X1 = Width;
        if ( Y0 > Height ) Y0 = Height;
        if ( Y1 > Height ) Y1 = Height;
        for ( int i = X0; i < X1; ++ i ) {
          for ( int j = Y0; j < Y1; ++ j ) {
            visual ( i, Height-j-1, 0, 1 ) = 50;//255;
            visual ( i, Height-j-1, 0, 0 ) = (unsigned char) ( 255.0*(brightness/max_bright) );
          }
        }
      }
      display = visual;
    } // iterates
    
    //char ch;
    //std::cout << "Press a key then press enter: ";
    //std::cin  >> ch;
    //CImgDisplay display(visual,"Intensity profile");
    //display . wait ();
    
    //TODO MAKE ABOVE WORK
}
	      

#endif
