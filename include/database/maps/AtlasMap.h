#ifndef ATLASMAP_H
#define ATLASMAP_H

#include <vector>
#include <utility>
#include "boost/unordered_map.hpp"
#include <memory>
#include <boost/foreach.hpp>
#include "database/structures/Atlas.h"
#include "database/structures/Geo.h"
#include "database/maps/Map.h"
#include "database/structures/UnionGeo.h"

/// AtlasMap<ChartMap>
template < class ChartMap >
class AtlasMap : public Map {
public:
	typedef uint64_t size_type;
  typedef std::pair < size_type, ChartMap > IntChartMapPair;
  typedef std::vector<IntChartMapPair> IntChartMapPairVec;
  typedef boost::unordered_map < size_type, IntChartMapPairVec > MapsType;

  /// addMap
  void 
  addMap ( const size_type & chartid1, 
           const size_type & chartid2, 
           const ChartMap & map );

  /// operator ()
  std::shared_ptr<Geo> 
  operator () ( const AtlasGeo & geo ) const;
  std::shared_ptr<Geo> 
  operator () ( std::shared_ptr < Geo > geo_ptr ) const;

private:
  MapsType maps_;  
};


template < class ChartMap >
void 
AtlasMap<ChartMap>::addMap ( const size_type & chartid1, 
                             const size_type & chartid2, 
                             const ChartMap & map ) {
  IntChartMapPair pair ( chartid2, map );
  typename MapsType::iterator it;
  it = maps_ . find ( chartid1 );
  if ( it == maps_ . end() ) {
    IntChartMapPairVec vect;
    vect . push_back ( pair );
    typename MapsType::value_type newpair ( chartid1, vect );
    maps_ . insert ( newpair );
  } else { 
    IntChartMapPairVec & vect = it -> second;
    vect . push_back ( pair );
  }
}

template < class ChartMap > 
std::shared_ptr<Geo> 
AtlasMap<ChartMap>::operator () ( const AtlasGeo & geo ) const {
  std::shared_ptr<UnionGeo> result ( new UnionGeo );
  size_t domain_id = geo . id ();
  const RectGeo & rect = geo . rect ();
  // Apply the different maps 
  if ( maps_ . count ( domain_id ) != 0 ) { 
    const IntChartMapPairVec & mymaps = 
      maps_ . find ( domain_id ) -> second;
    BOOST_FOREACH ( const IntChartMapPair & pair, mymaps ) {
      std::shared_ptr<Geo> image_geo 
        ( new AtlasGeo ( pair . first, pair . second ( rect ) ) );
      result -> insert ( image_geo );
    }
  }
  return result;
}

template < class ChartMap > 
std::shared_ptr<Geo> 
AtlasMap<ChartMap>::operator () ( std::shared_ptr < Geo > geo_ptr ) const {
  const AtlasGeo & geo = * std::dynamic_pointer_cast < AtlasGeo > ( geo_ptr );
  return operator ( ) ( geo );
}

#endif
