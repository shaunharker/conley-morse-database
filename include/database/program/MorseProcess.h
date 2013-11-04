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
private:
  size_t num_jobs_;
  size_t num_jobs_sent_;
  Configuration config;
  Model model;
  Database database;
  int progress_bar;                         // progress bar
  clock_t time_of_last_checkpoint;
  clock_t time_of_last_progress;
#if defined EDGEMETHOD || defined SKELETONMETHOD
  chomp::CubicalComplex param_complex;
  typedef std::pair<chomp::Index,int> Cell;
  std::vector< Cell > jobs_;
#endif
#ifdef PATCHMETHOD
  boost::shared_ptr<Grid> parameter_grid;
  typedef boost::unordered_set<Grid::GridElement> GridSubset;
  std::vector < GridSubset > PS_patches; // morse_work_items
#endif
};

#endif
