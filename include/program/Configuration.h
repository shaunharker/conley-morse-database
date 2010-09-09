/*
 *  Configuration.h
 */


#ifndef _CMDP_CONFIGURATION_
#define _CMDP_CONFIGURATION_


/* Toplex Choice */
#include "toplexes/Adaptive_Cubical_Toplex.h"
using namespace Adaptive_Cubical;

/* Parameter Space */
#define PS_DIMENSION 2
#define PS_SUBDIVISIONS 6
#define MAX_PATCH_SIZE 10

Real PS_lower_bounds [PS_DIMENSION] = { 8.0, 37.0 };
Real PS_upper_bounds [PS_DIMENSION] = { 3.0, 50.0 };

Geometric_Description PS_Bounds ( PS_DIMENSION , std::vector<Real> ( PS_lower_bounds, 
                                                      PS_lower_bounds + PS_DIMENSION ), 
                                  std::vector<Real> ( PS_upper_bounds, 
                                                      PS_upper_bounds + PS_DIMENSION ) );

/* Map Choice */
#include "maps/leslie.h"
typedef LeslieMap GeometricMap;


/* Default Settings */



#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Configuration.hpp"
#endif

#endif
