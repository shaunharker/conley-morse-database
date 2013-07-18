// HEADERS FOR DEALING WITH PICTURES
#ifndef CMDB_SINGLEOUTPUT_H
#define CMDB_SINGLEOUTPUT_H

#include <sstream>

#include "database/tools/picture.h"

#include "database/structures/MorseGraph.h"
#include "database/structures/Grid.h"
#include "chomp/ConleyIndex.h"
#include "chomp/Matrix.h"
#include "chomp/PolyRing.h"

/***********/
/* Output  */
/***********/
//void DrawMorseSets ( const Grid & phase_space, const CMG & conley_morse_graph );
//void CreateDotFile ( const MorseGraph & cmg );
//void output_cubes ( const Grid & my_grid, const MorseGraph & conley_morse_graph );

inline std::string returnConleyIndex ( const chomp::ConleyIndex_t & ci );

inline void DrawMorseSets ( const Grid & phase_space, const MorseGraph & conley_morse_graph, const char *modelpath ) {
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
  
  std::string path=modelpath;
  std::string ext=".bmp";
  std::string filename;
  std::string fullname;
  const char * bmpfile;

  filename="/morse_sets";
  fullname=path+filename+ext;
  bmpfile = (char * ) fullname . c_str ( );
  picture -> saveAsBMP ( bmpfile );

  filename="/grid";
  fullname=path+filename+ext;
  bmpfile = (char * ) fullname . c_str ( );
  picture2 -> saveAsBMP ( bmpfile );

  filename="/grid_and_morse";
  fullname=path+filename+ext;
  bmpfile = (char * ) fullname . c_str ( );
  picture3 -> saveAsBMP ( bmpfile );
  std::cout << "Output saved.\n";
  
  //picture -> saveAsTIFF ( "morse_sets.tiff" );
  //picture2 -> saveAsTIFF ( "grid.tiff" );
  //picture3 -> saveAsTIFF ( "grid_and_morse.tiff" );

  delete picture;
  delete picture2;
  delete picture3;
}



