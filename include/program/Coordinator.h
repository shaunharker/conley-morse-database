/*
 *  Coordinator.h
 */

#ifndef _CMDP_COORDINATOR_
#define _CMDP_COORDINATOR_

#include <iostream>
#include <map>

#include "boost/serialization/vector.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/set.hpp"

#include "distributed/Distributed.h"
#include "data_structures/Conley_Morse_Graph.h"
#include "data_structures/UnionFind.hpp"
#include "program/Configuration.h"

#include "toplexes/Adaptive_Cubical_Toplex.h"
#include "algorithms/Homology.h"

using namespace Adaptive_Cubical;

/// typedefs
typedef ConleyMorseGraph < std::vector < Adaptive_Cubical::Toplex::Top_Cell >, Conley_Index_t> Conley_Morse_Graph;

/// Coordinator class
class Coordinator : public CoordinatorBase {
 public:
  Coordinator(int argc, char **argv);
  virtual CoordinatorBase::State Prepare(Message *job);
  virtual void Process(const Message &result);
  void finalize ( void );
 private:
  Toplex param_toplex;
  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t num_jobs_received_;
  std::vector < Toplex_Subset > PS_patches;
  // database variables
  std::map <  Toplex::Top_Cell, Conley_Morse_Graph > graphs;
  typedef std::set < std::pair < Conley_Morse_Graph::Vertex, Conley_Morse_Graph::Vertex > > Clutch;
  std::map < std::pair < Toplex::Top_Cell, Toplex::Top_Cell >, Clutch > clutchings;
  UnionFind < Toplex::Top_Cell > continuation_classes;
  void save_continuation_graph ( void );
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Coordinator.hpp"
#endif

#endif
