#ifndef BOOLEAN_H
#define BOOLEAN

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "database/numerics/boost_interval.h"

#include <math.h>

#include "chomp/Rect.h"
#include "database/structures/Atlas.h"
#include "database/Maps/AtlasMap.h"

#include "BooleanClasses.h"
#include "ModelMap.h"

#include <stdlib.h>

typedef uint64_t size_type;

// ------------
// Declarations
// ------------

// Extract all the faces of a given BooleanBox. Phase space boundary are included.
// The faces are added to the vector faces if not present
void extract_faces ( std::vector < Face > * faces, 
			               const BooleanBox & box ); 
void extract_faces ( std::vector < Face > * faces, 
	                   const std::vector < BooleanBox > & boxes ); 

void constructFaces ( std::vector < Face > * faces, 
	                    const BooleanBox & box );

void constructFaces ( std::vector < Face > * faces, 
	                    const std::vector < BooleanBox > & boxes );

boost::shared_ptr < ModelMap > 
constructMaps ( const chomp::Rect & phasespace,
								const std::vector < BooleanBox > & boxes, 
								const std::vector < Face > & faces );

std::vector < std::pair < Face, Face > > 
findPairFaces ( const chomp::Rect & phasespace, 
								const BooleanBox & booleanbox );

// Check if a map is needed between two faces
bool need_map ( const BooleanBox & box,
								const Face & face1, 
								const Face & face2 );


// check if a BooleanBox contained its fixed point
bool contain_fixed_point ( const BooleanBox & box );



// Output the faces into a file
void exportFaces ( const std::vector < Face > & faces );
void exportCharts ( const char * inputfile, 
	                  const std::vector < Face > & faces );

// return the fixed point for a given booleanbox
chomp::Rect fixedpoint ( const BooleanBox & box );

// check if the face is on the phase space boundary
bool notonboundary ( const chomp::Rect & phasespace, 
	                   Face & face );

//  Definitions 


inline
std::vector < std::pair < Face, Face > > 
findPairFaces ( const chomp::Rect & phasespace, 
								const BooleanBox & booleanbox ) {

	double delta ( 1e-11 ); // to define a fixed point as a small segment, TO CHANGED

	std::vector < std::pair < Face, Face > > result;
// //
	std::vector < Face > faces;

// extract the bounds of the phase space
	std::vector < double > lbp = phasespace . lower_bounds;
	std::vector < double > ubp = phasespace . upper_bounds;
//	extract the dimension and bound of the BooleanBox
	int dim = booleanbox . rect . dimension ( );
	std::vector < double > lbounds = booleanbox . rect . lower_bounds;
	std::vector < double > ubounds = booleanbox . rect . upper_bounds;
// construct the list of faces of a given BooleanBox (all faces are included) 
	std::vector < Face > * faces_ptr;
	faces_ptr = & faces;
	extract_faces ( faces_ptr, booleanbox );

// // retrieve the parameters
	chomp::Rect gamma = booleanbox . gamma;
	chomp::Rect sigma = booleanbox . sigma;

	std::vector < double > mygamma, mysigma;
	for ( unsigned int i=0; i<dim; ++i ) {
		mygamma . push_back ( 0.5 * (  gamma.lower_bounds[i] + gamma.upper_bounds[i] ) ); 
		mysigma . push_back ( 0.5 * (  sigma.lower_bounds[i] + sigma.upper_bounds[i] ) ); 
	}

// //
// 	unsigned int j,k;
// // if we have a fixed point inside the boolean box
// // all faces are mapped to it and the fixed point onto itself
// // Every pair face1-face2 are added if face1 is not on the phase space boundary
// // and face2 is always the fixed point
// // at the end, we add the pair face-face with face the fixed point.
// // Convention : fixed point is a small rect around the fixed point 
	if ( contain_fixed_point ( booleanbox ) ) {
		// std::cout << "fixed point inside the box : " << booleanbox <<"\n";
		Face fp; // will be the fixed point
		fp . direction = -1; // in case we need to track it
// 		fp . location = - mysigma [ 0 ] / mygamma [ 0 ]; // convention 
		std::vector < double > lb, ub;
// 		// Assume small rect around the fixed point
		for ( unsigned int i=0; i<dim; ++i ) {
			lb . push_back ( - mysigma [ i ] / mygamma [ i ] - delta/2.0);
			ub . push_back ( - mysigma [ i ] / mygamma [ i ] + delta/2.0);
		}
// 		// 
		fp . rect = chomp::Rect ( dim, lb, ub ); // right now we assume, fixed point ~ tiny 
		for ( unsigned int i=0; i<faces.size(); ++i ) { 
			std::pair < Face, Face > newpair ( faces[i], fp );
			result . push_back ( newpair );
		}
		std::pair < Face, Face > newpair ( fp, fp ); // add the map of the fixed point onto itself
		result . push_back ( newpair );
	// if we don't have a fixed point
	} else { 
		for ( unsigned int j=0; j<faces.size(); ++j ) {
			for ( unsigned int k=0; k<faces.size(); ++k ) {
				if ( j != k ) {
					// std::cout << "faces considered : \n";
					// std::cout << faces[j];
					// std::cout << faces[k];
					// std::cout << "valid map : " << need_map ( indexg, indexs, booleanbox, faces[j], faces[k] ) << "\n";					

					// check only for faces[j] -> faces[k] if faces[j] or faces[k] is not on the boundary 
					// if ( notonboundary(phasespace,faces[j]) || notonboundary(phasespace,faces[k]) ) {
					if ( notonboundary(phasespace,faces[j]) ) {
						if ( need_map ( booleanbox, faces[j], faces[k] ) ) { 
							std::pair < Face, Face > newpair ( faces[j], faces[k] );
							result . push_back ( newpair );
						}
					} 	
				}
			}
		}
	}
return result;
}



