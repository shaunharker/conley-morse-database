/*
 *  PostProcessDatabase.cpp
 */


#include <iostream>
#include "data_structures/Database.h"

int main ( int argc, char * argv [] ) {
  if ( argc != 2 ) {
    std::cout << "usage: PostProcessDatabase filename.dat\n";
    exit ( 1 );
  }
  Database database;
  database . load ( argv [ 1 ] );
  
  return 0;
}
