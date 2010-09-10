/*
 *  Coordinator.h
 */

#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include <iostream>
#include <map>

#include "boost/serialization/vector.hpp"
#include "boost/serialization/map.hpp"

#include "distributed/Distributed.h"
#include "data_structures/Cached_Box_Information.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "program/Configuration.h"

#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "algorithms/Homology.h"

using namespace Adaptive_Cubical;

/// typedefs
typedef std::map <Toplex::Top_Cell, Cached_Box_Information> Toplex_Cached_Box_Map;
typedef std::pair <Toplex::Top_Cell, Cached_Box_Information> Toplex_Cached_Box_Pair;
typedef std::map <size_t, Cached_Box_Information> Patch_Cached_Box_Map;
typedef std::pair <size_t, Cached_Box_Information> Patch_Cached_Box_Pair;
typedef ConleyMorseGraph <Toplex_Subset, Conley_Index_t> Conley_Morse_Graph;

/// Coordinator class
class Coordinator : public CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  virtual CoordinatorBase::State Prepare(Message *job);
  virtual void Process(const Message &result);
 private:
  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t num_jobs_received_;
  std::vector < Toplex_Subset > PS_patches;
  Toplex param_toplex;
  Toplex_Cached_Box_Map PS_Toplex_Cached_Info;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