inline boost::shared_ptr < ModelMap > constructMaps ( const chomp::Rect & phasespace, 
											const std::vector < BooleanBox > & boxes, 
										 const std::vector < Face > & faces ) {

	boost::shared_ptr < ModelMap > atlasmap ( new ModelMap );

	std::ofstream ofile;
	ofile . open ( "MapsGraph.gv" );
	ofile << "Digraph G { \n";

	// loop through the BooleanBox : 
	for ( unsigned int i=0; i<boxes.size(); ++i ) {

		// std::cout << "\nBoolean Box : " << booleanbox[i] <<"\n";

		// retrieve the parameters
		chomp::Rect gamma = boxes[i] . gamma;
		chomp::Rect sigma = boxes[i] . sigma;
		std::vector < double > mygamma, mysigma;
		for ( unsigned int j=0; j<gamma.dimension(); ++j ) {
			mygamma . push_back ( 0.5 * (  gamma.lower_bounds[j] + gamma.upper_bounds[j] ) ); 
			mysigma . push_back ( 0.5 * (  sigma.lower_bounds[j] + sigma.upper_bounds[j] ) ); 
		}

		// For a given BooleanBox :
		// Find the list of pair of faces (face1,face2) for which a map face1->face2 is required
		// recall : a fixed point is a "face" 
		// findPairFaces need to filter out the phase space boundary.
		std::vector < std::pair < Face, Face > > myfaces = findPairFaces ( phasespace, boxes[i] );

		// for each pairface, find the corresponding charts numbers which are the same as the faces number
		for ( unsigned int j=0; j<myfaces.size(); ++j ) {
			Face face1 = myfaces[j].first;
			Face face2 = myfaces[j].second;
			// need to find the id of face1, face2 which are the same as the charts id
			int id1, id2;
			id1 = id2 = -1;
			for ( unsigned int k=0; k<faces.size(); ++k ) {
				if ( faces[k] == face1 ) { id1 = k; }
				if ( faces[k] == face2 ) { id2 = k; }
			}

			if ( id1 == -1 ) { std::cout << "error for id1\n"; std::cout << face1; abort(); }
			if ( id2 == -1 ) { std::cout << "error for id2\n"; std::cout << face2; abort(); }

			if ( face2 . direction != -1 ) {
				if ( notonboundary(phasespace,face2) ) {
					ofile << id1 << " -> " << id2 << "\n";
				} else { 
					ofile << id1 << " -> " << "-1" << "\n";
				}
			} else { 
				ofile << id1 << " -> " << id2 << "\n";
			}
			// if we map to a fixed point (it does not matter what face1 is)
			if ( face2 . direction == -1 ) {
				BooleanChartMap map ( 2, id2, mygamma, mysigma, face1, face2 );
				atlasmap -> addMap ( id1, id2, map );
			} else if ( notonboundary(phasespace,face2) ) {
			// if we have a "normal map"
				BooleanChartMap map ( 0, id2, mygamma, mysigma, face1, face2 );
				atlasmap -> addMap ( id1, id2, map );
			} else { 
				BooleanChartMap map ( 3, id2, mygamma, mysigma, face1, face2 );
				atlasmap -> addMap ( id1, id2, map );
			}
		}
	}

	ofile << "}\n";
	ofile . close ( );

	return atlasmap;
}



