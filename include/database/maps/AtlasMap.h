#ifndef AtlasMap_H
#define AtlasMap_H

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"
#include "database/structures/RectGeo.h"
#include "database/numerics/simple_interval.h"


class AtlasMap {

public:
  typedef simple_interval<double> interval;
	typedef uint64_t size_type;
  double lambda;
  AtlasMap ( void ) { lambda = 3.4; }
  AtlasMap ( double lambda ) : lambda(lambda) {}
	std::vector < AtlasGeo > operator ( ) ( boost::shared_ptr < Geo > geo_ptr ) const {
	 const AtlasGeo & geo = * boost::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
	 return operator ( ) ( geo );
	}

  std::vector < AtlasGeo > operator ( ) ( const AtlasGeo & geo ) const {
    std::vector < AtlasGeo > result;
    uint64_t id = geo . id ();
  	RectGeo rect = geo . rect ();
    interval x ( rect . lower_bounds [ 0 ], rect . upper_bounds [ 0 ] );
    /*
    0 0 par:p1;var:x;fun:p1*x*(1.0-0.5*x);
    0 1 par:p1;var:x;fun:p1*x*(1.0-0.5*x)-1.0;
    1 0 par:p1;var:x;fun:0.5*p1*(1.0-x*x);
    1 1 par:p1;var:x;fun:0.5*p1*(1.0-x*x)-1.0;
    */
    interval image0, image1;
    switch ( id ) {
      case 0:
        image0 = lambda * x * ( 1.0 - 0.5 * x );
        image1 = lambda * x * ( 1.0 - 0.5 * x ) - 1.0;
        break;
      case 1:
        image0 = 0.5 * lambda * ( 1.0 - x * x );
        image1 = 0.5 * lambda * ( 1.0 - x * x ) - 1.0;
        break;
      default:
        break;
    }
    RectGeo rect0 ( 1 ); 
    rect0 . lower_bounds [ 0 ] = image0 . lower ();  
    rect0 . upper_bounds [ 0 ] = image0 . upper ();
    RectGeo rect1 ( 1 ); 
    rect1 . lower_bounds [ 0 ] = image1 . lower ();
    rect1 . upper_bounds [ 0 ] = image1 . upper ();
    result . push_back ( AtlasGeo ( 0, rect0 ) );
    result . push_back ( AtlasGeo ( 1, rect1 ) );
    return result;
  }

  
};

#if 0
//#include "database/maps/CAPDMap.h"

/*
  void importMaps ( const RectGeo & param, const char * inputfile );

  void list_maps ( void ) {
    boost::unordered_map < size_type, std::vector < std::pair < size_type, Map > > >::iterator it;
    std::cout << "List of Maps : \n";
    for ( it=maps_.begin(); it!=maps_.end(); ++it ) {
      std::vector < std::pair < size_type, Map > > mymaps = it->second;
      for ( unsigned int i=0; i<mymaps.size(); ++i ) {
        std::cout << "Map from " << it->first << " to " << mymaps[i].first <<"\n";

      }
    }
  }
*/
// retrieve the maps needed
    std::vector < std::pair < size_type, Map > > mymaps = maps_ . find ( geo . id() ) -> second;
    // apply the different maps 
    for ( std::vector < std::pair < size_type, Map > >::iterator it=mymaps.begin(); it!=mymaps.end(); ++it ) {
      result . push_back ( AtlasGeo ( it -> first, it -> second ( geo . rect ( ) ) ) );   
    }

inline void AtlasMap::importMaps ( const RectGeo & rectangle, const char * inputfile ) { 
  std::ifstream ifile;
  std::string linestr;
  ifile . open ( inputfile );
  if ( ifile . is_open() ) {
    while ( std::getline ( ifile, linestr ) ) {
      Atlas::size_type id1, id2;
      std::stringstream ssline ( linestr );
      std::string mapstring;
      ssline >> id1 >> id2 >> mapstring;
      Map map ( rectangle, mapstring . c_str() );
      //
      std::pair < size_type, Map > pair ( id2, map );
      //
      boost::unordered_map < size_type, std::vector < std::pair < size_type, Map > > >::iterator it;
      //
      std::vector < std::pair < size_type, Map > > vect;
      it = maps_ . find ( id1 );
      if ( it == maps_ . end() ) {
        vect . push_back ( pair );
        std::pair < size_type, std::vector < std::pair < size_type, Map > > > newpair ( id1, vect );
        maps_ . insert ( newpair );
      } else { 
        vect = it -> second;
        vect . push_back ( pair );
        it -> second = vect;
      }
    }
    ifile . close ( );
    return;
  } else { 
    std::cout << "Could not read from the file : " << inputfile << "\n";
    return;
  }
}

private:
  boost::unordered_map < size_type, std::vector < std::pair < size_type, Map > > > maps_;

#endif

#endif