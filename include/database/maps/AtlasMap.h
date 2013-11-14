#ifndef AtlasMap_H
#define AtlasMap_H

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"

// #include "database/structures/Map.h"

#include "ModelMap.h"

class AtlasMap {

public:

	typedef uint64_t size_type;

	void importMaps ( const chomp::Rect & param, const char * inputfile );

  void addMap ( const size_type & chartid1, const size_type & chartid2, const ModelMap & map );

	void list_maps ( void ) {
		boost::unordered_map < size_type, std::vector < std::pair < size_type, ModelMap > > >::iterator it;
		std::cout << "List of Maps : \n";
		for ( it=maps_.begin(); it!=maps_.end(); ++it ) {
			std::vector < std::pair < size_type, ModelMap > > mymaps = it->second;
			for ( unsigned int i=0; i<mymaps.size(); ++i ) {
				std::cout << "Map from " << it->first << " to " << mymaps[i].first <<"\n";

			}
		}
	}

  std::vector < AtlasGeo > operator ( ) ( const AtlasGeo & geo ) const {
  	std::vector < AtlasGeo > result;
  	// retrieve the maps needed
    if ( maps_ . find ( geo . id() ) != maps_ . end() ) { 
  	std::vector < std::pair < size_type, ModelMap > > mymaps = maps_ . find ( geo . id() ) -> second;

  	// apply the different maps 
  	for ( std::vector < std::pair < size_type, ModelMap > >::iterator it=mymaps.begin(); it!=mymaps.end(); ++it ) {
  		result . push_back ( it -> second ( geo ) );  	
  	}
    }
	return result;
  }

  std::vector < AtlasGeo > operator ( ) ( boost::shared_ptr < Geo > geo_ptr ) const {
    const AtlasGeo & geo = * boost::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
    return operator ( ) ( geo );
  }

private:
  boost::unordered_map < size_type, std::vector < std::pair < size_type, ModelMap > > > maps_;  
};


inline void AtlasMap::addMap ( const size_type & chartid1, const size_type & chartid2, const ModelMap & map ) {

  std::pair < size_type, ModelMap > pair ( chartid2, map );
  //
  boost::unordered_map < size_type, std::vector < std::pair < size_type, ModelMap > > >::iterator it;
  //
  std::vector < std::pair < size_type, ModelMap > > vect;
  it = maps_ . find ( chartid1 );
  if ( it == maps_ . end() ) {
    vect . push_back ( pair );
    std::pair < size_type, std::vector < std::pair < size_type, ModelMap > > > newpair ( chartid1, vect );
    maps_ . insert ( newpair );
  } else { 
    vect = it -> second;
    vect . push_back ( pair );
    it -> second = vect;
  }

}



inline void AtlasMap::importMaps ( const chomp::Rect & rectangle, const char * inputfile ) { 
  std::ifstream ifile;
  std::string linestr;
  ifile . open ( inputfile );
  if ( ifile . is_open() ) {
    while ( std::getline ( ifile, linestr ) ) {
      Atlas::size_type id1, id2;
      std::stringstream ssline ( linestr );
      std::string mapstring;
      ssline >> id1 >> id2 >> mapstring;
      ModelMap map ( rectangle, mapstring . c_str() );
      addMap ( id1, id2, map );
    }
    ifile . close ( );
    return;
  } else { 
    std::cout << "Could not read from the file : " << inputfile << "\n";
    return;
  }
}


#endif