inline chomp::Rect fixedpoint ( const BooleanBox & box ) { 


	std::vector < double > lb = box . rect . lower_bounds;
	std::vector < double > ub = box . rect . upper_bounds;
	chomp::Rect gamma = box . gamma;
	chomp::Rect sigma = box . sigma;
	
	for ( unsigned int i=0; i<box.rect.dimension(); ++i ) {

		double mygamma, mysigma;
		mygamma = 0.5 * (  gamma.lower_bounds[i] + gamma.upper_bounds[i] );
		mysigma = 0.5 * (  sigma.lower_bounds[i] + sigma.upper_bounds[i] ); 
		
		lb [ i ] = - mysigma / mygamma; 
		ub [ i ] = - mysigma / mygamma;
	}

	return chomp::Rect ( box.rect.dimension(), lb, ub );

}




inline void constructFaces ( std::vector < Face > * faces, const std::vector < BooleanBox > & boxes ) {

	for ( unsigned int i=0; i<boxes.size(); ++i ) {
		constructFaces ( faces, boxes[i] );
	}

}


// Given a set of parameters, extract the faces from a booleanbox and add them to faces if not present
// The faces along the phase space boundary are included

inline void constructFaces ( std::vector < Face > * faces, const BooleanBox & box ) {

	//
	chomp::Rect rect = box . rect;
	int dim = rect . dimension ();
	std::vector < double > lbounds, ubounds;	
	lbounds = rect . lower_bounds;
	ubounds = rect . upper_bounds;
	unsigned int j;
	//
	double eps = 1e-11;
	// We consider the fixed points
	// we could make the function fixedpoint return the proper rect
	//
	Face fp;
	if ( contain_fixed_point ( box ) ) {
		// add a face at the fixed point inside the boolean box
		fp . direction = -1;	
		fp . rect = fixedpoint ( box );
		for ( unsigned int i=0; i<dim; ++i ) {
			fp . rect . lower_bounds [ i ] = - 0.5 * eps + fp . rect . lower_bounds [ i ];
			fp . rect . upper_bounds [ i ] = 0.5 * eps + fp . rect . upper_bounds [ i ];
		}
		// no need to check if it is in faces ( it cannot be )
	} else { 
		// add a face at the center of the boolean box
		fp . direction = -1;
		fp . rect = box . rect;
		for ( unsigned int i=0; i<dim; ++i ) {
			fp . rect . lower_bounds [ i ] = - 0.5 * eps + 0.5 * ( box . rect . lower_bounds [ i ] + box . rect . upper_bounds [ i ] );
			fp . rect . upper_bounds [ i ] = 0.5 * eps + 0.5 * ( box . rect . lower_bounds [ i ] + box . rect . upper_bounds [ i ] );
		}
	}
	faces -> push_back ( fp );

	// For each dimension, there are two faces
	for ( j=0; j<dim; ++j ) {
		Face face1, face2;
		face1 . direction = face2 . direction = j;
		face1 . rect = face2 . rect = rect;
		face1 . rect . lower_bounds [ j ] = rect.lower_bounds [ j ];
		face1 . rect . upper_bounds [ j ] = rect.lower_bounds [ j ];
		face2 . rect . lower_bounds [ j ] = rect.upper_bounds [ j ];
		face2 . rect . upper_bounds [ j ] = rect.upper_bounds [ j ];
		//
		int alreadythere1=0;
		int alreadythere2=0;
		// check if the new faces are already in the vector 
		// not the best, would be nice to have something like find in unordered_map
		std::vector < Face >::iterator it;
		std::vector < Face >::iterator it1, it2; // to store existing faces iterator
		for ( it=faces->begin(); it!=faces->end(); ++it ) { 
			if ( face1 == *it ) { alreadythere1 = 1; it1 = it; }
			if ( face2 == *it ) { alreadythere2 = 1; it2 = it; }
		}
		// add the face if needed
		if ( alreadythere1 == 0 ) { faces -> push_back ( face1 ); }
		if ( alreadythere2 == 0 ) { faces -> push_back ( face2 ); }
	}	
}


