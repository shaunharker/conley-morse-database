/*
 *  Configuration.h
 */


#ifndef _CMDP_CONFIGURATION_
#define _CMDP_CONFIGURATION_


/* Toplex Choice */
#include "toplexes/Adaptive_Cubical_Toplex.h"
using namespace Adaptive_Cubical;

/* Parameter Space */
#define PARAM_DIMENSION 2
#define PARAM_SUBDIVISIONS 6
#define MAX_PATCH_SIZE 10

const Real param_lower_bounds [PARAM_DIMENSION] = { 8.0, 3.0 };
const Real param_upper_bounds [PARAM_DIMENSION] = { 37.0, 50.0 };

const Geometric_Description param_bounds ( PARAM_DIMENSION , std::vector<Real> ( param_lower_bounds, 
                                                      param_lower_bounds + PARAM_DIMENSION ), 
                                                      std::vector<Real> ( param_upper_bounds, 
                                                      param_upper_bounds + PARAM_DIMENSION ) );

/* Phase Space */
#ifndef SPACE_DIMENSION
#define SPACE_DIMENSION 2
#endif
#ifndef MIN_PHASE_SUBDIVISIONS
#define MIN_PHASE_SUBDIVISIONS 12
#endif
#ifndef MAX_PHASE_SUBDIVISIONS
#define MAX_PHASE_SUBDIVISIONS 16
#endif
#ifndef COMPLEXITY_LIMIT
#define COMPLEXITY_LIMIT 10000
#endif
const Real space_lower_bounds [SPACE_DIMENSION] = { 0.0, 0.0 };
const Real space_upper_bounds [SPACE_DIMENSION] = { 320.056, 224.040 };

const Geometric_Description space_bounds ( SPACE_DIMENSION , std::vector<Real> ( space_lower_bounds, 
                                                      space_lower_bounds + SPACE_DIMENSION ), 
                                  std::vector<Real> ( space_upper_bounds, 
                                                      space_upper_bounds + SPACE_DIMENSION ) );

/* Map Choice */
//#include "maps/fisheries.h"
//typedef FishMap4 GeometricMap;
#include "maps/leslie.h"
typedef LeslieMap GeometricMap;


#ifndef _DO_NOT_INCLUDE_HPP_
#include "program/Configuration.hpp"
#endif

#endif
