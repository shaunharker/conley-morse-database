#ifndef CMDB_MORSEPROCESS_H
#define CMDB_MORSEPROCESS_H

#define EDGEMETHOD
#include <ctime>
#include "boost/unordered_set.hpp"
#include "delegator/delegator.h"
#include "database/structures/Database.h"
#include "database/program/Configuration.h"
#include "chomp/Toplex.h"
#include "chomp/CubicalComplex.h"

/* * * * * * * * * * * * * * */
/* MorseProcess declaration */
/* * * * * * * * * * * * * * */
class MorseProcess : public Coordinator_Worker_Process {
public:
  void initialize ( void );
  int  prepare ( Message & job );
  void work ( Message & result, const Message & job ) const;
  void accept ( const Message &result );
  void finalize ( void ); 
private:
  size_t num_jobs_;
  size_t num_jobs_sent_;
  Configuration config;
  Database database;
  int progress_bar;                         // progress bar
  clock_t time_of_last_checkpoint;
  clock_t time_of_last_progress;
#if defined EDGEMETHOD || defined SKELETONMETHOD
  CubicalComplex param_complex;
  typedef std::pair<Index,int> Cell;
  std::vector< Cell > jobs_;
#endif
#ifdef PATCHMETHOD
  Toplex param_toplex;
  typedef boost::unordered_set<GridElement> Toplex_Subset;
  std::vector < Toplex_Subset > PS_patches; // morse_work_items
#endif
};

#endif