///////////////


//
// Check if there a need for a map : Face1 -> Face2 
inline 
bool need_map ( const BooleanBox & box,
								const Face & face1, const Face & face2 ) {

	// double eps = 1e-12;

	int dim = box.rect.dimension();

// retrieve the parameter values
	chomp::Rect gamma = box . gamma;
	chomp::Rect sigma = box . sigma;

	std::vector < double > mygamma, mysigma;
	for ( unsigned int i=0; i<dim; ++i ) {
		mygamma . push_back ( 0.5 * (  gamma.lower_bounds[i] + gamma.upper_bounds[i] ) ); 
		mysigma . push_back ( 0.5 * (  sigma.lower_bounds[i] + sigma.upper_bounds[i] ) ); 
	}

	int dir1 = face1 . direction;
	int dir2 = face2 . direction;

	// std::cout << "We are in box : " << box << "\n";

	double x1, x2; // starting points 
	double y1, y2; // images

	// // if faces are parallel, we just have to check from the images of the edges of rectangle of both faces
	// // they are both along the same dimension, and with the same ordering
	// // Both faces are of the from : x_i = cst and the rects are of the form : 
	// // I_0 x I_1 x I_2 x ... x I_(i-1) x I_(i+1) X ... I_n
	// //
	// // for each I above, check that f ( I ) |^| I non-empty and !={c} ( as to be true for all of them )
	// //

	// define a "normal" map : face1 --> face2
	BooleanChartMap map ( 0, -1, mygamma, mysigma, face1, face2 );
	// construct the AtlasGeo from face1 
	std::vector < double > lb1, ub1, lb2, ub2; 
	for ( unsigned int i=0; i<dim; ++i ) {
		if ( i != face1.direction ) {
			lb1 .push_back ( face1.rect.lower_bounds[i] );
			ub1 .push_back ( face1.rect.upper_bounds[i] );
		}
	}
	AtlasGeo atlasgeo ( 0, RectGeo(dim-1,lb1,ub1) );

// std::cout << face1;
// std::cout << face2;
// atlasgeo . info();

	AtlasGeo image;

	if ( dir1 == dir2 ) {
		// here dir1 == dir2 and lower_bounds[dir1]=upper_bounds[dir1]
		x1 = face1 . rect . lower_bounds [ dir1 ];
		y1 = face2 . rect . lower_bounds [ dir1 ];
		// check if there is a time T such that there is an intersection phi(t,face1) with face2
		// then compute the image phi(T, face1)
		if ( findTbool ( mygamma[dir2], mysigma[dir2], x1, y1 ) ) {
			image = map ( atlasgeo );
		} else {
			return false;
		}
	} else { 
		x1 = face1 . rect . lower_bounds [ dir2 ];
		x2 = face1 . rect . upper_bounds [ dir2 ];
		y1 = face2.rect.lower_bounds[dir2];
    y2 = face2.rect.lower_bounds[dir2];
		if ( findTbool ( mygamma[dir2], mysigma[dir2], x1, y1 ) && findTbool ( mygamma[dir2], mysigma[dir2], x2, y2 ) ) {
			image = map ( atlasgeo );	
		} else { 
			return false;
		}
	}

	//
	// construct the Rect of codimension 1 from face2
	for ( unsigned int i=0; i<dim; ++i ) {
		if ( i != dir2 ) {
			lb2 . push_back ( face2.rect.lower_bounds[i] );
			ub2 . push_back ( face2.rect.upper_bounds[i] );
		}
	}

	chomp::Rect rect1 = image . rect();
	chomp::Rect rect2 ( dim - 1, lb2, ub2 );

	// check if we have a "proper intersection"
	if ( intersect ( rect1, rect2 ) ) {
		return true;
	} else {
		return false;
	}

}




