#ifndef CONSTRUCTBOOLEANMAPS_H
#define CONSTRUCTBOOLEANMAPS_H

#include <vector>
#include <utility>

#include "BooleanSwitchingClasses.h"
#include "LookUpTable.h"
#include "Reflection.h"

//==============//
// DECLARATIONS //
//==============//

/// BooleanPairFaceMaps
///   using a Look-Up Table (lut), returns a list of pair of codimension 1 faces <F1,F2> 
///   defining the maps F1->F2 for a given domain and closest face
///   thresholds_count is used to filter out the faces along the boundary of the 
///   phase space				
std::vector < std::pair < CFace, CFace > >
BooleanPairFacesMaps ( const Domain & domain,
											 const Face & closestface, 
											 const LUT & lut,
											 const std::vector<size_t> & number_of_bins );

//=============//
// DEFINITIONS //
//=============//

inline std::vector < std::pair < CFace, CFace > >
BooleanPairFacesMaps ( const Domain & domain,
											 const Face & closestface, 
											 const LUT & lut,
											 const std::vector<size_t> & number_of_bins ) {

	int dim = domain. size();

	// returns the set of reflections to go from the closest face 
	// to the corresponding face used for the key in the look-up table
	Reflections reflections=reflectionComposition(closestface);

	Reflections::const_iterator itR;

	// build the corresponding key for the look up table
	// Face with coding : 1...10...0 with k 1's for "k-dimensional face"
	Face facekey;
	facekey . assign ( dim, 0 );
	int dimface = faceDimension(closestface);
	for ( unsigned int i=0; i<dimface; ++i ) {
		facekey[i]=1;
	}
	
	// find the list of maps from the look-up table
	std::vector < std::pair < CFace, CFace > > listpair = lut.find(facekey)->second;

	// Transform back the codimension 1 face in listpair;
	std::vector < std::pair < CFace, CFace > >::iterator it;
	for ( it=listpair.begin(); it!=listpair.end(); ++it ) {
		(*it).first = convertBack ( dim, (*it).first, reflections );
		(*it).second = convertBack ( dim, (*it).second, reflections );
	}

	std::vector < std::pair < CFace, CFace > > output;

	// from the list of pairs, 
	// remove any maps F1 -> F2 with F1 or F2 on the boundary of the phase space
	for ( it=listpair.begin(); it!=listpair.end(); ++it ) {
		bool good = true;
		for ( unsigned int i=0; i<dim; ++i ) {
			if ( domain[i] == 0 ) { 
				if ( ( (*it).first == -(i+1) ) || ( (*it).second == -(i+1) ) ) {
					good = false;
				}
			} else if ( domain[i] == number_of_bins[i] ) { 
				if ( ( (*it).first == (i+1) ) || ( (*it).second == (i+1) ) ) {
					good = false;
				}
			}	
		}
		if ( good ) { 
			output . push_back ( *it );
		}
	}

	return output;
}

#endif
