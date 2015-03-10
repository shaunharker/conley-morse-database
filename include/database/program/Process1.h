#ifndef CMDB_PROCESSONE_H
#define CMDB_PROCESSONE_H

#include <ctime>
#include "boost/unordered_set.hpp"
#include "delegator/delegator.h"
#include "database/structures/Database.h"
#include "database/program/Configuration.h"
#include "database/structures/PointerGrid.h"
#include "chomp/CubicalComplex.h"

#include "Model.h"
/* * * * * * * * * * * * * * */
/* Process1 declaration */
/* * * * * * * * * * * * * * */
class Process1 : public Coordinator_Worker_Process {
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
  void work_timer ( Message & result, const Message & job ) const;
  void work_job ( Message & result, const Message & job ) const;
  void work_helper ( Message & result, const Message & job ) const;

  // Input data
  Configuration config;
  Model model;

  // Output data
  size_t output_flush_begin_;
  size_t output_flush_end_;
  size_t output_flush_size_;
  size_t output_flush_pos_; // location of 1-past-the-end in file, as measured in bits
  std::unordered_map < uint64_t, std::vector<bool> > output_data_;
  // Control data
  size_t num_jobs_;
  size_t num_jobs_sent_;
  size_t progress_bar_;
  clock_t time_of_last_progress_report_;
  clock_t time_of_last_checkpoint_;
  bool checkpoint_timer_running_;
  boost::shared_ptr<ParameterSpace> parameter_space_;
};

#endif
