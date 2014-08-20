#ifndef REFLECTION_H
#define REFLECTION_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <exception>
#include <sstream>

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

    // START DEBUG
    Face debug_face = input;
    Face debug_target_face ( input . size (), 0 );
    for ( int d = 0; d < faceDimension ( input ); ++ d ) 
        debug_target_face [ d ] = 1;

    // END DEBUG

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
        if ( input[i] == 2 )    { 
            // reflection is needed bc of the container reflections 
            // (not the best, should be changed)
            reflection . push_back ( i ); 
            reflections . push_back ( reflection );
            // DEBUG BEGIN
            debug_face [ i ] = 2 - debug_face [ i ];
            // DEBUG END
        }
    }
    // up to here output, contains the reflection with respect to one spatial 
    // direction 
    // needs to do the diagonal reflection 
#if 0
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
            // DEBUG BEGIN
            std::swap ( debug_face [ counter - 1 ], debug_face [ diagreflection [ i ] ] );
            // DEBUG END
        }
    }
#else
// PATCH BEGIN 
    int counter=0;
    for ( unsigned int i=0; i<diagreflection.size(); ++i ) {
        while ( input [ counter ] == 1 ) ++ counter;
        if ( diagreflection[i] < dimension ) continue;
        std::vector < int > tmp;
        tmp . push_back ( counter );
        tmp . push_back ( diagreflection [ i ] );
        reflections . push_back ( tmp );
        ++ counter;
        std::swap ( debug_face [ counter - 1 ], debug_face [ diagreflection [ i ] ] );
    }
// PATCH END
#endif


    // DEBUG BEGIN
    bool failflag = false;
    for ( int d = 0; d < debug_face . size (); ++ d ) {
        if ( debug_face [ d ] != debug_target_face [ d ] ) {
            failflag = true;
        }
    }
    if ( failflag ) {
        std::cout << "Input: \n";
        for ( int d = 0; d < input . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << input [ d ];
        }
        std::cout << "\n";

        std::cout << "Debug Target Face: \n";
        for ( int d = 0; d < debug_target_face . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << debug_target_face [ d ];
        }
        std::cout << "\n";
        
        std::cout << "Debug Face: \n";
        for ( int d = 0; d < debug_face . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << debug_face [ d ];
        }
        std::cout << "\n";

        throw std::logic_error ( "reflectionComposition did not reflect into standard position.\n");
    }

    Face converted_back = convertBack ( debug_face, reflections );
    for ( int d = 0; d < converted_back . size (); ++ d ) {
        if ( converted_back [ d ] != input [ d ] ) {
            failflag = true;
        }
    }

    if ( failflag ) {
        std::cout << "Input: \n";
        for ( int d = 0; d < input . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << input [ d ];
        }
        std::cout << "\n";

        std::cout << "Debug Target Face: \n";
        for ( int d = 0; d < debug_target_face . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << debug_target_face [ d ];
        }
        std::cout << "\n";
        
        std::cout << "Debug Face: \n";
        for ( int d = 0; d < converted_back . size (); ++ d ) {
            if ( d != 0 ) std::cout << ", ";
            std::cout << converted_back [ d ];
        }
        std::cout << "\n";

        throw std::logic_error ( "convertBack did not restore original position.\n");
    }
    // DEBUG END
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
            // BEGIN DEBUG
            if ( r[0] >= output . size () ) {
                throw std::logic_error ( "convertBack. memory corruption!\n");
            }
            // END DEBUG
            //if ( output [ r[0] ] == 0 ) {
            //  output [ r[0] ] = 2;
            //} else if ( output [ r[0] ] == 2) {
            //  output [ r[0] ] = 0;
            //}
            output [ r [ 0 ] ] = 2 - output [ r [ 0 ] ];
        } else if ( r.size() == 2 ) {
            // BEGIN DEBUG
            if ( r[0] >= output . size () ) {
                throw std::logic_error ( "convertBack. memory corruption (CASE A)!\n");
            }
            if ( r[1] >= output . size () ) {
                throw std::logic_error ( "convertBack. memory corruption (CASE B)!\n");
            }
            // END DEBUG
            
            //int tmp = output [ r[0] ];
            //output [ r[0] ] = output [ r [1] ];
            //output [ r[1] ] = tmp;

            std::swap ( output[r[0]], output[r[1]] );
        } else { 
            throw std::logic_error ( "convertBack. Invalid reflections.");
            //std::cout << "Something wrong with the reflections\n";
            //abort();
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
        // BEGIN DEBUG
        if ( cface-1 >= face . size () ) {
            throw std::logic_error ( "convertBack. memory corruption (CASE C)!\n");
        }
        // END DEBUG
        face [ cface-1 ] = 2;
    } else if ( cface < 0 ) { // SRH: added the if( cface < 0) code
        // BEGIN DEBUG
        if ( std::abs(cface)-1 >= face . size () ) {
            std::stringstream ss;
            ss <<  "convertBack. memory corruption (CASE D)! abs(cface)-1=" << std::abs(cface)-1 
            << " and face.size()=" << face . size () << "\n";
            throw std::logic_error ( ss . str () );
        }
        // END DEBUG
        face [ std::abs(cface)-1 ] = 0;
    } else if ( cface == 0 ) { // SRH: added to deal with cface == 0 case
        return cface;
    }

    Face newface = convertBack ( face, reflections );
    
    CFace output;
    
    // BEGIN DEBUG
    bool failflag = false;
    bool singleset = false;
    // END DEBUG

    // convert face into a CFace
    for ( unsigned int i=0; i<dim; ++i ) {
        // BEGIN DEBUG
        if ( i >= newface . size () ) {
            throw std::logic_error ( "convertBack. memory error (CASE E)!\n");
        }
        // END DEBUG
        if ( newface [ i ] == 0 ) {
            output = -(i+1);
            // BEGIN DEBUG
            if ( singleset ) failflag = true;
            singleset = true;
            // END DEBUG
        } else if ( newface [ i ] == 2 ) {
            output = i+1;
            // BEGIN DEBUG
            if ( singleset ) failflag = true;
            singleset = true;
            // END DEBUG
        }
    }
    // BEGIN DEBUG
    if ( failflag ) {
        throw std::logic_error ( "convertBack overwrote its own answer\n");
    }
    // END DEBUG
    return output;
}

#endif
