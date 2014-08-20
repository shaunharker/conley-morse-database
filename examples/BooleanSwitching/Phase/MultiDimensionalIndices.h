#ifndef MULTIDIMENSIONALINDICES_H
#define MULTIDIMENSIONALINDICES_H 

#include <vector>
#include <iterator>

class MultiDimensionalIterator 
  : public std::iterator< std::forward_iterator_tag, const std::vector<size_t> > {
public:
  MultiDimensionalIterator ( void ) {}
  MultiDimensionalIterator ( const std::vector<size_t> * limits_, bool end_ ) 
  : limits_(limits_), end_(end_) {
    dimension_ = limits_ -> size ();
    data_ . resize ( dimension_, 0 );
  }
  const std::vector<size_t> & operator * ( void ) const {
    return data_;
  }
  MultiDimensionalIterator & operator ++ ( void ) {
    if ( end_ ) return *this;
    end_ = true;
    for ( size_t d = 0; d < dimension_; ++ d ) {
      ++ data_ [ d ];
      if ( data_ [ d ] == (*limits_) [ d ] ) {
        data_ [ d ] = 0;
      } else { 
        end_ = false;
        break;
      }
    }
    return *this;
  }
  bool operator == ( const MultiDimensionalIterator & rhs ) const {
    if ( end_ != rhs . end_ ) return false;
    for ( size_t d = 0; d < dimension_; ++ d ) {
      if ( data_ [ d ] != rhs . data_ [ d ] ) return false;
    }
    return true;
  }
  bool operator != ( const MultiDimensionalIterator & rhs ) const {
    if ( not ( *this == rhs ) ) return true;
    return false;
  }
private:
  size_t dimension_;
  std::vector<size_t> data_;
  const std::vector<size_t> * limits_;
  bool end_;
};

class MultiDimensionalIndices {
public:
  typedef MultiDimensionalIterator iterator;
  typedef MultiDimensionalIterator const_iterator;
  iterator begin ( void ) const {
    return iterator ( &limits_, false );
  }
  iterator end ( void ) const {
    return iterator ( &limits_, true);
  }
  size_t size ( void ) const {
    size_t result = 1;
    for ( size_t d = 0; d < limits_ . size (); ++ d ) {
      result *= limits_ [ d ];
    }
    return result;
  }
  MultiDimensionalIndices ( void ) {}
  MultiDimensionalIndices ( const std::vector<size_t> & limits )
  : limits_ ( limits ) {}

  void assign ( const std::vector<size_t> & limits ) {
    limits_ = limits;
  }

  const std::vector<size_t> & limits ( void ) const {
    return limits_;
  }
  
private:
  std::vector<size_t> limits_;
};

#endif
