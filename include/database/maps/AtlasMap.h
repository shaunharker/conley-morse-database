#ifndef ATLASMAP_H
#define ATLASMAP_H

#include <vector>
#include <utility>
#include "boost/unordered_map.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"
#include "database/maps/Map.h"
#include "database/structures/UnionGeo.h"


template < class ChartMap >
class AtlasMap : public Map {

public:

	typedef uint64_t size_type;

  void addMap ( const size_type & chartid1, const size_type & chartid2, const ChartMap & map );

	void list_maps ( void ) const {
		// typename boost::unordered_map < size_type, std::vector < std::pair < size_type, ChartMap > > >::iterator it;
		std::cout << "List of Maps : \n";
		for ( typename boost::unordered_map < size_type, std::vector < std::pair < size_type, ChartMap > > >::const_iterator it=maps_.begin(); it!=maps_.end(); ++it ) {
			std::vector < std::pair < size_type, ChartMap > > mymaps = it->second;
			for ( unsigned int i=0; i<mymaps.size(); ++i ) {
				std::cout << "Map from " << it->first << " to " << mymaps[i].first <<"\n";

			}
		}
	}

  boost::shared_ptr<Geo> operator () ( const AtlasGeo & geo ) const {
    boost::shared_ptr<UnionGeo> result;
    size_t domain_id = geo . id ();
    const RectGeo & rect = geo . rect ();
    // Apply the different maps 
    if ( maps_ . count ( domain_id ) != 0 ) { 
      const std::vector < std::pair < size_type, ChartMap > > & mymaps = 
        maps_ . find ( domain_id ) -> second;
      typedef std::pair<size_type, ChartMap> sizeTypeChartPair;
      BOOST_FOREACH ( const sizeTypeChartPair & pair, mymaps ) {
        boost::shared_ptr<Geo> image_geo 
          ( new AtlasGeo ( pair . first, pair . second ( rect ) ) );
        result -> insert ( image_geo );
      }
    }
    return result;
  }

  boost::shared_ptr<Geo> operator () ( boost::shared_ptr < Geo > geo_ptr ) const {
    const AtlasGeo & geo = * boost::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
    return operator ( ) ( geo );
  }

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
  it = maps_ . find ( chartid1 );
  if ( it == maps_ . end() ) {
    std::vector < std::pair < size_type, ChartMap > > vect;
    vect . push_back ( pair );
    std::pair < size_type, std::vector < std::pair < size_type, ChartMap > > > newpair ( chartid1, vect );
    maps_ . insert ( newpair );
  } else { 
    std::vector < std::pair < size_type, ChartMap > > & vect = it -> second;
    vect . push_back ( pair );
  }

}

#endif