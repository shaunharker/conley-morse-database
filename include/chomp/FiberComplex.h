// FiberComplex.h
// Shaun Harker
// 9/16/11

#ifndef CHOMP_FIBERCOMPLEX_H
#define CHOMP_FIBERCOMPLEX_H

#include <vector>
#include "boost/unordered_set.hpp"
#include "boost/unordered_map.hpp"

#include "chomp/Complex.h"
#include "chomp/Chain.h"
#include "chomp/Preboundary.h"
#include "chomp/Closure.h"
#include "chomp/CubicalComplex.h"


// debug
#include "chomp/Draw.h"

class FiberComplex : public Complex {
public:
CHOMP_COMPLEX(Index,HashIndexer)
private:
  CubicalComplex * supercomplex_;
  Index choose_;
  std::vector < std::vector < bool > > data_;
  bool query ( const Index i, int d ) const {
    return data_ [ d ] [ i ];
  }
public:
  template < class PrismMap >
  FiberComplex ( const boost::unordered_set < Index > & X_nbs, 
                 const boost::unordered_set < Index > & A_nbs,
                 const CubicalComplex & domain,
                 CubicalComplex & codomain, 
                 const PrismMap & F );
    
  virtual ~FiberComplex ( void ) {}; 
  
  // Inclusion and Projection
  void include ( Chain * output, const Chain & input ) const;
  void project ( Chain * output, const Chain & input ) const;
  Index choose ( void ) const { return choose_; }
  // Boundary and Coboundary
  virtual void 
  boundary ( Chain * output, const Index input, int dim ) const;
  virtual void 
  coboundary ( Chain * output, const Index input, int dim ) const;
  
  virtual void 
  boundary ( Chain * output, const Chain & input ) const;  
  
  virtual void 
  coboundary ( Chain * output, const Chain & input ) const;
    
  Chain preboundary ( const Chain & input ) /* const */;
  
  //convenience
  Chain include ( const Chain & input ) const {
    Chain output; include ( &output, input ); return output;
  }
  Chain project ( const Chain & input ) const {
    Chain output; project ( &output, input ); return output;
  }  
  
  Prism geometry ( Index i, int dim ) const;

};
/********************
 *   DEFINITIONS    *
 ********************/

template < class PrismMap >
inline FiberComplex::FiberComplex
( const boost::unordered_set < Index > & X_nbs, 
  const boost::unordered_set < Index > & A_nbs,
  const CubicalComplex & domain,
  CubicalComplex & codomain, 
  const PrismMap & F ) {
  // Construct the fiber
  supercomplex_ = & codomain;
  int D = supercomplex_ -> dimension ();
  
  // Determine Images
  
  std::vector < boost::unordered_set < Index > > X_image ( D + 1 );
  std::vector < boost::unordered_set < Index > > A_image ( D + 1 );
  BOOST_FOREACH ( Index i, X_nbs ) {
    std::insert_iterator<boost::unordered_set < Index > > 
    ii ( X_image [ D ], X_image [ D ] . end () );
    codomain . cover ( ii, F ( domain . geometry ( i, D ) ) );
  }
  BOOST_FOREACH ( Index i, A_nbs ) {
    std::insert_iterator<boost::unordered_set < Index > > 
    ii ( A_image [ D ], A_image [ D ] . end () );
    codomain . cover ( ii, F ( domain . geometry ( i, D ) ) );
  }

  // Find the algebraic closures of X and A
  closure ( X_image, * supercomplex_ );
  closure ( A_image, * supercomplex_ );

  /*
   // DEBUG
  std::cout << "FiberComplex. Choose.\n";
  for ( int d = 0; d <= D; ++ d ) {
    std::cout << " size of X_image [ " << d << " = " << X_image [ d ] . size () << "\n";
  }
   */
  
  // Pick choose   (dirty: this makes sense only if A is empty )
#if 0
  // DEBUG //////////////////////////////////////
  if ( X_image [ 0 ]. size () == 0 ) {
    std::cout << "FiberComplex failure. The fiber doesn't even have a single vertex.\n";
    for ( int d = 0; d <= D; ++ d ) {
      std::cout << " size of X_image [ " << d << " ] = " << X_image [ d ] . size () << "\n";
    }
    {
      ComplexVisualization * cv = new ComplexVisualization ( "Codomain Picture." );
      std::cout << "About to draw complex.\n";
      cv -> drawComplex ( codomain, 100 );
      BOOST_FOREACH ( Index i, X_nbs ) {
        cv -> drawPrism ( domain . geometry ( i, D ), 200 );
        cv -> drawPrism ( F ( domain . geometry ( i, D ) ) , 200 );
      }
      // explore a minute here
      cv -> explore ();
      delete cv;
    }
    exit ( 1 );
  }
  ///////////////////////////////////////////////
#endif
  
  choose_ = * X_image [ 0 ] . begin ();
  
  // Initialize bitmap.
  data_ . resize ( D + 1 );
  for ( int d = 0; d <= D; ++ d ) {
    data_ [ d ] . resize ( supercomplex_ -> size ( d ), false );
    BOOST_FOREACH ( Index i, X_image [ d ] ) data_ [d][i] = true;
    BOOST_FOREACH ( Index i, A_image [ d ] ) data_ [d][i] = false;
  }
  
  // Initialize complex
  startInserting ();
  for ( int d = 0; d <= D; ++ d ) {
    Index N = supercomplex_ -> size ( d );
    for ( Index i = 0; i < N; ++ i ) {
      if ( data_ [ d ] [ i ] ) insertCell ( i, d );
    }
  }
  finishedInserting ();
  
}

