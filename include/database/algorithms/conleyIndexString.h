#ifndef CMDB_CONLEY_INDEX_STRING_H
#define CMDB_CONLEY_INDEX_STRING_H


#include "chomp/ConleyIndex.h"
#include "chomp/Matrix.h"
#include "chomp/PolyRing.h"
#include <boost/thread.hpp>
#include <boost/chrono/chrono_io.hpp>

typedef chomp::SparseMatrix < chomp::PolyRing < chomp::Ring > > PolyMatrix;

class SNFThread {

private:

  PolyMatrix * U;
  PolyMatrix * Uinv;
  PolyMatrix * V;
  PolyMatrix * Vinv;
  PolyMatrix * D;
  const PolyMatrix & A;
public:
  bool * result;
  SNFThread( PolyMatrix * U,
       PolyMatrix * Uinv,
       PolyMatrix * V,
       PolyMatrix * Vinv,
       PolyMatrix * D,
       const PolyMatrix & A, 
       bool * result ) 
  : U(U), Uinv(Uinv), V(V), Vinv(Vinv), D(D), A(A), result(result) {}

  void operator () ( void ) {
    try {
      SmithNormalForm ( U, Uinv, V, Vinv, D, A );
      *result = true;
    } catch ( ... /* boost::thread_interrupted& */) {
      *result = false;
    }
  }
};

std::vector<std::string> conleyIndexString ( const chomp::ConleyIndex_t & ci, int time_out = 180 ) {
  using namespace chomp;
  std::cout << "conley index string\n";
  std::vector<std::string> result;
  if ( ci . undefined () ) { 
    std::cout << "undefined.\n";
    return result;
  }
  for ( unsigned int i = 0; i < ci . data () . size (); ++ i ) {
    std::cout << "dimension is " << i << "\n";
    std::stringstream ss;
    PolyMatrix poly = ci . data () [ i ];
    
    int N = poly . number_of_rows ();
    PolyRing<Ring> X;
    X . resize ( 2 );
    X [ 1 ] = Ring ( -1 );
    for ( int i = 0; i < N; ++ i ) {
      poly . add ( i, i, X );
    }
    PolyMatrix U, Uinv, V, Vinv, D;

    // use a thread to perform the following line:    
    //      SmithNormalForm ( &U, &Uinv, &V, &Vinv, &D, poly );
    bool computed;
    SNFThread snf ( &U, &Uinv, &V, &Vinv, &D, poly, &computed );
    boost::thread t(snf);
    if ( not t . try_join_for ( boost::chrono::seconds( time_out ) ) ) {
      t.interrupt();
      t.join();
    }
    if ( not computed ) {
      result . push_back ( std::string ( "Problem computing SNF.\n") );
      continue;
    }
    // end threading

    bool is_trivial = true;
    PolyRing < Ring > x;
    x . resize ( 2 );
    x [ 1 ] = Ring ( 1 );
    for ( int j = 0; j < D . number_of_rows (); ++ j ) {
      PolyRing < Ring > entry = D . read ( j, j );
      while ( ( entry . degree () >= 0 )
             && ( entry [ 0 ] == Ring ( 0 ) )) {
        entry = entry / x;
      }
      if ( entry . degree () <= 0 ) continue;
      is_trivial = false;
      ss << "   " << entry << "\n";
    }
    if ( is_trivial ) ss << "Trivial.\n";
    result . push_back ( ss . str () );
    std::cout << "Wrote the poly " << ss . str () << "\n";
  }
  return result;
}

#endif