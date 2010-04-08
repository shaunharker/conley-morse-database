/*
 *  Directed_Graph.h
 */

#ifndef _CMDP_DIRECTED_GRAPH_
#define _CMDP_DIRECTED_GRAPH_

template < Toplex_Type >
class Directed_Graph : public std::map < typename Toplex_Type::Top_Cell, typename Toplex_Type::Subset_Container > {};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "distributed/Directed_Graph.hpp"
#endif

#endif
