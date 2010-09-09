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

  /// Minimum number of parameter patches
  size_t min_num_patches = static_cast <size_t> (PS_Toplex . size () / maxPatchSize) + 1;
  /// Number of subdivisons per side
  size_t subdivisions_per_side = std::ceil (std::exp (std::log (min_num_patches) / param_dim));

  /// Lenghts of the sides of the patch
  std::vector < Real > patch_sides_length (param_dim);

  for (size_t i = 0; i < param_dim; ++i) {
    patch_sides_length [i] = (PS_Bounds . upper_bounds [i] - PS_Bounds . lower_bounds [i]) / subdivisions_per_side;
  }

  /// Factor of the size of patch side that intersect next patch
  Real intersect_factor = 0.05;

  /// Bounds for the patch bounding box
  std::vector < Real > patch_lower_bounds (param_dim);
  std::vector < Real > patch_upper_bounds (param_dim);

  /// Create all the patches
  for (size_t k = 0; k < subdivisions_per_side; ++k) {
    for (size_t i = 0; i < param_dim; ++i) {
      for (size_t n = 0; n < subdivisions_per_side; ++n) {
        for (size_t j = 0; j < param_dim; ++j) {   /// k + i * n
	      /// Compute bounding box for the patch
          patch_lower_bounds [j] = PS_Bounds . lower_bounds [j] + n * patch_sides_length [j];
	      patch_upper_bounds [j] = patch_lower_bounds [j] + intersect_factor * patch_sides_length [j];

	      /// Construct the patch toplex
	      Geometric_Description patch_bounds (param_dim, patch_lower_bounds, patch_upper_bounds);
          Toplex_Subset patch_subset = PS_Toplex . cover ( patch_bounds );

          /// Add  patch to the vector of patches
          PS_patches . push_back (patch_subset);
		}
	  }
    }
  }

  /// Number of patches
  size_t num_patches = PS_patches . size();

  /// Create a map with Cached_Box_Information for the intersecting boxes
  for (size_t i = 0; i < num_patches; ++i) {
    for (Toplex_Subset::const_iterator it = PS_patches [i] . begin (); it != PS_patches [i] . end (); ++it) {
	  for (size_t j = i + 1; j < num_patches; ++j) {
	    if (PS_patches [j] .find (*it)) {  /// Top cell is in the intersection of PS_patches [i] and PS_patches [j]
		  /// Add an entry to the map of Cached_Box_Information
		  Cached_Box_Information cached_box_info;
          PS_Toplex_Cached_Info . insert (Toplex_Cached_Box_Pair (*it, cached_box_info));
		}
	  }
    }
  }

  num_jobs_ = num_patches;
  num_jobs_sent_ = 0;
  num_jobs_received_ = 0;
}


CoordinatorBase::State Coordinator::Prepare(Message *job) {
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
  Toplex_Subset patch_subset = PS_patches [job_number];

  std::vector < Geometric_Description > geometric_descriptions (patch_subset . size ());
  Patch_Cached_Box_Map patch_cached_info;
  
  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_subset . begin (); it != patch_subset . end (); ++it, ++key) {
    geometric_descriptions . push_back (PS_Toplex . geometry (PS_Toplex . find (*it)));
    /// Insert cached box info into the map if there is any
	if (PS_Toplex_Cached_Info . find (*it))
      patch_cached_info . insert (Patch_Cached_Box_Pair (key, PS_Toplex_Cached_Info . find (*it) -> second ));
  }

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;

  // prepare the message with the job to be sent
  *job << job_number;
  *job << geometric_descriptions;
  *job << patch_cached_info;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}


void Coordinator::Process(const Message &result) {
  /// Read the results from the input message
  size_t job_number;
  result >> job_number;
  Patch_Cached_Box_Map patch_cached_info;
  result >> patch_cached_info;
  std::vector <Conley_Morse_Graph> conley_morse_graphs;
  result >> conley_morse_graphs;
  std::vector <std::vector <size_t> > equivalence_classes;
  result >> equivalence_classes;

  /// Increment jobs received counter
  ++num_jobs_received_;

  return;
}
