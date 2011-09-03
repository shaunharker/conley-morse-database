#ifndef CMDB_CONLEYPROCESS_H
#define CMDB_CONLEYPROCESS_H

#include "delegator/delegator.h"

#include "structures/Database.h"

#include "toplexes/Adaptive_Cubical_Toplex.h"
using namespace Adaptive_Cubical;

/* * * * * * * * * * * * * * */
/* ConleyProcess declaration */
/* * * * * * * * * * * * * * */
class ConleyProcess : public Coordinator_Worker_Process {
public:
  void initialize ( void );
  int  write ( Message & job );
  void work ( Message & result, const Message & job ) const;
  void read ( const Message &result );
  void finalize ( void ); 
private:
  size_t num_jobs_;
  size_t num_jobs_sent_;
  Toplex param_toplex;
  Database database;
  std::vector<std::pair<int, int> > conley_work_items;
};

#endif
