/*
 *  Extendible_Array.h
 */

#ifndef _CMDP_EXTENDIBLE_ARRAY_
#define _CMDP_EXTENDIBLE_ARRAY__

#include <vector>

/// An extendible array of objects of given type.
/// Works essentially like std::vector, except each object in the array
/// is created only once and is not copied while increasing or decreasing
/// the size of the array.
/// In particular, the address of each element of the array is constant
/// for its entire lifetime.
/// This structure is useful for creating arrays of objects
/// that are initially small, but then change and become larger.
/// One of possible ideas to implement this data structure
/// is to use std::vector of pointers to dynamically allocated objects,
/// and to delete the objects pointed-to in the destructor.
template < class T >
class Extendible_Array
{
  public:
    ~Extendible_Array ();

  private:
    // The vector of addresses of objects stored in the array.
    //std::vector < T * > vect;
};

#ifndef _DO_NOT_INCLUDE_HPP_
#include "data_structures/Extendible_Array.hpp"
#endif

#endif

