//CompressedGrid.h
#ifndef CMDB_COMPRESSED_GRID_H
#define CMDB_COMPRESSED_GRID_H
#include <vector>
#include "database/structures/RectGeo.h"
#include "database/structures/CompressedTree.h"
#include "boost/shared_ptr.hpp"

class CompressedGrid {
public:
  typedef uint64_t size_type;
  size_type size_;
  RectGeo bounds_;
  int dimension_;
  std::vector < bool > periodic_;
  boost::shared_ptr<CompressedTree> tree_;
};
#endif
