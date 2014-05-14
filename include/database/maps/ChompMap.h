#ifndef CMDB_CHOMP_MAP
#define CMDB_CHOMP_MAP

#include "database/structures/RectGeo.h"
#include "database/maps/Map.h"
#include "boost/shared_ptr.hpp"

class ChompMap {
public:
  ChompMap ( boost::shared_ptr<const Map> cmdb_map_ ) : cmdb_map_ ( cmdb_map_ ) {}
  chomp::Rect operator () ( const chomp::Rect & rect ) const {
    boost::shared_ptr<Geo> geo ( new RectGeo ( rect ) );
    boost::shared_ptr<Geo> val = (*cmdb_map_) ( geo );
    RectGeo image = * boost::dynamic_pointer_cast<RectGeo> ( val );
    return image; 
  }
  boost::shared_ptr<Geo> operator () ( const boost::shared_ptr<Geo> & geo ) const {
    return (*cmdb_map_) ( geo ); 
  }
private:
  boost::shared_ptr<const Map> cmdb_map_;
};

#endif
