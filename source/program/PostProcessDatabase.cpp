/*
 *  PostProcessDatabase.cpp
 */


#include <iostream>
#include "data_structures/Database.h"
#include "data_structures/UnionFind.hpp"

#include "boost/foreach.hpp"

#include "tools/visualization.h"

bool CheckIsomorphism (const ParameterBoxRecord & r1, 
                       const ParameterBoxRecord & r2,
                       const ClutchingRecord & c ) {
  if ( r1 . num_morse_sets_ != r2 . num_morse_sets_ ) return false;
  /* Method. First, generate a one-to-one correspondence from c. 
             If that cannot be done, return false.
             Next, use the correspondence to check for isomorphism by
             first translating each each in r1 to r2 and checking for its
             presence, and then vice-versa. */
  // cheat. for now, just check for a one-to-one
  std::map < int, int > forward;
  std::map < int, int > backward;
  typedef std::pair < int, int > Edge;
  BOOST_FOREACH ( const Edge & e, c . clutch_ ) {
    if ( forward . count ( e . first ) != 0 ) return false;
    if ( backward . count ( e . second ) != 0 ) return false;
    forward [ e . first ] = e . second;
    backward [ e . second ] = e . first;
  }
  // Now check for isomorphism
  return true;
}

void draw2DClasses (GraphicsWindow & window, 
                    std::map < int, int > & color_choice,
                    const Database & database,
                    const UnionFind < int > & classes ) {
  double l0 = 1000;
  double u0 = -1000;
  double l1 = 1000;
  double u1 = -1000;
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    l0 = std::min ( l0, record . ge_ . lower_bounds_ [ 0 ] );
    u0 = std::max ( u0, record . ge_ . upper_bounds_ [ 0 ] );
    l1 = std::min ( l1, record . ge_ . lower_bounds_ [ 1 ] );
    u1 = std::max ( u1, record . ge_ . upper_bounds_ [ 1 ] );
  }
  
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    double x = (record . ge_ . lower_bounds_ [ 0 ] - l0 ) / ( u0 - l0 );
    double y = (record . ge_ . lower_bounds_ [ 1 ] - l1 ) / ( u1 - l1 );
    double w = (record . ge_ . upper_bounds_ [ 0 ] - record . ge_ . lower_bounds_ [ 0 ] ) / ( u0 - l0 );
    double h = (record . ge_ . upper_bounds_ [ 1 ] - record . ge_ . lower_bounds_ [ 1 ] ) / ( u1 - l1 );
    window . rect ( color_choice [ classes . Representative ( record . id_ ) ], 
                   (int) ( 512.0 * x ), 
                   (int) ( 512.0 * y ), 
                   (int) ( 512.0 * w ), 
                   (int) ( 512.0 * h ) );
  }
  
  window . wait ();
}


void ContinuationClasses ( const Database & database ) {
  UnionFind < int > classes;
  std::map < int, ParameterBoxRecord > indexed_box_records;
  std::map < int, int> color_choice;
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    classes . Add ( record . id_ );
    color_choice [ record . id_ ] = rand () % 64 + 64;
    indexed_box_records [ record . id_ ] = record;
  }


  BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
     // CHECK IF ISOMORPHISM
    if ( CheckIsomorphism (indexed_box_records [ record . id1_ ], indexed_box_records [ record . id2_ ], record ) ) {
      classes . Union ( record . id1_, record . id2_ );
    }
  }

  GraphicsWindow window ( "Parameter Space\n" );
  draw2DClasses ( window, color_choice, database, classes );

}

int main ( int argc, char * argv [] ) {
  // Check command line arguments
  if ( argc != 2 ) {
    std::cout << "usage: PostProcessDatabase filename.dat\n";
    exit ( 1 );
  }
  
  // Create Database Object
  Database database;
  
  // Load Database File
  database . load ( argv [ 1 ] );

  // Compute Continuation Classes
  ContinuationClasses ( database );
  
  // Exit Program
  return 0;
}
