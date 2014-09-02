#ifndef _AnnotationConditions_h
#define _AnnotationConditions_h

// Convention for the condition string
// Symbol : Description : extra information
//
// Symbol will be used to label the Morse Sets
// if we have extra information, it will be added to the label
//
// The code may add extra information however every conditional
// test will be done only over symbol : description
//

#define CONDITION0STRING "FP : Morse Set is a fixed point (FP OFF and FP ON not included)"
#define CONDITION1STRING "FP OFF : Morse Set is a fixed point with all states off"
#define CONDITION2STRING "FP ON : Morse Set is a fixed point with all states on"
#define CONDITION3STRING "FC : Morse Set in which every variable makes a transition"
#define CONDITION4STRING "XC : Morse Set with transition given by { }"

#endif
