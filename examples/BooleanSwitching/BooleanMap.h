#ifndef BOOLEANSWITCHINGCHARTMAP_H
#define BOOLEANSWITCHINGCHARTMAP_H

#include "database/structures/RectGeo.h"

// RectMap

class BooleanChartMap {
public:
  BooleanChartMap ( void ) { }  
  BooleanChartMap ( const RectGeo & chart1, 
                    const RectGeo & chart2 ) 
  : chart1_(chart1),chart2_(chart2) {}

  RectGeo operator () ( const RectGeo & geo ) const {
    return chart2_;
  }

private:
  RectGeo chart1_, chart2_;
};

#endif
