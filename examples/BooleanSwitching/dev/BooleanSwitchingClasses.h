#ifndef BOOLEANSWITCHINGCLASSES_H
#define BOOLEANSWITCHINGCLASSES_H

#include <vector>

// Declare basic types used for the Boolean Switching Network

// Coding for a Face: 
// 0 : x_i = A_i
// 1 : x_i in [ A_i, B_i ]
// 2 : x_i = B_i
typedef std::vector < int > Face;

// Codimension 1 Face (just an int to save space)
// j : codimension 1 Face in the abs(j) - 1 spatial direction (degenerate direction) 
//     with "Front/Back" information from the sign "+/-" 
typedef int CFace;

// Coordinates of the subdomain 
// x_i in 0...m_i with m_i : # thresholds in the i-th direction
typedef std::vector < size_t > Domain;

// Coding for the semi-axis (positive or negative)
// used for the closest axis information
// x_i = 0 is a special case (discarded)
// spatial direction = abs(x_i) - 1
typedef int Axis;

// return the dimension of the face
inline int 
faceDimension ( const Face & face ) {
    int counter = 0;
    for ( Face::const_iterator it=face.begin(); it!=face.end(); ++it ) {
        if ( *it == 1 ) { ++counter; }
    }
    return counter;
}

#endif
