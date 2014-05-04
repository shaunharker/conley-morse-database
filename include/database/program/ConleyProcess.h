#ifndef CMDB_CONLEYPROCESS_H
#define CMDB_CONLEYPROCESS_H

#include "delegator/delegator.h"
#include "database/structures/Database.h"
#include "database/program/Configuration.h"

#include <vector>
#include "database/structures/Grid.h"

#include "Model.h"

/* * * * * * * * * * * * * * */
/* ConleyProcess declaration */
/* * * * * * * * * * * * * * */
class ConleyProcess : public Coordinator_Worker_Process {
public:
  void command_line ( int argc, char * argv [] );
  void initialize ( void );
  int  prepare ( Message & job );
  void work ( Message & result, const Message & job ) const;
  void accept ( const Message &result );
  void finalize ( void ); 

  void checkpoint ( void );
  void progressReport ( void );
private:

  Configuration config;
  Database database;
  Model model;

  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t num_incc_;
  std::vector<uint64_t> attempts_;
  std::vector<bool> finished_;
  size_t num_finished_;
  clock_t time_of_last_progress_report;
  bool checkpoint_timer_running_;

};

#endif