inline void FiberComplex::include ( Chain * output, 
                                 const Chain & input ) const {
  int dim = input . dimension ();
  BOOST_FOREACH ( Term term, input () ) {
    term . index () = indexToCell ( term . index (), dim );
    (*output) += term;
  }
  output -> dimension () = dim;
}

inline void FiberComplex::project ( Chain * output, 
                                 const Chain & input ) const {
  int dim = input . dimension ();
  BOOST_FOREACH ( Term term, input () ) {
    if ( query ( term . index (), dim ) ) {
      term . index () = cellToIndex ( term . index (), dim );
      (*output) += term;
    }
  }
  output -> dimension () = dim;
}

inline Chain FiberComplex::preboundary 
( const Chain & input ) /* const */ {
  return SmithPreboundary ( input, *this );

  //std::cout << "FiberComplex preboundary, dim = " << input . dimension () <<"\n";
  MorseComplex reduction ( *this );
  Chain canon;
  Chain gamma;
  reduction . flow ( &canon, &gamma, input );
  //std::cout << "canon/gamma dim = " << canon . dimension () << ", " << gamma . dimension () << "\n";
  Chain lowered = reduction . project ( canon );
  //std::cout << "lowered.dim = " << lowered . dimension () << "\n";
  Chain morse_preboundary = SmithPreboundary ( lowered, reduction );
  //std::cout << "smithpreboundary.dim = " << morse_preboundary . dimension () << "\n";
  Chain lifted = reduction . lift ( morse_preboundary );
  //std::cout << "lifted.dim = " << lifted . dimension () << "\n";
  return ( lifted - gamma );
}

inline void FiberComplex::boundary 
( Chain * output, const Index input, int dim ) const {
  Chain input_chain;
  input_chain += Term ( input, Ring ( 1 ) );
  input_chain . dimension () = dim;
  boundary ( output, input_chain );
}

inline void FiberComplex::coboundary 
( Chain * output, const Index input, int dim ) const {
  Chain input_chain;
  input_chain += Term ( input, Ring ( 1 ) );
  input_chain . dimension () = dim;
  coboundary ( output, input_chain );
}

inline void FiberComplex::boundary 
( Chain * output, const Chain & input ) const {
  project ( output, supercomplex_ -> boundary ( include ( input ) ) );
}

inline void FiberComplex::coboundary 
( Chain * output, const Chain & input ) const {
  project ( output, supercomplex_ -> coboundary ( include ( input ) ) );
  
}

inline Prism FiberComplex::geometry ( Index i, int dim ) const {
  //std::cout << "FiberComplex::geometry ( " << i << ", " << dim << ")\n";
  return supercomplex_ -> geometry ( indexToCell ( i, dim ), dim );
}


#endif