inline void CreateDotFile ( const MorseGraph & cmg, const char * modelpath ) {
  typedef MorseGraph::Vertex V;
  typedef MorseGraph::Edge E;
  typedef MorseGraph::VertexIterator VI;
  typedef MorseGraph::EdgeIterator EI;
  
  std::string path=modelpath;
  std::string ext=".gv";
  std::string filename;
  std::string fullname;
  const char * gvfile;
  filename="/morsegraph";
  fullname=path+filename+ext;
  gvfile = (char * ) fullname . c_str ( );

  std::ofstream outfile (gvfile);
  
  outfile << "digraph G { \n";
  //outfile << "node [ shape = point, color=black  ];\n";
  //outfile << "edge [ color=red  ];\n";
  
  // LOOP THROUGH VERTICES AND GIVE THEM NAMES
  std::map < V, int > vertex_to_index;
  VI start, stop;
  int i = 0;
  for (boost::tie ( start, stop ) = cmg . Vertices (); start != stop; ++ start ) {
    vertex_to_index [ *start ] = i;
    //outfile << i << " [label=\""<< cmg . grid (*start) -> size () << "\"]\n";
    outfile << i << " [label=\""<< returnConleyIndex ( * cmg . conleyIndex ( *start ) ) << "\"]\n";
    ++ i;

    //std::cout << returnConleyIndex ( * cmg . conleyIndex ( *start ) );
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


inline std::string returnConleyIndex ( const chomp::ConleyIndex_t & ci ) {
  using namespace chomp;

  std::string resultstr;
  std::stringstream sstr;

  if ( ci . undefined () ) resultstr = "NaN";
  int biggest = 0;
  static int corrupt = 0;
  for ( unsigned int i = 0; i < ci . data () . size (); ++ i ) {
    typedef SparseMatrix < PolyRing < Ring > > PolyMatrix;
    if ( ( ci . data () [ i ] . number_of_rows () !=
          ci . data () [ i ] . number_of_columns () ) ||
        ci . data () [ i ] . number_of_rows () < 0 ) {
      std::cout << "return ConleyIndex : " << "Corrupt.\n";
      std::cout << ++ corrupt << "\n";
      continue;
    }
    
    PolyMatrix poly = ci . data () [ i ];
    
    int N = poly . number_of_rows ();
    PolyRing<Ring> X;
    X . resize ( 2 );
    X [ 1 ] = Ring ( -1 );
    for ( int i = 0; i < N; ++ i ) {
      poly . add ( i, i, X );
    }
    PolyMatrix U, Uinv, V, Vinv, D;
    //std::cout << "SNF:\n";
    SmithNormalForm ( &U, &Uinv, &V, &Vinv, &D, poly );
    //std::cout << "SNF complete.\n";
    bool is_trivial = true;
    PolyRing < Ring > x;
    x . resize ( 2 );
    x [ 1 ] = Ring ( 1 );
    for ( int j = 0; j < D . number_of_rows (); ++ j ) {
      //std::stringstream ss; //
      PolyRing < Ring > entry = D . read ( j, j );
      while ( ( entry . degree () >= 0 )
             && ( entry [ 0 ] == Ring ( 0 ) )) {
        entry = entry / x;
      }
      if ( entry . degree () <= 0 ) continue;
      is_trivial = false;
      //ss << "   " << entry << "\n";
      sstr << entry;
      resultstr = sstr.str();
      if ( entry . degree () > biggest ) biggest = entry . degree ();
    }
    if ( is_trivial ) resultstr="0";
  }

  //return biggest;


  //std::cout << "Biggest " << biggest << "\n";
  return resultstr;

}

/*
int check_index ( std::ostream & outstream, const chomp::ConleyIndex_t & ci ) {
  using namespace chomp;
  if ( ci . undefined () ) outstream << "Conley Index not computed for this Morse Set.\n";
  int biggest = 0;
  static int corrupt = 0;
  //std::cout << "check index.\n";
  //std::cout << "data . size () = " << ci . data () . size () << "\n";
  for ( unsigned int i = 0; i < ci . data () . size (); ++ i ) {
    typedef SparseMatrix < PolyRing < Ring > > PolyMatrix;
    if ( ( ci . data () [ i ] . number_of_rows () !=
          ci . data () [ i ] . number_of_columns () ) ||
        ci . data () [ i ] . number_of_rows () < 0 ) {
      outstream << "Corrupt.\n";
      outstream << ++ corrupt << "\n";
      continue;
    }
    //if ( ci . data () [ i ] . number_of_rows () < 4 ) continue;
    //std::cout << "Dimension " << i << "\n";
    //print_matrix ( ci . data () [ i ] );
    
    PolyMatrix poly = ci . data () [ i ];
    
    int N = poly . number_of_rows ();
    PolyRing<Ring> X;
    X . resize ( 2 );
    X [ 1 ] = Ring ( -1 );
    for ( int i = 0; i < N; ++ i ) {
      poly . add ( i, i, X );
    }
    PolyMatrix U, Uinv, V, Vinv, D;
    //std::cout << "SNF:\n";
    SmithNormalForm ( &U, &Uinv, &V, &Vinv, &D, poly );
    //std::cout << "SNF complete.\n";
    bool is_trivial = true;
    PolyRing < Ring > x;
    x . resize ( 2 );
    x [ 1 ] = Ring ( 1 );
    for ( int j = 0; j < D . number_of_rows (); ++ j ) {
      //std::stringstream ss; //
      PolyRing < Ring > entry = D . read ( j, j );
      while ( ( entry . degree () >= 0 )
             && ( entry [ 0 ] == Ring ( 0 ) )) {
        entry = entry / x;
      }
      if ( entry . degree () <= 0 ) continue;
      is_trivial = false;
      //ss << "   " << entry << "\n";
      outstream << entry << "\n";
      if ( entry . degree () > biggest ) biggest = entry . degree ();
    }
    if ( is_trivial ) outstream << "Trivial.\n";
  }
  return biggest;
}
*/

#endif


