#ifndef CMDB_MORSEPROCESS_H
#define CMDB_MORSEPROCESS_H

#include "boost/unordered_set.hpp"

#include "delegator/delegator.h"

#include "structures/Database.h"

#include "chomp/Toplex.h"

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
  Toplex param_toplex;
  Database database;
  typedef boost::unordered_set<GridElement> Toplex_Subset;
  std::vector < Toplex_Subset > PS_patches; // morse_work_items
};

#endif
