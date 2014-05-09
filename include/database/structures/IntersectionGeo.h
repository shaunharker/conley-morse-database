#ifndef CMDB_INTERSECTIONGEO_H
#define CMDB_INTERSECTIONGEO_H

#include "database/structures/Geo.h"

class IntersectionGeo : public Geo {
public:
  std::vector< boost::shared_ptr<Geo> > elements;
  void insert ( boost::shared_ptr < Geo > geo ) {
    elements . push_back ( geo );
  }
};

#endif
