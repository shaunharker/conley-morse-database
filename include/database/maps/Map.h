#ifndef CMDB_MAP_H
#define CMDB_MAP_H

class Map {
public:
  virtual boost::shared_ptr<Geo> operator () ( boost::shared_ptr<Geo> geo ) const = 0;
private:
};

#endif