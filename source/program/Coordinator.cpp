/*
 *  Coordinator.cpp
 */

#include "program/Coordinator.h"

Coordinator::Coordinator(int argc, char **argv) {
  size_t maxPatchSize = MAX_PATCH_SIZE; /// Maximum size of a patch in parameter space
  size_t param_dim = PS_DIMENSION;      /// Parameter space dimension
  PS_Toplex . initialize (PS_Bounds);   /// Parameter space toplex

  /// Subdivide parameter space toplex
  for (int i = 0; i < PS_SUBDIVISIONS; ++i) {
    for (Toplex::const_iterator it = PS_Toplex . begin (); it != PS_Toplex . end (); ++it)
      PS_Toplex . subdivide (it);
  }
  
  if (maxPatchSize < 2)
    maxPatchSize = 2;
  
  // Number of parameter cells
  size_type num_param_cells = parameter_toplex . size ();
  // Number of parameter patches
  size_type num_patches = static_cast <size_type> (num_param_cells / maxPatchSize) + 1;
  
  Geometric_Description parameter_bounds = parameter_toplex . bounds ();
  
  std::vector < Real > side_lengths;
  for (int i = 0; i < param_dim; ++i)
    side_lengths . push_back ((parameter_bounds . upper_bounds [i] -
                               parameter_bounds . lower_bounds [i]) / 2); // <- modify this
  
  for (unsigned int i = 0; i < num_patches; ++i) {
    std::vector < Real > patch_lower_bounds;
    std::vector < Real > patch_upper_bounds;
    
    for (int j = 0; j < param_dim; ++j) {
      patch_lower_bounds . push_back (parameter_bounds . lower_bounds [j]);
      patch_upper_bounds . push_back (parameter_bounds . upper_bounds [j]);
    }
    
    Geometric_Description patch_bounds (param_dim, patch_lower_bounds, patch_upper_bounds);
    Toplex_Subset patch_subset = parameter_toplex . cover ( patch_bounds );
    
    patches_ . push_back (patch_subset);
  }
  
  // Find intersecting boxes  <- to do
  
  Cached_Box_Map toplex_cached_info;
  
  for (Toplex::const_iterator it = parameter_toplex . begin (); it != parameter_toplex . end (); ++it) {
    Cached_Box_Information cached_box_information;
    toplex_cached_info . insert (Cached_Box_Pair (*it, cached_box_information));
  }
  
  num_jobs_ = num_patches;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
}


CoordinatorBase::State Coordinator::Prepare(Message *job) {
  /// typedefs
  typedef std::map <size_t, Cached_Box_Information> Cached_Box_Map;
  typedef std::pair <size_t, Cached_Box_Information> Cached_Box_Pair;

  /// All jobs have finished
  if (num_jobs_received_ == num_jobs_)
    return kFinish;

  /// All jabs have been sent
  /// Still waiting for some jobs to finish
  if (num_jobs_sent_ == num_jobs_)
    return kPending;

  /// There are jobs to be processed
  // Prepare a new job and send it to process

  /// Job number (job id) of job to be sent
  size_t job_number = num_jobs_sent_;

  /// Toplex with the patch to be sent
  Toplex_Subset patch_subset = patches_ [job_number];

  std::vector < Geometric_Description > geometric_descriptions (patch_subset . size ());
  Cached_Box_Map patch_cached_info;
  
  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_subset . begin (); it != patch_subset . end (); ++it, ++key) {
    geometric_descriptions . push_back (parameter_toplex . geometry (parameter_toplex . find (*it)));
    patch_cached_info . insert (Cached_Box_Pair (key, toplex_cached_info .find (*it) -> second ));
  }

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;

  // prepare the message with the result of the computations
  *job << job_number;
//  *job << geometric_descriptions [ 0 ];
//  *job << patch_cached_info;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}

void Coordinator::Process(const Message &result) {
  /// typedefs
  typedef std::map <size_t, Cached_Box_Information> Cached_Box_Map;
  typedef std::pair <size_t, Cached_Box_Information> Cached_Box_Pair;
//  typedef ConleyMorseGraph <Toplex_Subset, Conley_Index> Conley_Morse_Graph;

  /// Read the results from the input message
  size_t job_number;
  result >> job_number;
//  Cached_Box_Map patch_cached_info;
//  result >> patch_cached_info;
//  std::vector <Conley_Morse_Graph> conley_morse_graphs;
//  result >> conley_morse_graphs;
  std::vector <std::vector <size_t> > equivalence_classes;
  result >> equivalence_classes;

  return;
}
