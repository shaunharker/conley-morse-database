// HEADERS FOR DEALING WITH PICTURES
#ifndef CMDB_SINGLEOUTPUT_H
#define CMDB_SINGLEOUTPUT_H

#include "database/tools/picture.h"

#include "database/structures/Conley_Morse_Graph.h"
#include "database/structures/Grid.h"

/***********/
/* Output  */
/***********/
//void DrawMorseSets ( const Grid & phase_space, const CMG & conley_morse_graph );
//void CreateDotFile ( const MorseGraph & cmg );
//void output_cubes ( const Grid & my_grid, const MorseGraph & conley_morse_graph );

inline void DrawMorseSets ( const Grid & phase_space, const MorseGraph & conley_morse_graph ) {
  // Create a Picture
  int Width =  4096;
  int Height = 4096;
  
  Picture * picture = draw_morse_sets( Width, Height, phase_space, conley_morse_graph );
  Picture * picture2 = draw_grid ( Width, Height, phase_space );
  Picture * picture3 = draw_grid_and_morse_sets( Width, Height, phase_space, conley_morse_graph );
  
  std::cout << "Saving output... ";
  std::cout . flush ();
  //picture -> saveAsPNG ( "morse_sets.png" );
  //picture2 -> saveAsPNG ( "grid.png" );
  //picture3 -> saveAsPNG ( "grid_and_morse.png" );
  
  picture -> saveAsBMP ( "morse_sets.bmp" );
  picture2 -> saveAsBMP ( "grid.bmp" );
  picture3 -> saveAsBMP ( "grid_and_morse.bmp" );
  std::cout << "Output saved.\n";
  
  //picture -> saveAsTIFF ( "morse_sets.tiff" );
  //picture2 -> saveAsTIFF ( "grid.tiff" );
  //picture3 -> saveAsTIFF ( "grid_and_morse.tiff" );

  delete picture;
  delete picture2;
  delete picture3;
}



inline void CreateDotFile ( const MorseGraph & cmg ) {
  typedef MorseGraph::Vertex V;
  typedef MorseGraph::Edge E;
  typedef MorseGraph::VertexIterator VI;
  typedef MorseGraph::EdgeIterator EI;
  
  std::ofstream outfile ("morsegraph.gv");
  
  outfile << "digraph G { \n";
  //outfile << "node [ shape = point, color=black  ];\n";
  //outfile << "edge [ color=red  ];\n";
  
  // LOOP THROUGH VERTICES AND GIVE THEM NAMES
  std::map < V, int > vertex_to_index;
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
    vertex_to_index [ *start ] = i;
    outfile << i << " [label=\""<< cmg . grid (*start) -> size () << "\"]\n";
    ++ i;
  }
  int N = cmg . NumVertices ();
  
  // LOOP THROUGH MorseGraph EDGES
  EI estart, estop;
  typedef std::pair<int, int> int_pair;
  std::set < int_pair > edges;
  for (boost::tie ( estart, estop ) = cmg . Edges ();
       estart != estop;
       ++ estart ) {
    V source = estart -> first;
    V target = estart -> second;
    int index_source = vertex_to_index [ source ];
    int index_target = vertex_to_index [ target ];
    if ( index_source != index_target ) // Cull the self-edges
      edges . insert ( std::make_pair ( index_source, index_target ) );
  }
  // TRANSITIVE REDUCTION (n^5, non-optimal)
  // We determine those edges (a, c) for which there are edges (a, b) and (b, c)
  // and store them in "transitive_edges"
  std::set < int_pair > transitive_edges;
  BOOST_FOREACH ( int_pair edge, edges ) {
    for ( int j = 0; j < N; ++ j ) {
      bool left = false;
      bool right = false;
      BOOST_FOREACH ( int_pair edge2, edges ) {
        if ( edge2 . first == edge . first && edge2 . second == j ) left = true;
        if ( edge2 . first == j && edge2 . second == edge . second ) right = true;
      }
      if ( left && right ) transitive_edges . insert ( edge );
    }
  }
  
  // PRINT OUT EDGES OF TRANSITIVE CLOSURE
  BOOST_FOREACH ( int_pair edge, edges ) {
    if ( transitive_edges . count ( edge ) == 0 )
      outfile << edge . first << " -> " << edge . second << ";\n";
  }
  
  edges . clear ();
  transitive_edges . clear ();
  
  outfile << "}\n";
  
  outfile . close ();
  
}

#if 0
inline void output_cubes ( const Grid & my_grid,
                   const MorseGraph & conley_morse_graph ) {
  using namespace chomp;
  
  // Loop Through Morse Sets to determine bounds
  typedef typename MorseGraph::VertexIterator VI;
  VI it, stop;
  std::vector < std::vector < uint32_t > > cubes;
  for (boost::tie ( it, stop ) = conley_morse_graph . Vertices (); it != stop; ++ it ) {
    CellContainer const & my_subset = conley_morse_graph . CellSet ( *it );
    int depth = my_grid . getDepth ( my_subset );
    BOOST_FOREACH ( const Grid::GridElement & ge, my_subset ) {
      my_grid . GridElementToCubes ( & cubes, ge, depth  );
    }
  }
  std::ofstream outfile ( "morsecubes.txt" );
  for ( uint32_t i = 0; i < cubes . size (); ++ i ) {
    for ( uint32_t j = 0; j < cubes [ i ] . size (); ++ j ) {
      outfile << cubes [ i ] [ j ] << " ";
    }
    outfile << "\n";
  }
  outfile . close ();
} /* output_cubes */
#endif

#endif


