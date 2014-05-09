#ifndef REFLECTION_H
#define REFLECTION_H

#include <iostream>
#include <vector>
#include "BooleanSwitchingClasses.h"


typedef std::vector < std::vector < int > > Reflections;

// Return the Composition of reflections
// to go from input to a Face used for the key of the look up table  
Reflections reflectionComposition ( const Face & input );


// apply the inverse composition of the reflection to face
// and return the resulting face
Face convertBack ( const Face & face, 
									 const Reflections & reflections );

CFace convertBack ( const int & dim,
										const CFace & cface, 
									  const Reflections & reflections );



//-------------//
// Definitions //
//-------------//


// procedure : 
// swap every 2s into 0s, the positions of the swaps are 
// the first part of the composition. < ith >
// the second part is to put the 1's at the beginning of the 
// face code sequence. defined by "pair" < ith, jth >
inline Reflections 
reflectionComposition ( const Face & input ) {
	Reflections reflections;
	//
	int dimension = faceDimension ( input ); // Dimension of the face
	int size = input . size(); // Dimension of the space
	std::vector < int > diagreflection;
	for ( unsigned int i=0; i<size; ++i ) {
		std::vector < int > reflection;		
		// strore the location of the 1s
		if ( input[i] == 1 ) { diagreflection . push_back ( i ); }
		// if we find a 2, then store the location, will be switched to 0
		if ( input[i] == 2 )	{ 
			// reflection is needed bc of the container reflections 
			// (not the best, should be changed)
			reflection . push_back ( i ); 
			reflections . push_back ( reflection );
		}
	}
	// up to here output, contains the reflection with respect to one spatial 
	// direction 
	// needs to do the diagonal reflection 
	int counter=0;
	for ( unsigned int i=0; i<diagreflection.size(); ++i ) {
		if ( diagreflection[i] < dimension ) {
			++counter;
		} else {
			std::vector < int > tmp;
			tmp . push_back ( counter );
			tmp . push_back ( diagreflection [ i ] );
			++counter;
			reflections . push_back ( tmp );
		}
	}
	return reflections;
}


inline Face 
convertBack ( const Face & face, 
							const Reflections & reflections ) { 
	Face output;
	output = face;
	for ( int i=reflections.size()-1; i>=0; --i ) {
		std::vector < int > r = reflections[i];
		if ( r.size() == 1 ) {
			if ( output [ r[0] ] == 0 ) {
				output [ r[0] ] = 2;
			} else if ( output [ r[0] ] == 2) {
				output [ r[0] ] = 0;
			}
		} else if ( r.size() == 2 ) {
			int tmp = output [ r[0] ];
			output [ r[0] ] = output [ r [1] ];
			output [ r[1] ] = tmp;
		} else { 
			std::cout << "Something wrong with the reflections\n";
			abort();
		}
	}
	return output;
}

inline CFace 
convertBack ( const int & dim,
						  const CFace & cface, 
							const Reflections & reflections ) {
	// convert cface into a face
	Face face;
	face . assign ( dim, 1 );
	if ( cface > 0 ) {
		face [ cface-1 ] = 2;
	} else {
		face [ abs(cface)-1 ] = 0;
	}

	Face newface = convertBack ( face, reflections );
	
	CFace output;
	// convert face into a CFace
	for ( unsigned int i=0; i<dim; ++i ) {
		if ( newface [ i ] == 0 ) {
			output = -(i+1);
		} else if ( newface [ i ] == 2 ) {
			output = i+1;
		}
	}
	return output;
}

#endif
