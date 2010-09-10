/*
 *  Coordinator.cpp
 */

#include "program/Coordinator.h"
#include "boost/foreach.hpp"

Coordinator::Coordinator(int argc, char **argv) {
  size_t maxPatchSize = MAX_PATCH_SIZE; /// Maximum size of a patch in parameter space
  size_t param_dim = PS_DIMENSION;      /// Parameter space dimension
  PS_Toplex . initialize (PS_Bounds);   /// Parameter space toplex

  /// Factor of the size of patch side that intersect next patch
  Real intersect_factor = 0.5;

  /// Subdivide parameter space toplex
  for (int i = 0; i < PS_SUBDIVISIONS; ++i) {
    intersect_factor /= 2.0;  /// Make intersect_factor x box_side equal half the size of a cell
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

  /// Number of patches
  size_t num_patches = 1;
  for (size_t i = 0; i < param_dim; ++i) {
    patch_sides_length [i] = (PS_Bounds . upper_bounds [i] - PS_Bounds . lower_bounds [i]) / subdivisions_per_side;
	num_patches *= subdivisions_per_side;
  }

  /// Bounds for the patch bounding box
  std::vector < Real > patch_lower_bounds (param_dim);
  std::vector < Real > patch_upper_bounds (param_dim);

  /// Create all the patches
  for (size_t n = 0; n < num_patches; ++n) {
    size_t factor = 1;
    for (size_t i = 0; i < param_dim; ++i) {
     factor *= (i == 0) ? 1 : subdivisions_per_side;
      size_t index = (n / factor) % subdivisions_per_side;
	  /// Compute bounding box for the patch
	  patch_lower_bounds [i] = PS_Bounds . lower_bounds [i] + index * patch_sides_length [i];
	  patch_upper_bounds [i] = patch_lower_bounds [i] + intersect_factor * patch_sides_length [i];
    }

    /// Construct the patch toplex
    Geometric_Description patch_bounds (param_dim, patch_lower_bounds, patch_upper_bounds);
    Toplex_Subset patch_subset = PS_Toplex . cover ( patch_bounds );

    /// Add  patch to the vector of patches
    PS_patches . push_back (patch_subset);
  }

  /// Create a map with Cached_Box_Information for the intersecting boxes
  for (size_t i = 0; i < num_patches; ++i) {
    BOOST_FOREACH ( Toplex::Top_Cell cell, PS_patches [i] ) {
      for (size_t j = i + 1; j < num_patches; ++j) {
	    /// If top cell is in the intersection of PS_patches [i] and PS_patches [j]
        if (PS_patches [j] . find (cell) != PS_patches [j] . end ()) {
          /// Add an entry to the map of Cached_Box_Information
          Cached_Box_Information cached_box_info;
          PS_Toplex_Cached_Info . insert (Toplex_Cached_Box_Pair (cell, cached_box_info));
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

  /// Map with the pairing (top_cell, index in geometric_descriptions vector)
  std::map <Toplex::Top_Cell, size_t> cells_indices_map;

  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_subset . begin (); it != patch_subset . end (); ++it, ++key) {
    Geometric_Description Cell_GD = PS_Toplex . geometry (PS_Toplex . find (*it));
	/// Add geometric description to the vector of geo descriptions
    geometric_descriptions . push_back ( Cell_GD );
	/// Add the pair (top_cell, key) to the indices map
    cells_indices_map . insert ( std::pair <Toplex::Top_Cell, size_t> (* PS_Toplex . find (*it), key) );
    /// Insert cached box info into the map if there is any
	if (PS_Toplex_Cached_Info . find (*it) != PS_Toplex_Cached_Info . end ())
      patch_cached_info . insert (Patch_Cached_Box_Pair (key, PS_Toplex_Cached_Info . find (*it) -> second));
  }

  /// Adjacency information vector
  std::vector < std::vector <size_t> > adjacency_information (patch_subset . size (), std::vector <size_t> (0) );

  /// Find adjacency information for cells in the patch
  BOOST_FOREACH ( Toplex::Top_Cell cell_in_patch, patch_subset ) {
    /// Find geometric description for the cell
    Geometric_Description Cell_GD = PS_Toplex . geometry (PS_Toplex . find (cell_in_patch));

	/// Cover the geometric description in the toplex to get the neighbors
    Toplex_Subset Cell_GD_Cover = PS_Toplex . cover ( Cell_GD );

    /// For all cells in the cover
    BOOST_FOREACH ( Toplex::Top_Cell cell_in_cover, Cell_GD_Cover ) {
	  /// If cell_in_cover is in the patch its not the same as cell_in_patch, then its a neighbor
	  if ( ( patch_subset . find (cell_in_cover) != patch_subset . end () ) &&
	       ( patch_subset . find (cell_in_patch) != patch_subset . find (cell_in_cover) ) ) {
        size_t cell_key = cells_indices_map . find (cell_in_patch) -> second;
		size_t neighbor_key = cells_indices_map . find (cell_in_cover) -> second;
	    adjacency_information [cell_key] . push_back (neighbor_key);
      }
    }
  }

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;

  // prepare the message with the job to be sent
  *job << job_number;
  *job << geometric_descriptions;
  *job << patch_cached_info;
  *job << adjacency_information;

  /// Increment the jobs_sent counter
  ++num_jobs_sent_;
  
  /// A new job was prepared and sent
  return kOK;
}


void Coordinator::Process(const Message &result) {
  /// Read the results from the result message
  size_t job_number;
  result >> job_number;
  Patch_Cached_Box_Map patch_cached_info;
  result >> patch_cached_info;
  std::vector <Conley_Morse_Graph> conley_morse_graphs;
  result >> conley_morse_graphs;
  std::vector < std::vector <size_t> > equivalence_classes;
  result >> equivalence_classes;

  /// Paramter patch corresponding to received results
  Toplex_Subset patch_results = PS_patches [job_number];

  /// Update the cached box information in PS_Toplex_Cached_Info
  size_t key = 0;
  for (Toplex_Subset::const_iterator it = patch_results . begin (); it != patch_results . end (); ++it, ++key) {
    if (patch_cached_info . find (key) != patch_cached_info . end ()) {
	  PS_Toplex_Cached_Info . insert (Toplex_Cached_Box_Pair (* PS_Toplex . find (*it), patch_cached_info . find (key) -> second));
	}
  }

  /// Number of parameter cells in this patch
  size_t num_cells = patch_results . size ();

  /// Save the Conley Morse graphs
  for (size_t i = 0; i < num_cells; ++i) {
    // std::cout << conley_morse_graphs [i] << std::endl;
  }

  /// Save the equivalence class information
  for (size_t i = 0; i < num_cells; ++i) {
    for (size_t j = 0; j < num_cells; ++j) {
      // std::cout << equivalence_classes [i] [j] << std::endl;
	}
  }

  /// Increment jobs received counter
  ++num_jobs_received_;

  return;
}
