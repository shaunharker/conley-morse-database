// IAS_MapHelper.h
// Author: Aniket Shah

#ifndef IAS_MAPHELPER_H
#define IAS_MAPHELPER_H

#include <utility>
#include <cmath>
#include <exception>
#include <stdexcept>
#include "database/numerics/boost_interval.h"

/// expMinRoot
///   Find the minimal root in t \in [ t_min, \infty )
///        f(t) = A exp ( at ) + B exp ( bt ) + C exp (ct) + D
double 
expMinRoot (double a, double b, double c,
            double A, double B, double C, double D, 
            double t_min);

/// expMinRoot
///   Find an enclosure of the minimal root in t \in [ t_min, \infty )
///        f(t) = A exp ( at ) + B exp ( bt ) + C exp (ct) + D, where A, B, C may be intervals.
std::pair<interval,int> 
expMinRoot (interval a, interval b, interval c, 
            interval A, interval B, interval C, double D, 
            double t_min);


/// Definitions
namespace ias_detail {

class ExponentialCombination {
private:
  double a_;
  double b_;
  double c_;
  double A_;
  double B_;
  double C_;
  double D_;
  double t_min_;
    double t_max_;
public:
    double tmax (void) {
        return t_max_;
    }
  ExponentialCombination ( void ) {}
  ExponentialCombination ( double a, double b, double c, 
                            double A, double B, double C, 
                            double D, double t_min ) :
    
    a_(a), b_(b), c_(c), A_(A), B_(B), C_(C), D_(D), t_min_(t_min) {
        //make sure equation has appropriate form
        if (a>=b and b>=c){
            a_=a, b_=b, c_=c, A_=A, B_=B, C_=C, D_=D, t_min_=t_min;
        } else if (a>=c and c>=b) {
            a_=a, b_=c, c_=b, A_=A, B_=C, C_=B, D_=D, t_min_=t_min;
        } else if (b>=a and a>=c) {
            a_=b, b_=a, c_=c, A_=B, B_=A, C_=C, D_=D, t_min_=t_min;
        } else if (b>=c and c>=a) {
            a_=b, b_=c, c_=a, A_=B, B_=C, C_=A, D_=D, t_min_=t_min;
        } else if (c>=a and a>=b) {
            a_=c, b_=a, c_=b, A_=C, B_=A, C_=C, D_=B, t_min_=t_min;
        } else /*if (c>=b and b>=a)*/ {
            a_=c, b_=b, c_=a, A_=C, B_=B, C_=A, D_=D, t_min_=t_min;
        }
        //now a>=b>=c
        
        if (A_==0){
            A_=B_;
            B_=C_;
            C_=0;
        }
        
        if (A_==0){
            A_=B_;
            B_=0;
        }
        
        if (A_==0) {
            throw std::logic_error ("function constant, should not occur");
        }
        
        double sup_bc;
        if ( b>c) {
            sup_bc = b;
        } else {
            sup_bc = c;
        }

        double inf_2B2C;
        if ( B>C) {
            inf_2B2C = 2*C;
        } else {
            inf_2B2C = 2*B;
        }
        t_max_ = log(-1*(inf_2B2C-10)/A)/(a-sup_bc)+1;
    }
    

  double operator () ( double x ) const {
    return A_*std::exp(a_ * (x+t_min_)) + B_*std::exp(b_ * (x+t_min_)) 
           + C_*std::exp(c_ * (x+t_min_)) + D_;
  }
};

template < class functionObject >
double find_zero ( const functionObject & f, 
                   double left,
                   double right ) {
  // assume switching sign 
  if ( f(left) * f(right) > 0 ) {
    //throw std::logic_error ( "find_zero: expected sign " 
    //                         "           change between left and right.\n");
    throw std::logic_error ( "find_zero: expected sign " 
                             "           change between left and right.\n");
  }

  double tol = 1e-8;
  while ( right - left > tol ) {
    double x = (left + right) / 2.0;
    if ( f(left) * f(x) <= 0 ) {
      right = x;
    } else {
      left = x;
    }
  }

  return left;
}

} // namespace ias_detail

