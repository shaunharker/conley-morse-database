#ifndef CMDB_CONLEYPROCESS_H
#define CMDB_CONLEYPROCESS_H

#include "delegator/delegator.h"
#include "database/structures/Database.h"
#include "database/program/Configuration.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/chrono/chrono_io.hpp>

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
  std::shared_ptr<ParameterSpace> parameter_space_;
  size_t num_jobs_sent_;
  size_t num_incc_;
  int64_t current_incc_;
  std::vector<uint64_t> attempts_;
  std::vector<bool> finished_;
  size_t num_finished_;
  boost::posix_time::ptime time_of_last_checkpoint_;
  boost::posix_time::ptime time_of_last_progress_report_;
  bool checkpoint_timer_running_;

};

#endif
