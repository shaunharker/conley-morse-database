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
#include "database/structures/AtlasMap.h"

#include "BooleanClasses.h"
#include "ModelMap.h"

#include <stdlib.h>

typedef uint64_t size_type;

// ------------
// Declarations
// ------------

// Extract all the faces of a given BooleanBox. Phase space boundary are included.
// The faces are added to the vector faces if not present
void extract_faces ( std::vector < Face > * faces, const BooleanBox & box ); 
void extract_faces ( std::vector < Face > * faces, const std::vector < BooleanBox > & boxes ); 

// Construct the maps for the entire set of booleanboxes
void constructMaps ( boost::shared_ptr<Atlas> & atlas, AtlasMap & atlasmap, 
										 const chomp::Rect & phasespace, 
										 const std::vector < BooleanBox > & booleanbox, 
										 std::vector < double > indexg,
										 std::vector < double > indexs );

// Return the list of pair of faces where a map is required for a given booleanbox
std::vector < std::pair < Face, Face > > findPairFaces ( const chomp::Rect & phasespace, 
																												 const BooleanBox & booleanbox,
																												 std::vector < double > indexg,
																												 std::vector < double > indexs );

// Check if a map is needed between two faces
bool need_map ( const std::vector < double > & indexg, 
								const std::vector < double > & indexs, 
								const BooleanBox & box,
								const Face & face1, const Face & face2 );

// check if a BooleanBox contained its fixed point
bool contain_fixed_point ( const BooleanBox & box,
													 std::vector < double > indexg,
										 			 std::vector < double > indexs );


// Output the faces into a file
void exportFaces ( const std::vector < Face > & faces );


//
// Check if there a need for a map : Face1 -> Face2 
bool need_map ( const std::vector < double > & indexg, 
								const std::vector < double > & indexs, 
								const BooleanBox & box,
								const Face & face1, const Face & face2 ) {

	double eps = 1e-12;

	int dim = box.rect.dimension();

// retrieve the parameter values
	chomp::Rect gamma = box . gamma;
	chomp::Rect sigma = box . sigma;

	std::vector < double > mygamma, mysigma;
	for ( unsigned int i=0; i<dim; ++i ) {
		mygamma . push_back ( ( 1.0 - indexg[i] ) * (  gamma.lower_bounds[i] - gamma.upper_bounds[i] ) + gamma.upper_bounds[i] ); 
		mysigma . push_back ( ( 1.0 - indexs[i] ) * (  sigma.lower_bounds[i] - sigma.upper_bounds[i] ) + sigma.upper_bounds[i] ); 
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
	Map map ( 0, -1, mygamma, mysigma, face1, face2 );
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
bool contain_fixed_point ( const BooleanBox & box,
													 std::vector < double > indexg,
										 			 std::vector < double > indexs ) {

	std::vector < double > lb = box . rect . lower_bounds;
	std::vector < double > ub = box . rect . upper_bounds;
	chomp::Rect gamma = box . gamma;
	chomp::Rect sigma = box . sigma;
	for ( unsigned int i=0; i<box.rect.dimension(); ++i ) {

		double mygamma, mysigma;
		mygamma = ( 1.0 - indexg[i] ) * (  gamma.lower_bounds[i] - gamma.upper_bounds[i] ) + gamma.upper_bounds[i]; 
		mysigma = ( 1.0 - indexs[i] ) * (  sigma.lower_bounds[i] - sigma.upper_bounds[i] ) + sigma.upper_bounds[i]; 
		
		double tmp = - mysigma / mygamma;
		if ( ( tmp < lb[i] ) || ( tmp > ub[i] ) ) { return false;}
	}
	return true;
}

// For a given BooleanBox, we return the list of faces pair 
// for which we have a map
//
// face1 -> face2 = ( face1, face2 )
//
std::vector < std::pair < Face, Face > > findPairFaces ( const chomp::Rect & phasespace, 
																												 const BooleanBox & booleanbox,
																												 std::vector < double > indexg,
																												 std::vector < double > indexs ) {

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
	// std::cout << "Faces extracted : \n";
	// for ( unsigned int i=0; i<faces.size(); ++i ) {
	// 	std::cout << faces[i];
	// }
	// std::cout << "\n";

// // retrieve the parameters
	chomp::Rect gamma = booleanbox . gamma;
	chomp::Rect sigma = booleanbox . sigma;

	std::vector < double > mygamma, mysigma;
	for ( unsigned int i=0; i<dim; ++i ) {
		mygamma . push_back ( ( 1.0 - indexg[i] ) * (  gamma.lower_bounds[i] - gamma.upper_bounds[i] ) + gamma.upper_bounds[i] ); 
		mysigma . push_back ( ( 1.0 - indexs[i] ) * (  sigma.lower_bounds[i] - sigma.upper_bounds[i] ) + sigma.upper_bounds[i] ); 
	}

// //
// 	unsigned int j,k;
// // if we have a fixed point inside the boolean box
// // all faces are mapped to it and the fixed point onto itself
// // Every pair face1-face2 are added if face1 is not on the phase space boundary
// // and face2 is always the fixed point
// // at the end, we add the pair face-face with face the fixed point.
// // Convention : fixed point is a small rect around the fixed point 
	if ( contain_fixed_point ( booleanbox, indexg, indexs ) ) {
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
					if ( need_map ( indexg, indexs, booleanbox, faces[j], faces[k] ) ) { 
						std::pair < Face, Face > newpair ( faces[j], faces[k] );
						result . push_back ( newpair );
					} 	
				}
			}
		}
	}