// Check if a BooleanBox contained its fixed point
inline 
bool contain_fixed_point ( const BooleanBox & box ) {

	std::vector < double > lb = box . rect . lower_bounds;
	std::vector < double > ub = box . rect . upper_bounds;
	chomp::Rect gamma = box . gamma;
	chomp::Rect sigma = box . sigma;
	for ( unsigned int i=0; i<box.rect.dimension(); ++i ) {

		// Currently take the midpoint for the parameters gamma, sigma
		double mygamma, mysigma;
		mygamma = 0.5 * (  gamma.lower_bounds[i] + gamma.upper_bounds[i] ); 
		mysigma = 0.5 * (  sigma.lower_bounds[i] + sigma.upper_bounds[i] );
		
		double tmp = - mysigma / mygamma;
		if ( ( tmp < lb[i] ) || ( tmp > ub[i] ) ) { return false;}
	}
	return true;
}




// // Extract all the faces of the given BooleanBox
// // if the faces are not part of the vector faces then they are added
inline 
void extract_faces ( std::vector < Face > * faces, const BooleanBox & box ) {
	chomp::Rect rect = box . rect;
	int dim = rect . dimension ();
	std::vector < double > lbounds, ubounds;	
	lbounds = rect . lower_bounds;
	ubounds = rect . upper_bounds;
	unsigned int j;
	// For each dimension, there are two faces
	for ( j=0; j<dim; ++j ) {
		Face face1, face2;
		face1 . direction = face2 . direction = j;
		face1 . rect = face2 . rect = rect;
		face1 . rect . lower_bounds [ j ] = rect.lower_bounds [ j ];
		face1 . rect . upper_bounds [ j ] = rect.lower_bounds [ j ];
		face2 . rect . lower_bounds [ j ] = rect.upper_bounds [ j ];
		face2 . rect . upper_bounds [ j ] = rect.upper_bounds [ j ];
		
		//
		int alreadythere1=0;
		int alreadythere2=0;
		// check if the new faces are already in the vector 
		// not the best, would be nice to have something like find in unordered_map
		std::vector < Face >::iterator it;
		for ( it=faces->begin(); it!=faces->end(); ++it ) { 
			if ( face1 == *it ) { alreadythere1=1; }
			if ( face2 == *it ) { alreadythere2=1; }
		}
		// add the face if needed
		if ( alreadythere1 == 0 ) { faces -> push_back ( face1 ); }
		if ( alreadythere2 == 0 ) { faces -> push_back ( face2 ); }
	}	
}

inline
void extract_faces ( std::vector < Face > * faces, const std::vector < BooleanBox > & boxes ) { 
	for ( unsigned int i=0; i<boxes.size(); ++i ) {
		extract_faces ( faces, boxes[i] );
	}
}


// Check if face is on the phase space boundary
inline
bool notonboundary ( const chomp::Rect & phasespace, Face & face ) { 
	//
	double eps = 1e-10;
	//
	if ( face . direction == -1 ) { 
		std::cout << "Trying to check if a fixed point is on the phase space boundary. Not implemented yet\n";
		abort();
	}
	//
	double location = face . rect . lower_bounds [ face . direction ];
	double lps = phasespace . lower_bounds [ face . direction ];
	double ups = phasespace . upper_bounds [ face . direction ];
	if ( ( std::abs ( location - lps ) > eps ) && ( std::abs ( location - ups ) > eps ) ) {
		return true;
	}
	return false;
}





// Save the faces into a text file that can be open to plot them 
// using for instance plot_faces.py
// faces are given in the following format :
//
// id lb0 ub0 lb1 ub1 ...
//
inline
void exportFaces ( const std::vector < Face > & faces ) {
	std::ofstream ofile;
	ofile . open ( "faces.txt" );
	
	for ( unsigned int i=0; i<faces.size(); ++i ) {
		for ( unsigned int j=0; j<faces[i].rect.dimension(); ++j ) {
			ofile << faces[i].rect.lower_bounds[j] << " " << faces[i].rect.upper_bounds[j] << " ";
		}
		ofile <<"\n";
	}
	ofile . close();
}


inline
void exportCharts ( const char * inputfile, const std::vector < Face > & faces ) {
	std::ofstream ofile;
	ofile . open ( inputfile );
	int dir;
	for ( unsigned int i=0; i<faces.size(); ++i ) {
		if ( faces[i].direction == -1 ) {
			dir = 0 ;
		} else { 
			dir = faces[i].direction;
		}
		ofile << i << " " << dir << " ";
		for ( unsigned int j=0; j<faces[i].rect.dimension(); ++j ) {
			ofile << faces[i].rect.lower_bounds[j] << " " << faces[i].rect.upper_bounds[j] << " ";
		}
		ofile <<"\n";
	}
	ofile . close();
}



#endif