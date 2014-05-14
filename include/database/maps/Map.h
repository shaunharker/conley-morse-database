#ifndef CMDB_MAP_H
#define CMDB_MAP_H

#include "boost/shared_ptr.hpp"
#include "database/structures/Geo.h"

class Map {
public:
  virtual boost::shared_ptr<Geo> operator () ( boost::shared_ptr<Geo> geo ) const = 0;
private:
};

#endif