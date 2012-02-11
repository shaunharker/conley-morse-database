#ifndef CMDB_CONLEYPROCESS_H
#define CMDB_CONLEYPROCESS_H

#include "delegator/delegator.h"

#include "database/structures/Database.h"

#include "database/program/Configuration.h"

#include "chomp/Toplex.h"
#include "chomp/Rect.h"

/* * * * * * * * * * * * * * */
/* ConleyProcess declaration */
/* * * * * * * * * * * * * * */
class ConleyProcess : public Coordinator_Worker_Process {
public:
  void initialize ( void );
  int  prepare ( Message & job );
  void work ( Message & result, const Message & job ) const;
  void accept ( const Message &result );
  void finalize ( void ); 
private:
  size_t num_jobs_;
  size_t num_jobs_sent_;
  Toplex param_toplex;
  Configuration config;
  Database database;
  // A Morse-Set ID is a pair<int,int> where first is the parameter box id, and
  // second is the number of the morse set within the parameter box.
  std::vector<std::pair<int, int> > conley_work_items;
  std::map < int, Rect > param_boxes;
};

#endif
