/* Integrator for Database using CAPD */
// contributed by T. Kapela

#ifndef CMDP_VANDERPOLTK_H
#define CMDP_VANDERPOLTK_H

//#include "capd/krak/krak.h"
#undef None
#undef MIN
#undef MAX
#include "capd/capdlib.h"
#include "OdeMap.h"

#include "chomp/Rect.h"
//#include "database/structures/RectGeo.h"

class ModelMap : public OdeMap {
public:
  
  typedef chomp::Rect Rect;
  typedef Rect image_type;
  ModelMap ( void ) {
    std::cout << "VandelPolRect default constructor.\n";
  }
  
  ModelMap ( 
          const Rect & params , 
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
