#ifndef CMDB_UNIONGEO_H
#define CMDB_UNIONGEO_H

#include "database/structures/Geo.h"

class UnionGeo : public Geo {
public:
  std::vector< boost::shared_ptr<Geo> > elements;
  void insert ( boost::shared_ptr < Geo > geo ) {
    elements . push_back ( geo );
  }
};

#endif
