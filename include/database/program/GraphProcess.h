#ifndef CMDB_GRAPHPROCESS_H
#define CMDB_GRAPHPROCESS_H

#include "database/program/ComputeGraph.h"
#include "ModelMap.h"

#include "delegator/delegator.h"
/* * * * * * * * * * * * * * */
/* GraphProcess declaration */
/* * * * * * * * * * * * * * */
class GraphProcess : public Coordinator_Worker_Process {
public:
  void command_line ( int argc, char * argv [] );
  void initialize ( void );
  void work ( Message & result, const Message & job ) const;
  void finalize ( void );
private:
  MapEvals evals;
  Toplex toplex_;
  ModelMap * f_;
};

#endif
