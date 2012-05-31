
#include <chomp/Rect.h>

class HyperbolicLinearMap {
 public:
  chomp::Rect operator()(const chomp::Rect & in) const {
    chomp::Rect out(2);
    out.lower_bounds[0] = in.lower_bounds[0]*0.3;
    out.upper_bounds[0] = in.upper_bounds[0]*0.3;
    out.lower_bounds[1] = in.lower_bounds[1]*3.0;
    out.upper_bounds[1] = in.upper_bounds[1]*3.0;
    return out;
  }
};

class ContractingLinearMap {
 public:
  chomp::Rect operator()(const chomp::Rect & in) const {
    chomp::Rect out(2);
    out.lower_bounds[0] = in.lower_bounds[0]*0.3;
    out.upper_bounds[0] = in.upper_bounds[0]*0.3;
    out.lower_bounds[1] = in.lower_bounds[1]*0.3;
    out.upper_bounds[1] = in.upper_bounds[1]*0.3;
    return out;
  }
};

class ExpandingLinearMap {
 public:
  chomp::Rect operator()(const chomp::Rect & in) const {
    chomp::Rect out(2);
    out.lower_bounds[0] = in.lower_bounds[0]*3;
    out.upper_bounds[0] = in.upper_bounds[0]*3;
    out.lower_bounds[1] = in.lower_bounds[1]*3;
    out.upper_bounds[1] = in.upper_bounds[1]*3;
    return out;
  }
};
