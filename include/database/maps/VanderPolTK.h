/* Integrator for Database using CAPD */

#ifndef CMDP_INTEGRATOR_H
#define CMDP_INTEGRATOR_H

//#include "capd/krak/krak.h"
#undef MIN
#undef MAX
#include "capd/capdlib.h"
#include "OdeMap.h"

#include "database/structures/RectGeo.h"

class VanderPolRect : public OdeMap {
public:
  
  typedef RectGeo Rect;
  typedef RectGeo image_type;
  VanderPolRect ( void ) {
    std::cout << "VandelPolRect default constructor.\n";
  }
  
  VanderPolRect ( 
          const RectGeo & params , 
          int order = 10, 
          double timeOfIntegration = 1./8.,
          double errorTolerance = 1.0e-4
  ) {
    std::cout << "VandelPolRect constructor.\n";
    using namespace capd;
    dim = 2;
    f . reset ( new IMap("par:c;var:x,y;fun:y,-x+c*(1-x^2)*y;") );
 
    f -> setParameter ( "c", interval(params.lower_bounds[0],
                                      params.upper_bounds[0]));
    this->order = order;
    integrationTime = timeOfIntegration;
    
    solver . reset ( new ITaylor (*f, order));//, capd::dynsys::ILastTermsStepControl(2, 1.e-7, 1.e-5)) );
    solver-> setRelativeTolerance(errorTolerance);
    solver-> setAbsoluteTolerance(errorTolerance);
    timeMap . reset (new ITimeMap(*solver));
    //solver->setStep(step);
    return;
  }
};

#endif
