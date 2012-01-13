// Complex.h
// Shaun Harker
// 9/10/11


#ifndef CHOMP_COMPLEX_H
#define CHOMP_COMPLEX_H

#include <vector>
#include <utility>

#include "boost/foreach.hpp"

#include "chomp/Ring.h"
#include "chomp/Chain.h"
#include "chomp/HashIndexer.h"
#include "chomp/VectorIndexer.h"
#include "chomp/SparseMatrix.h"

class Complex {
protected:
  int dimension_;
  std::vector<Index> sizes_;
public:
  // Boundary and Coboundary Of Cells -- User Supplied.
  virtual void boundary ( Chain * output, const Index input, int dim ) const = 0;
  virtual void coboundary ( Chain * output, const Index input, int dim ) const = 0;

  // Boundary and Coboundary of Chains.
  virtual void boundary ( Chain * output, const Chain & input) const {
    int d = input . dimension ();
    BOOST_FOREACH ( const Term & term, input () ) {
      (*output) += term . coef () * boundary ( term . index (), d );
    }
    output -> dimension () = d - 1;
  }
  
  virtual void coboundary ( Chain * output, const Chain & input ) const {
    int d = input . dimension ();
    BOOST_FOREACH ( const Term & term, input () ) {
      (*output) += term . coef () * coboundary ( term . index (), d );
    }
    output -> dimension () = d + 1;
  }
  
  // Convenience Forms
  Chain boundary ( const Index input, int dim ) const {
    Chain output; boundary ( &output, input, dim ); return output; 
  }
  Chain coboundary ( const Index input, int dim ) const {
    Chain output; coboundary ( &output, input, dim ); return output; 
  }
  Chain boundary ( const Chain & input ) const {
    Chain output; boundary ( &output, input ); return output; 
  }
  Chain coboundary ( const Chain & input ) const {
    Chain output; coboundary ( &output, input ); return output; 
  }

  // Constructor
  Complex ( void ) : dimension_ (0) {}
  virtual ~Complex (void) {}
  
  // dimension accessor
  int & dimension ( void ) { return dimension_; }
  const int & dimension ( void ) const { return dimension_; }
  
  // size
  Index size ( int d ) const { 
    if ( d < 0 || d > dimension_ ) return 0;
    return sizes_ [ d ]; 
  }
  Index size ( void ) const { 
    Index result = 0;
    for ( int d = 0; d <= dimension (); ++ d ) result += size ( d );
    return result;
  }
  // reindex
  virtual void reindex ( std::vector<std::vector< Index > > & permute ) = 0;

};

typedef SparseMatrix<Ring> Matrix;

/// BoundaryMatrix
/// Creates a sparse matrix representation of the boundary map 
/// The columns are the boundaries of the d-dimensional cells.
/// output is returned in "output_matrix".
void BoundaryMatrix ( SparseMatrix < Ring > * output_matrix, 
                      const Complex & complex, 
                     const int d );

inline void BoundaryMatrix ( SparseMatrix < Ring > * output_matrix, 
                            const Complex & complex, 
                            const int d ) {
  uint32_t rows, columns;
  if ( d > 0 ) rows = complex . size ( d - 1 ); else rows = 0;
  if ( d <= complex . dimension () ) columns = complex . size ( d ); else columns = 0;
  output_matrix -> resize ( rows, columns );
  //std::cout << " d = " << d << "\n";
  //std::cout << " complex . dimension () = " << complex . dimension () << "\n";
  //std::cout << " rows = " << rows << " and columns = " << columns << "\n";
  if ( d >= 0 && d <= complex . dimension () )
  for ( Index i = 0; i < complex . size ( d ); ++ i ) {
		Chain bd = complex . boundary ( i, d );
    //std::cout << " boundary(" << i << ", " << d << ") = " << bd << "\n";
    BOOST_FOREACH ( const Term & t, bd () ) {
      //std::cout << "Write to " << t . index () << ", " << i << "\n";
      //std::cout << "value to write = " << t . coef () << "\n";
      output_matrix -> write ( t . index (), i, t . coef () ); 
    }
  } /* for */
} 

///  CHOMP_COMPLEX(Cell) Macro
///  This provides several pre-written functions to a class 
///    derived from Complex. The implementor should write
///      "CHOMP_COMPLEX(Cell)" as a line in his class definition, where
///       Cell is the class the implementor has written specifically for
///       the complex.
///
///  The methods provided by this macro are:
///    (1) startInserting
///    (2) insertCell
///    (3) finishedInserting
///    (4) indexToCell
///    (5) cellToIndex
///    (6) reindex
///   
///  In order to prepare the cell indexing, the following must be done:
///    (A) User calls "startInserting"
///    (B) User calls "insertCell" on all cells.
///    (C) User then calls "finishedInserting".
///    Now the complex is ready for use by algorithms using index interface
///  If one wants to reorder the indexing, the reindex method can accomplish 
///  this. One produces vector<vector<Index> > v where we wish
///  cell at (i,d)  --ends up as --> cell at ( v[d][i],d ) 
///
///  In order to write "bd" and "cbd" methods, the implementor will:
///    (A) Get the index of a cell as an argment
///    (B) Use "indexToCell" to convert it to a cell
///    (C) Apply a boundary/coboundary algorithm to the cell
///    (D) Use "cellToIndex" to convert the cells in the result to indexes
///  

#define CHOMP_COMPLEX(MyCell,MyIndexer)                         \
using Complex::boundary;                                        \
using Complex::coboundary;                                      \
typedef MyCell Cell;                                            \
typedef MyIndexer < Cell, Index > Indexer;                      \
std::vector < Indexer > indexers_;                              \
std::vector < std::vector < Cell > > init_cell_stack_;          \
void startInserting ( void ) {                                  \
  BOOST_FOREACH ( Indexer & indexer, indexers_ )                \
    indexer . clear ();                                         \
}                                                               \
void insertCell ( const Cell & c, const int dim ) {             \
  if ( (int) init_cell_stack_ . size () <= dim )                \
    init_cell_stack_ . resize ( dim + 1 );                      \
  if ( dim > dimension () )                                     \
    dimension () = dim;                                         \
  init_cell_stack_ [ dim ] . push_back ( c );                   \
}                                                               \
void finishedInserting ( void ) {                               \
  init_cell_stack_ . resize ( dimension () + 1 );               \
  indexers_ . resize ( dimension () + 1 );                      \
  sizes_ . resize ( dimension () + 1 );                         \
  for ( int d = 0; d <= dimension (); ++ d ) {                  \
    indexers_ [ d ] . initialize ( init_cell_stack_ [ d ] );    \
    sizes_ [ d ] = indexers_ [ d ] . size ();                   \
  }                                                             \
  init_cell_stack_ . clear ();                                  \
}                                                               \
Cell indexToCell ( const Index i, const int dim ) const {       \
  return indexers_ [ dim ] . key ( i );                         \
}                                                               \
Index cellToIndex ( const Cell & key, const int dim ) const {    \
  return indexers_ [ dim ] . rank ( key );                      \
}                                                               \
void reindex ( std::vector<std::vector< Index > > & permute ) { \
  for ( int d = 0; d <= dimension (); ++ d )                    \
    indexers_ [ d ] . reindex ( permute [ d ] );                \
} 


#endif