inline
double expMinRoot (double a, double b, double c,
                double A, double B, double C, double D, double t_min){
  using namespace ias_detail;
  ExponentialCombination f = ExponentialCombination(a, b, c, A, B, C, D,t_min);
 
 if ((A>=0 && B>=0 && C>=0 && D>=0)||(A<=0 && B<=0 && C<=0 && D<=0))
  {
    throw std::logic_error ( "function is monotonic and will not hit zero\n");
  } else if (A<=0 && B<=0 && C<=0)
  {
    throw std::logic_error ( "function is monotonically decreasing, which is unrealistic\n");
  }

  ExponentialCombination fder = ExponentialCombination(a, b, c, A*a, B*b, C*c, 0, t_min);

  //upper bound for checks, should be big enough for f and f'
    double fbig = f.tmax();

  //zero of f'
  double fder_zero1, fder_zero2;

  //if argument of log function to determine root of g' is negative, probably means that g' has no root? so g has no crit pts and is monotonic
  //thus f' is monotonic, since it is the product of two monotonic functions. thus run findZero on f'. 0 to fderbig
  
  // Define g(x) := f'(x) / exp(Cx)

  // Case 1. g has fewer than 2 zeros.
  double tilde_log_argument = B*b*(b-c) / (A*a*(a-c));

  if ( tilde_log_argument <= 0 ) {
    // Then g is monotonic.
    // Thus f' is monotonic and has at most one zero.

    // If f switches sign on [0, fbig] we can 
    // use find_zero to find root.

    bool f_switches_sign = ( f(0) * f(fbig) <= 0 );
    if ( f_switches_sign ) {
      return find_zero(f, 0, fbig);
    }

    // So f does not switch sign on the window [0, fbig]
    // It may be that there is a critical point of f in the
    // window [0, big]. We check this.
    bool fder_switches_sign = ( fder(0) * fder(fbig) <= 0 );
    if ( not fder_switches_sign ) { //f' always pos or neg, so f monotonic
      throw std::logic_error ( "function will not hit zero\n");
    } 
    // We find the critical point of f in the window [0, fbig]
    fder_zero1 = find_zero(fder, 0, fbig); 
    // We check if f switches sign in the window [0, fder_zero1]
    f_switches_sign = (f(0) * f(fder_zero1) <= 0);
    if ( f_switches_sign ) {
      return find_zero(f, 0, fder_zero1);
    } else {
      return find_zero(f, fder_zero1, fbig);
    }
  }


  // TODO: deal with special case a == b
  //now dealing with if t_tilde is well-defined
  double t_tilde = log ( tilde_log_argument ) / ( a - b );

  if ( t_tilde <= 0) { 
    // Then the zero of g' is negative
    // So g (and thus f' )is monotonic on [0, \infty) 
    
    bool fder_switches_sign = ( fder(0) * fder(fbig) <= 0 );
    if ( not fder_switches_sign ) { //f' always pos or neg, so f monotonic
      bool f_switches_sign_on_zero_fbig = 
        f(0)*f(fbig) <= 0;
      if ( f_switches_sign_on_zero_fbig ) {
        return find_zero(f, 0, fbig);
      } else {
        throw std::logic_error ( "no solution\n" );
      }
    } 
    // Find the solution to f'(x) = 0 for x \in [0, fbig]
    fder_zero1 = find_zero(fder, 0, fbig);
    bool f_switches_sign = 
      f(0)*f(fder_zero1) <= 0;
    if ( f_switches_sign ) {
      return find_zero(f, 0, fder_zero1);
    } else {
      // RIGHT HERE
      return find_zero(f, fder_zero1, fbig);
    }
  } else {  //if PSA is above 4
    if (fder(0)*fder(t_tilde)<=0 && fder(t_tilde)*fder(fbig)<=0) {
      fder_zero1 = find_zero(fder,0,t_tilde);
      fder_zero2 = find_zero(fder,t_tilde, fbig);
      if (f(0)*f(fder_zero1)<=0) {
        return find_zero(f,0,fder_zero1);
      } else if (f(fder_zero1)*fder_zero2<=0){
        return find_zero(f,fder_zero1,fder_zero2);
      } else if (f(fder_zero2)*f(fbig)<=0){
        return find_zero(f,fder_zero2,fbig);
      }

      //if in this case there are roots for positive t
      throw std::logic_error ( "no solution\n");

    } else if (fder(0)*fder(t_tilde)<=0 && fder(t_tilde)*fder(fbig)>0) { 
      fder_zero1 = find_zero(fder,0, t_tilde);
      if (f(0)*f(fder_zero1)<0) {
        return find_zero(f,0,fder_zero1);
      } else {
        return find_zero(f,fder_zero1,fbig);
      }

    } else if (fder(0)*fder(t_tilde)>0 && fder(t_tilde)*fder(fbig)<=0) {
      fder_zero1 = find_zero(fder,t_tilde, fbig);
      if (f(0)*f(fder_zero1)<0) {
        return find_zero(f,0,fder_zero1);
      } else {
        return find_zero(f,fder_zero1,fbig);
      }
    } else {
      return find_zero(f,0,fbig);
    }
  }
}