return result;
}




// construct the maps, add charts as we go along
void constructMaps ( boost::shared_ptr<Atlas> & atlas, AtlasMap & atlasmap, 
										 const chomp::Rect & phasespace, 
										 const std::vector < BooleanBox > & booleanbox,
										 std::vector < double > indexg,
										 std::vector < double > indexs ) {

	std::ofstream ofile;
	std::ofstream ofile2;
	double eps = 1e-11;

	std::vector < double > lb1, ub1, lb2, ub2;

// extract the bounds of the phase space
	std::vector < double > lbp = phasespace . lower_bounds;
	std::vector < double > ubp = phasespace . upper_bounds;

	int dim = phasespace . dimension();

// Format : chart_id dir location lb ub 
	ofile2 . open ( "charts_details.txt" );

	ofile . open ( "face_graph.gv" );
	ofile << "Digraph G {\n";

	// will store the faces with a "valid" map associated with it
	std::vector < Face > faces;
	int counter = 0;
	//
	for ( unsigned int i=0; i<booleanbox.size(); ++i ) {

		// std::cout << "\nBoolean Box : " << booleanbox[i] <<"\n";

		// retrieve the parameters
		chomp::Rect gamma = booleanbox[i] . gamma;
		chomp::Rect sigma = booleanbox[i] . sigma;
		std::vector < double > mygamma, mysigma;
		for ( unsigned int i=0; i<gamma.dimension(); ++i ) {
			mygamma . push_back ( ( 1.0 - indexg[i] ) * (  gamma.lower_bounds[i] - gamma.upper_bounds[i] ) + gamma.upper_bounds[i] ); 
			mysigma . push_back ( ( 1.0 - indexs[i] ) * (  sigma.lower_bounds[i] - sigma.upper_bounds[i] ) + sigma.upper_bounds[i] ); 
		}

		// For a given BooleanBox :
		// Find the list of pair of faces (face1,face2) for which a map face1->face2 is required
		// recall : a fixed point is a "face" 
		std::vector < std::pair < Face, Face > > myfaces = findPairFaces ( phasespace, booleanbox[i], indexg, indexs );
		
		// std::cout << "PairFaces found : \n";
		// for ( unsigned int m=0; m<myfaces.size(); ++m ) {
		// 	std::cout << myfaces[m].first;
		// 	std::cout << " ---> \n";
		// 	std::cout << myfaces[m].second;
		// 	std::cout << "\n";
		// }

		// for each pair construct the map and add it to atlasmap and also keep track of the faces
		for ( unsigned int m=0; m<myfaces.size(); ++m ) {
			// retrieve face1
			Face face1 = myfaces[m].first;

			int dir1 = face1.direction;

			double location1;
			// this way should be easier 
			if ( dir1 == -1 ) {
				dir1=0;
			}
			location1 = face1.rect.lower_bounds[dir1];
			
			Face face2 = myfaces[m].second;
			int dir2 = face2.direction;
			double location2;
			if ( dir2 == -1 ) {
				dir2 = 0;
			}
			location2 = face2.rect.lower_bounds[dir2];
			
			// std::cout << "---\n";
			// std::cout << face1;
			// std::cout << "--->\n";
			// std::cout << face2;
			// std::cout << "---\n";

			double lps, ups;
			
			lps = lbp[dir1];
			ups = ubp[dir1];
			
			// Consider only pairs for which face1 is not on the phase space boundary
			if ( ( std::abs ( location1 - lps ) > eps ) && ( std::abs ( location1 - ups ) > eps ) ) {
				// retrieve face2
				// Face face2 = myfaces[m].second;			
				// int dir2 = face2.direction;
				// double location2 = face2.rect.lower_bounds[dir2];
				// //
				// if ( dir2 == -1 ) {
				// 	dir2 = 0;
				// }
			// 	lps = lbp[0];
			// 	ups = ubp[0];
			// } else { 
				lps = lbp[dir2];
				ups = ubp[dir2];
			// }
				// if face2 is not on the phase space boundary 
				if ( ( std::abs(location2-lps)>eps ) && ( std::abs(location2-ups)>eps ) ) {
				// check if face1 and face2 are already in the list faces
					int id1, id2;
					int found1 = 0;
					int found2 = 0;
					// should be improved
					for ( unsigned int j=0; j<faces.size(); ++j ) {
						if ( faces[j] == face1 ) { id1 = j; found1 = 1; }
						if ( faces[j] == face2 ) { id2 = j; found2 = 1; }
					} 
					// if face1 is new
					if ( found1 == 0 ) { 
						faces . push_back ( face1 );
						id1 = counter;
						++counter;
					}
					// if face2 is new
					if ( found2 == 0 ) {
						faces . push_back ( face2 );
						id2 = counter;
						++counter;
					}
					// Add charts if necessary 
					// map between two new faces
					if ( found1 == 0 && found2 == 0 ) {

						// std::cout << "These two faces are new : \n";
						// std::cout << face1;
						// std::cout << face2;
						// std::cout << "dir1, dir2 : " << dir1 << " " << dir2 <<"\n";

						std::vector < double > lb1, ub1, lb2, ub2;
						// the charts are codimension-1;
						for ( unsigned int i=0; i<dim; ++i ) {
							if ( i != dir1 ) {
								lb1 . push_back ( face1 . rect . lower_bounds [ i ] );
								ub1 . push_back ( face1 . rect . upper_bounds [ i ] );
							}
							if ( i != dir2 ) { 
								lb2 . push_back ( face2 . rect . lower_bounds [ i ] );
								ub2 . push_back ( face2 . rect . upper_bounds [ i ] );
							}
						}

						// std::cout << "lb1 : ";
						// for ( unsigned int i=0; i<lb1.size(); ++i ) {
						// 	std::cout << lb1 [ i ] << " ";
						// }
						// std::cout << "\n";
						// std::cout << "ub1 : ";
						// for ( unsigned int i=0; i<ub1.size(); ++i ) {
						// 	std::cout << ub1 [ i ] << " ";
						// }
						// std::cout << "\n";
						// std::cout << "lb2 : ";
						// for ( unsigned int i=0; i<lb2.size(); ++i ) {
						// 	std::cout << lb2 [ i ] << " ";
						// }
						// std::cout << "\n";
						// std::cout << "ub2 : ";
						// for ( unsigned int i=0; i<lb2.size(); ++i ) {
						// 	std::cout << ub2 [ i ] << " ";
						// }
						// std::cout << "\n";

						//
						atlas -> add_chart ( counter - 2 , RectGeo ( dim-1, lb1, ub1 ) );
						//
						ofile2 << counter - 2 << " " << face1.direction << " ";
						for ( unsigned int j=0; j<face1.rect.dimension(); ++j ) {
							ofile2 << face1.rect.lower_bounds[j] << " ";
							ofile2 << face1.rect.upper_bounds[j] << " ";
						}
						ofile2 << "\n";
						//
						atlas -> add_chart ( counter - 1 , RectGeo ( dim - 1, lb2, ub2 ) );
						//
						ofile2 << counter - 1 << " " << face2.direction << " ";
						for ( unsigned int j=0; j<face2.rect.dimension(); ++j ) {
							ofile2 << face2.rect.lower_bounds[j] << " ";
							ofile2 << face2.rect.upper_bounds[j] << " ";
						}
						ofile2 << "\n";
					} 
					// map from existing face to new face
					if ( found1 == 0 && found2 != 0 ) {
						std::vector < double > lb1, ub1;
						// the charts are codimension-1;
						for ( unsigned int i=0; i<dim; ++i ) { 
							if ( i != dir1 ) {
								lb1 . push_back ( face1 . rect . lower_bounds [ i ] );
								ub1 . push_back ( face1 . rect . upper_bounds [ i ] );
							}
						}			
						//			
						atlas -> add_chart ( counter - 1 , RectGeo ( dim - 1, lb1, ub1 ) );
						//
						ofile2 << counter - 1 << " " << face1.direction << " ";
						for ( unsigned int j=0; j<face1.rect.dimension(); ++j ) {
							ofile2 << face1.rect.lower_bounds[j] << " ";
							ofile2 << face1.rect.upper_bounds[j] << " ";
						}
						ofile2 << "\n";
					}
					// map from new face to existing face
					if ( found1 != 0 && found2 == 0 ) {
						std::vector < double > lb2, ub2;
						// the charts are codimension-1;
						for ( unsigned int i=0; i<dim; ++i ) {
							if ( i != dir2 ) { 
								lb2 . push_back ( face2 . rect . lower_bounds [ i ] );
								ub2 . push_back ( face2 . rect . upper_bounds [ i ] );
							}
						}
						//
						atlas -> add_chart ( counter - 1, RectGeo ( dim - 1, lb2, ub2 ) );
						//
						ofile2 << counter - 1 << " " << face2.direction << " ";
						for ( unsigned int j=0; j<face2.rect.dimension(); ++j ) {
							ofile2 << face2.rect.lower_bounds[j] << " ";
							ofile2 << face2.rect.upper_bounds[j] << " ";
						}
						ofile2 << "\n";
					}
					//
					// Define the Maps
					//
					// Standard Case : face1 -> face2 with face2 not a fixed point. (face1 cannot be a fixed point)
					if ( face1.direction != -1 && face2.direction != -1 ) {
				
						Map map ( 0, id2, mygamma, mysigma, face1, face2 );
						atlasmap . addMap ( id1, id2, map );
						ofile << id1 <<"->" << id2 <<"\n";
					//	
					// Second Case : face1 -> face2 with face1:normal and face2:fixed point
					//
					} else if ( face1.direction != -1 && face2.direction == -1 ) {
						
						Map map ( 2, id2, mygamma, mysigma, face1, face2 );
						atlasmap . addMap ( id1, id2, map );
						ofile << id1 <<"->" << id2 <<"\n"; 
					// 
					// Third Case : face1 -> face2 with face1:fixed point (face2 must be the same )
					} else if ( face1.direction == -1 && face2.direction==-1 ) { 
						Map map ( 1, id2, mygamma, mysigma, face1, face2 ); // like Identity
						atlasmap . addMap ( id1, id2, map );
						ofile << id1 <<"->" << id2 <<"\n"; 
					//
					} 
				// if we map to a boundary
				} else {
					// std::cout << "@@@@@ we map to a boundary @@@@@\n";
					int id1;
					int found1 = 0;
					for ( unsigned int j=0; j<faces.size(); ++j ) {
						if ( faces[j] == face1 ) { id1 = j; found1 = 1; }
					} 
					// if face1 is new, add it to the list and also the chart in atlas
					if ( found1 == 0 ) { 
						faces . push_back ( face1 );
						id1 = counter;
						++counter;
					// to be changed later
						std::vector < double > lb2, ub2;
						for ( unsigned int i=0; i<dim; ++i ) {
							if ( i != dir2 ) { 
								lb2 . push_back ( face2 . rect . lower_bounds [ i ] );
								ub2 . push_back ( face2 . rect . upper_bounds [ i ] );
							}
						}
						//
						atlas -> add_chart ( counter - 1 , RectGeo ( dim-1, lb2, ub2 ) );
						//
						ofile2 << counter - 1 << " " << face1.direction << " ";
						for ( unsigned int j=0; j<face1.rect.dimension(); ++j ) {
							ofile2 << face1.rect.lower_bounds[j] << " ";
							ofile2 << face1.rect.upper_bounds[j] << " ";
						}
						ofile2 << "\n";

					}
					// This is not really a valid map between faces because it means 
					// the GridElements are mapped outside the phasespace and are lost.
					// The map will return a GridElement that cannot be covered.
					Map map ( 3, id1, mygamma, mysigma, face1, face2 ); //
					atlasmap . addMap ( id1, id1, map ); // add to atlasmap
					// WARNING ABOVE WE DECLARE THE MAP AS FACE1->FACE1, bad choice 
					// In Graphviz it will show up as a map to chart = -1, meaning lost (outside the phase space)
					ofile << id1 <<"-> -1" <<"\n"; 
				}
			} else {
// 				// std::cout << "Face1 is on a phase space boundary \n";
// 				// std::cout << face1 <<"\n";
			}	
		}
	}

	exportFaces ( faces );

	ofile << "}";
	ofile.close();

	ofile2.close();

}


// // Extract all the faces of the given BooleanBox
// // if the faces are not part of the vector faces then they are added
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

void extract_faces ( std::vector < Face > * faces, const std::vector < BooleanBox > & boxes ) { 
	for ( unsigned int i=0; i<boxes.size(); ++i ) {
		extract_faces ( faces, boxes[i] );
	}
}


// Save the faces into a text file that can be open to plot them 
// using for instance plot_faces.py
// faces are given in the following format :
//
// id lb0 ub0 lb1 ub1 ...
//
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


#endif