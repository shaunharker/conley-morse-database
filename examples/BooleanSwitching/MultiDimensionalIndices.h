#ifndef MULTIDIMENSIONALINDICES_H
#define MULTIDIMENSIONALINDICES_H 

class MultiDimensionalIndices {
public:
  class iterator {
  public:
    iterator ( void ) {}
    iterator ( const std::vector<size_t> * limits_, bool end_ ) 
    : limits_(limits_), end_(end_) {
      dimension_ = limits_ -> size ();
      data_ . resize ( dimension_, 0 );
    }
    const std::vector<int> & operator * ( void ) const {
      return data_;
    }
    iterator & operator ++ ( void ) {
      if ( end ) return *this;
      end_ = true;
      for ( size_t d = 0; d < dimension_; ++ d ) {
        ++ data [ d ];
        if ( data [ d ] == (*limits) [ d ] ) {
          data [ d ] = 0;
        } else { 
          end_ = false;
          break;
        }
      }
      return *this;
    }
    bool operator == ( const iterator & rhs ) const {
      if ( end_ != rhs . end_ ) return false;
      for ( size_t d = 0; d < dimension_; ++ d ) {
        if ( data [ d ] != rhs . data [ d ] ) return false;
      }
      return true;
    }
    bool operator != ( const iterator & rhs ) const {
      if ( not ( *this == rhs ) ) return true;
      return false;
    }
  private:
    size_t dimension_;
    std::vector<size_t> data_;
    std::vector<size_t> * limits_;
    bool end_;
  };
  iterator begin ( void ) const {
    return iterator ( &limits_, false );
  }
  iterator end ( void ) const {
    return iterator ( &limits_, true)
  }
  size_t size ( void ) const {
    size_t result = 1;
    for ( size_t d = 0; d < limits_ . size (); ++ d ) {
      result *= limits_ [ d ];
    }
    return result;
  }
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