inline
std::pair<interval,int> 
expMinRoot (interval a, interval b, interval c,
            interval A, interval B, interval C, double D, double t_min){
  using namespace ias_detail;
  // Lower envelope
  ExponentialCombination f_lower = ExponentialCombination(a.lower(),b.lower(),c.lower(),A.lower(),B.lower(),C.lower(),D,t_min);
  // Upper envelope
  ExponentialCombination f_upper = ExponentialCombination(a.upper(),b.upper(),c.upper(),A.upper(),B.upper(),C.upper(),D,t_min);
  interval result;
  int result_type; //0 if all intersect, 1 if no intersection, 2 if some intersect and some don't
  bool upper_root_exists = true; // set to true if upper envelope has a root
  bool lower_root_exists = true; // set to true if lower envelope has a root
  double upper_root;
  double lower_root;


  //In this case the interval is completely positive at t=0
  if (f_lower(0)>0) {

    //if both upper and lower intersect, good
    //if only one intersects, then the interval points to a bad area, and a good area
    //if neither intersect, then only bad
    
    //error catching must reflect this
    try{
      upper_root = expMinRoot(a.upper(),b.upper(),c.upper(),A.upper(),B.upper(),C.upper(),D,t_min);
    }
    catch(std::logic_error){
      upper_root_exists = false;
    }
    
    try{
      lower_root = expMinRoot(a.lower(),b.lower(),c.lower(),A.lower(),B.lower(),C.lower(),D,t_min);
    }
    catch(std::logic_error){
      lower_root_exists = false;
    }

    if (lower_root_exists == false) { //then upper_root_exists will also be false
      result.assign(10,10); //whatever appropriate interval should be for empty interval
      result_type = 1;
      //if the interval is actually empty
    } else if (lower_root_exists == true && upper_root_exists == false) {
      //must be outer approximation for all the values in the interval that do cross 0
      double f_lowerder_root = expMinRoot(a.lower(),b.lower(),c.lower(),a.lower()*lower(A),b.lower()*lower(B),c.lower()*lower(C),0,
                        t_min+lower_root);

      result.assign(lower_root,expMinRoot(a.lower(),b.lower(),c.lower(),A.lower(),B.lower(),C.lower(),
                   D,t_min+f_lowerder_root));
      result_type = 2;
    } else if (upper_root_exists == true){
      result.assign(lower_root,upper_root);
      result_type = 0;
    } 

  //In this case the interval is completely negative at t=0
  } else if (f_upper(0)<0) {
    try{
      upper_root = expMinRoot(a.lower(),b.lower(),c.lower(),lower(A),lower(B),lower(C),D,t_min);
    }
    catch(std::logic_error){
      upper_root_exists = false;
    }

    try{
      lower_root = expMinRoot(a.upper(),b.upper(),c.upper(),upper(A),upper(B),upper(C),D,t_min);
    }
    catch(std::logic_error){
      lower_root_exists = false;
    }

    if (lower_root_exists == false) { //then upper_root_exists will also be false
      result.assign(10,10); //whatever the no intersection interval should be
      result_type = 1;
    } else if (lower_root_exists == true && upper_root_exists == false) {
      //must somehow account for all the values in the interval that do cross 0
      double f_upperder_root = expMinRoot(a.upper(),b.upper(),c.upper(),a.upper()*A.upper(),b.upper()*B.upper(),c.upper()*C.upper(),0,
                        t_min+lower_root);

      result.assign(lower_root,expMinRoot(a.upper(),b.upper(),c.upper(),A.upper(),B.upper(),C.upper(),
                   D,t_min+f_upperder_root));
      result_type = 2;
    } else if (upper_root_exists == true){
      result.assign(lower_root,upper_root);
      result_type = 0;
    }

  } else {
    throw std::logic_error ("interval starts partially negative and positive -- should not happen");
  }
  return std::make_pair(result, result_type);
}

#endif

