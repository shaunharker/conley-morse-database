#ifndef ATLASMAP_H
#define ATLASMAP_H

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"


template < class ChartMap >
class AtlasMap {

public:

	typedef uint64_t size_type;

  void addMap ( const size_type & chartid1, const size_type & chartid2, const ChartMap & map );

	void list_maps ( void ) const {
		typename boost::unordered_map < size_type, std::vector < std::pair < size_type, ChartMap > > >::iterator it;
		std::cout << "List of Maps : \n";
		for ( it=maps_.begin(); it!=maps_.end(); ++it ) {
			std::vector < std::pair < size_type, ChartMap > > mymaps = it->second;
			for ( unsigned int i=0; i<mymaps.size(); ++i ) {
				std::cout << "Map from " << it->first << " to " << mymaps[i].first <<"\n";

			}
		}
	}

  std::vector < AtlasGeo > operator ( ) ( const AtlasGeo & geo ) const {
  	std::vector < AtlasGeo > result;
  	// retrieve the maps needed
    if ( maps_ . find ( geo . id() ) != maps_ . end() ) { 
  	std::vector < std::pair < size_type, ChartMap > > mymaps = maps_ . find ( geo . id() ) -> second;

  	// apply the different maps 
  	for ( typename std::vector < std::pair < size_type, ChartMap > >::iterator it=mymaps.begin(); it!=mymaps.end(); ++it ) {
  		result . push_back ( it -> second ( geo ) );  	
  	}
    }
	return result;
  }

  std::vector < AtlasGeo > operator ( ) ( boost::shared_ptr < Geo > geo_ptr ) const {
    const AtlasGeo & geo = * boost::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
    return operator ( ) ( geo );
  }

  bool good ( void ) const { return true; }
private:
  boost::unordered_map < size_type, std::vector < std::pair < size_type, ChartMap > > > maps_;  
};


template < class ChartMap >
void AtlasMap<ChartMap>::addMap ( const size_type & chartid1, 
                               const size_type & chartid2, 
                               const ChartMap & map ) {

  std::pair < size_type, ChartMap > pair ( chartid2, map );
  //
  typename boost::unordered_map < size_type, std::vector < std::pair < size_type, ChartMap > > >::iterator it;
  //
  std::vector < std::pair < size_type, ChartMap > > vect;
  it = maps_ . find ( chartid1 );
  if ( it == maps_ . end() ) {
    vect . push_back ( pair );
    std::pair < size_type, std::vector < std::pair < size_type, ChartMap > > > newpair ( chartid1, vect );
    maps_ . insert ( newpair );
  } else { 
    vect = it -> second;
    vect . push_back ( pair );
    it -> second = vect;
  }

}

#endif