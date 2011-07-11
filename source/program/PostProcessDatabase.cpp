/*
 *  PostProcessDatabase.cpp
 */


#include <iostream>
#include "data_structures/Database.h"

#include "boost/foreach.hpp"

#include "tools/visualization.h"

void ContinuationClasses ( const Database & database ) {
  int count;
  double l0 = 1000;
  double u0 = -1000;
  double l1 = 1000;
  double u1 = -1000;
  count = 0;
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    ++ count;
    l0 = std::min ( l0, record . ge_ . lower_bounds_ [ 0 ] );
    u0 = std::max ( u0, record . ge_ . upper_bounds_ [ 0 ] );
    l1 = std::min ( l1, record . ge_ . lower_bounds_ [ 1 ] );
    u1 = std::max ( u1, record . ge_ . upper_bounds_ [ 1 ] );
  }
  std::cout << "There are " << count << " parameter box records\n";
  std::cout << "[" << l0 << ", " << u0 << "] x [" << l1 << ", " << u1 << "]\n";
  
  std::vector < ParameterBoxRecord > vec_box_records;
  GraphicsWindow window ( "Parameter Space\n" );
  BOOST_FOREACH ( const ParameterBoxRecord & record, database . box_records () ) {
    vec_box_records . push_back ( record );
    double x = (record . ge_ . lower_bounds_ [ 0 ] - l0 ) / ( u0 - l0 );
    double y = (record . ge_ . lower_bounds_ [ 1 ] - l1 ) / ( u1 - l1 );
    double w = (record . ge_ . upper_bounds_ [ 0 ] - record . ge_ . lower_bounds_ [ 0 ] ) / ( u0 - l0 );
    double h = (record . ge_ . upper_bounds_ [ 1 ] - record . ge_ . lower_bounds_ [ 1 ] ) / ( u1 - l1 );
    int color = rand () % 64 + 64;
    window . rect ( color, 
                   (int) ( 512.0 * x ), 
                   (int) ( 512.0 * y ), 
                   (int) ( 512.0 * w ), 
                   (int) ( 512.0 * h ) );
  }
  count = 0;
  BOOST_FOREACH ( const ClutchingRecord & record, database . clutch_records () ) {
    ++ count;
    if ( record . id1_ >= vec_box_records . size () ||
         record . id2_ >= vec_box_records . size ()) {
      std::cout << "Record corrupt -- id1_ = " << record . id1_ << "\n";
      std::cout << "Record corrupt -- id2_ = " << record . id2_ << "\n";
      continue;
    }
    double x1 = (vec_box_records [ record . id1_ ] . ge_ . lower_bounds_ [ 0 ] - l0 ) / ( u0 - l0 );
    double y1 = (vec_box_records [ record . id1_ ] . ge_ . lower_bounds_ [ 1 ] - l1 ) / ( u1 - l1 );
    double x2 = (vec_box_records [ record . id2_ ] . ge_ . lower_bounds_ [ 0 ] - l0 ) / ( u0 - l0 );
    double y2 = (vec_box_records [ record . id2_ ] . ge_ . lower_bounds_ [ 1 ] - l1 ) / ( u1 - l1 );
    window . line ( 240, 
                   (int) ( 512.0 * x1 ),
                   (int) ( 512.0 * y1 ),
                   (int) ( 512.0 * x2 ),
                   (int) ( 512.0 * y2 ) );
  }
  std::cout << "there are " << count << " clutching records \n";
  
  window . wait ();
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
