#ifndef CONSTRUCTBOOLEANMAPS_H
#define CONSTRUCTBOOLEANMAPS_H

#include <vector>
#include <utility>
#include <exception>
#include <sstream>

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

    // build the corresponding key for the look up table
    // Face with coding : 1...10...0 with k 1's for "k-dimensional face"
    Face facekey;
    facekey . assign ( dim, 0 );
    int dimface = faceDimension(closestface);
    for ( unsigned int i=0; i<dimface; ++i ) {
        facekey[i]=1;
    }
    
    // DEBUG BEGIN
    if ( dim < dimface ) {
        std::stringstream ss;
        ss << "BooleanPairFacesMaps. dim = " << dim << " and dimface = " << dimface << "\n";
        throw std::logic_error ( ss . str () );
    }
    if ( lut . find ( facekey ) == lut . end () ) {
        std::stringstream ss;
        ss << "BooleanPairFacesMaps. LUT does not have facekey. dim = " << dim << "\n";
        throw std::logic_error ( ss . str () );
    }
    // DEBUG END

    // find the list of maps from the look-up table
    std::vector < std::pair < CFace, CFace > > listpair = lut.find(facekey)->second;

    std::vector < std::pair < CFace, CFace > >::iterator it;

// DEBUG BEGIN
#if 1
// DEBUG END
    // returns the set of reflections to go from the closest face 
    // to the corresponding face used for the key in the look-up table
    Reflections reflections=reflectionComposition(closestface);

    // Transform back the codimension 1 face in listpair;
    for ( it=listpair.begin(); it!=listpair.end(); ++it ) {
        //DEBUG BEGIN
        bool equal_before = (it -> first == it -> second);
        CFace a = it -> first;
        CFace b = it -> second;
        //DEBUG END
        (*it).first = convertBack ( dim, (*it).first, reflections );
        (*it).second = convertBack ( dim, (*it).second, reflections );
        //DEBUG BEGIN
        bool equal_after = (it -> first == it -> second);
        if ( equal_before != equal_after ) {
            std::cout << a << " : " << b << " --> " << it->first << " : " << it->second << "\n";
            throw std::logic_error ( "Unequal faces reflected into equal ones.\n");
        }
        //DEBUG END
    }
// DEBUG BEGIN
#endif
// DEBUG END

// DEBUG BEGIN

typedef std::pair<CFace, CFace> CFacePair;
std::vector < CFacePair > debug_listpair = 
    BooleanSwitchingMaps ( closestface );
boost::unordered_set< CFacePair > 
    arnaud_listpair (listpair.begin(), listpair.end() );
boost::unordered_set< CFacePair > 
    shaun_listpair (debug_listpair.begin (), debug_listpair.end() );
bool failflag = false;
BOOST_FOREACH ( CFacePair p, arnaud_listpair ) {
    if ( shaun_listpair . count ( p ) == 0 ) {
        std::cout << " arnaud_listpair contains (" << p.first << ", " << p.second << ") but shaun_listpair does not.\n";
        failflag = true;
    }
}
BOOST_FOREACH ( CFacePair p, shaun_listpair ) {
    if ( shaun_listpair . count ( p ) == 0 ) {
        std::cout << " shaun_listpair contains (" << p.first << ", " << p.second << ") but arnaud_listpair does not.\n";
        failflag = true;
    }
}
if ( failflag ) {
    std::cout << "closestface Input: \n";
        for ( int d = 0; d < closestface . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << closestface [ d ];
        }
        std::cout << "\n";
    throw std::logic_error ( "Two methods for chart pairs return different results.\n");
}
// DEBUG END
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
