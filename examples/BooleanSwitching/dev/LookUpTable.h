#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <vector>
#include <cstdlib>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include "BooleanSwitchingClasses.h"

//============================
// VERSION 1 : NO CLOSEST AXIS 
//============================

// Construct the Lookup table for a generic box 
//
// The look up table will return a vector of pair of Codimension 1 faces
// Each pair represent a map F1 -> F2

// The key for the Look Up Table is a k-dimensional face
// with representation 1...10...0 ( k 1s and N-k 0s )
typedef boost::unordered_map < Face, std::vector < std::pair < CFace, CFace > > > LUT;


// Construct the look up table for a n-dimensional space
LUT constructLookUpTable ( int n );

// check if we have a map face1 -> face2
bool isValidMap ( const int & dim, 
                    const CFace & face1, 
                    const CFace & face2 );

//--------------//
// DEFINITIONS  // 
//--------------//
 
inline LUT 
constructLookUpTable ( int n ) {
    //
    LUT lookuptable;
    //
    // Special case (Fixed Point)

    Face specialfacekey ( n, 1 );
    //Face specialfacekey;
    //specialfacekey . assign ( n, 0 );
    //for ( unsigned int i=0; i<n; ++i ) { 
    //  specialfacekey [ i ] = 1;
    //} 
    CFace fixedpoint;
    fixedpoint = 0;
    std::vector < std::pair < CFace, CFace > > speciallistmaps;
    // construct the list of maps F1 -> FixedPoint
    for ( int f1=-n; f1<=n; ++f1 ) {
        // skip f1 = 0         // SRH: changed because I think it stopped fixed points from mapping to self
        //if ( f1 != 0 ) {     // SRH: changed because I think it stopped fixed points from mapping to self
            speciallistmaps . push_back ( std::pair < CFace, CFace > (f1,fixedpoint) ); 
        //}                    // SRH: changed because I think it stopped fixed points from mapping to self
    }
    lookuptable [ specialfacekey ] = speciallistmaps;

    // LOOP THROUGH THE K-DIMENSIONAL FACES
    // WITH K=0,...,n-1
    for ( unsigned int dim=0; dim<n; ++dim ) {
        // build the look up table key
        Face facekey;
        facekey . assign ( n, 0 ); 
        for ( unsigned int j=0; j<dim; ++j ) {
            facekey [ j ] = 1;
        }
        //
        // Consider maps between two codimension 1 faces
        // F1 -> F2
        // Note : F2 has to be adjacent to the facekey
        std::vector < std::pair < CFace, CFace > > listmaps;
        // Loop for F1
        // recall : direction = abs(f1) - 1
        for ( int f1=-n; f1<=n; ++f1 ) {
            // ignore f1 = 0 
            if ( f1 != 0 ) {
                // Loop for F2 and F2 is adjacent to facekey 
                for ( int f2=-(dim+1); f2>=-n; --f2) {
                    if ( f1 != f2 ) {
                        if ( isValidMap( dim, f1, f2) ) {
                            listmaps . push_back ( std::pair < CFace, CFace > (f1,f2) );
                        }
                    }
                }
            }
        } 
        lookuptable [ facekey ] = listmaps;
    }
    return lookuptable;
}

inline bool 
isValidMap ( const int & dim, 
               const CFace & face1, 
               const CFace & face2 ) {
// IMPLEMENT NOTES CONDITION 
// Here by construction face2 is adjacent to the k-face used for the LUT key

    // this is the case where a refinement is necessary 
    // but not considered here, so return always true
    if ( std::abs(face1)-1 == std::abs(face2)-1 ) {
        return true;
    } else { 
        if ( std::abs(face1)-1 > dim-1 ) {
            if ( face1 < 0 ) {
                return false;
            }
        }
    }
    return true;
}

#endif
