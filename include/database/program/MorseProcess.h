#ifndef CMDB_MORSEPROCESS_H
#define CMDB_MORSEPROCESS_H

#include <ctime>
#include "boost/unordered_set.hpp"
#include "delegator/delegator.h"
#include "database/structures/Database.h"
#include "database/program/Configuration.h"
#include "database/structures/PointerGrid.h"
#include "chomp/CubicalComplex.h"

#include "Model.h"
/* * * * * * * * * * * * * * */
/* MorseProcess declaration */
/* * * * * * * * * * * * * * */
class MorseProcess : public Coordinator_Worker_Process {
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
  size_t num_jobs_;
  size_t num_jobs_sent_;
  Configuration config;
  Model model;
  Database database;
  size_t progress_bar_;                         // progress bar
  clock_t time_of_last_progress_report_;
  clock_t time_of_last_checkpoint_;
  bool checkpoint_timer_running_;
  std::shared_ptr<ParameterSpace> parameter_space_;
};

#endif
