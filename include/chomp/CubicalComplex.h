// CubicalComplex.h
// Shaun Harker
// 9/13/11

#ifndef CHOMP_CUBICALCOMPLEX_H
#define CHOMP_CUBICALCOMPLEX_H

#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <stack>

#include "boost/unordered_set.hpp"

#include "chomp/Complex.h"
#include "chomp/Chain.h"
#include "chomp/Rect.h"


namespace chomp {
  
  /*******************************
   *      CUBICAL COMPLEX        *
   *******************************/
  class CubicalComplex : public Complex {
  public:
    // Cell type: uint64_t
    CHOMP_COMPLEX(uint64_t)
    /*******************************
     *      COMPLEX INTERFACE      *
     *******************************/
    /// boundary. See Complex.h 
    virtual void boundary ( Chain * output, const Index input, int dim ) const;
    /// coboundary. See Complex.h
    virtual void coboundary ( Chain * output, const Index input, int dim ) const;
    
    /*******************************
     * SPECIFIC TO CUBICAL COMPLEX *
     *******************************/
    
    /// FILE INTERFACE
    
    /// loadFromFile.
    /// Cubical format with full cubes. (n1, n2, ... ).
    /// It automatically translates the tuples so that the minimal entry for a given
    /// ordinate is set to 0.
    void loadFromFile ( const char * FileName );
    
    /// LOW-LEVEL INTERFACE
    /// To use, use calls "initialize" with a desired bitmap size, then
    /// addFullCube and removeFullCube, and finishes by calling "finalize"
    
    /// Important note on "bitmap" in this implementation:
    /// we use a hash-table implementation, so there is no actual
    /// bitmap in memory. rather, there is an interface which 
    /// simulates a bitmap using the "bitmap", "insert", and "erase" methods
    
    /// initialize.
    ///  Initializes bitmap so that the complex may contain a full cubical complex
    /// which full cubes (0,0,...,0) to (sizes[0] - 1, sizes[1] - 1, ... , sizes [ dimension - 1] - 1).
    /// Internally this is stored with \prod_i ( sizes[i]+1 ) * 2^d bits, using a lower left and upper right (actually this is
    /// simulated rather than explicitly done)
    /// buffer for the following technical reasons:
    /// - the lower left buffer is so the coboundary routine doesn't need a special check to see if it is at 
    /// the edge of memory
    /// - the upper right buffer is to store the lower dimensional cells which may boundaries
    void initialize ( const std::vector<uint32_t> & sizes );
    
    /// fullCube
    /// returns a list of "Cells" corresponding to the cube coordinates
    std::vector < CubicalComplex::Cell >
    fullCube ( const std::vector<uint32_t> & cube_coordinates ) const;
    
    /// fullCube
    /// returns a list of "Index"s corresponding to the cube coordinates
    std::vector < std::vector < Index > >
    fullCubeIndexes ( const std::vector<uint32_t> & cube_coordinates ) const;
    
    /// addFullCube.
    /// Adds the full cube (3^d cells) indicated by "cube_coordinates". */
    void addFullCube ( const std::vector<uint32_t> & cube_coordinates );
    
    /// insertRect -- see geometric interface
    
    /// removeFullCube.
    /// Remove the full cube (3^d cells) indicated by "cube_coordinates" 
    void removeFullCube ( const std::vector<uint32_t> & cube_coordinates );
    
    /// finalize. Call one complex is created in order to compute indexing
    /// used by algorithms. 
    void finalize ( void );
    
    /// cubeIndex. Get indexes of highest dimensional cells.
    Index cubeIndex ( const std::vector<uint32_t> & cube_coordinates ) const;
    std::vector<uint32_t> indexToCube ( Index i ) const;
    std::vector<uint32_t> addressToCube ( uint64_t address ) const;
    
    /// low-level interface
    bool bitmap ( const uint64_t address ) const;
    void insert ( const uint64_t address );
    void erase ( const uint64_t address );
    
    /// geometric interface
    Rect & bounds ( void );
    const Rect & bounds ( void ) const;
    
    // cover: returns indexes of inserted cells (Rect)
    template < class InsertIterator > void cover ( InsertIterator & ii, const Rect & p ) const; 
    std::vector < Index > cover ( const Rect & p ) const;  
    
    // cover: returns indexes of inserted cells (Prism)
    template < class InsertIterator > void cover ( InsertIterator & ii, const Prism & p ) const; 
    std::vector < Index > cover ( const Prism & p ) const;  

    // cover: returns indexes of inserted cells (Vector)
    template < class InsertIterator, class T > void cover ( InsertIterator & ii, 
                                                           const std::vector<T> & V ) const;
    template < class T >
    std::vector < Index > cover ( const std::vector<T> & V ) const;  
    
    // insertRect -- adds full cubes to cover p
    void insertRect ( const Rect & p ); 
    
    // geometry and geometryOfCube: give floating point bounds of geometric incarnation of cell
    Rect geometry ( Index i, int dim ) const;
    Rect geometryOfCube ( const std::vector < uint32_t > & cube ) const;
    
  private:
    /* Cubical Complex */
    std::vector<uint32_t> dimension_sizes_; 
    std::vector<uint64_t> jump_values_; 
    boost::unordered_set < uint64_t > cells_;
    uint64_t mask_;
    Rect bounds_;
    //std::vector<bool> bitmap_;
    template < class InsertIterator >
    void coverHelper ( InsertIterator & ii,
                      uint64_t /* DANGER */ partial,
                      const std::vector < uint32_t > & low, 
                      const std::vector < uint32_t > & high,
                      int d ) const;
    void insertRectHelper (std::vector < uint32_t > & selected,
                           const std::vector < uint32_t > & low, 
                           const std::vector < uint32_t > & high,
                           int d );
  };
  
  /*******************************
   *        DEFINITIONS          *
   *******************************/
  
  // EXPLANATION OF CUBICAL BITMAP SCHEME
  // There is a bitmap_[i], where 0<= i < N.
  // Each i has a bitmap representation. The last "dimension()" bits of this representation
  // determines which type of cell it is; if the kth least significant bit is set, then
  // the cell has extent in the kth dimension. Otherwise, it is not. The remaining bits,
  // i >> dimension(), indicate the position of the cube. See the member variables
  // dimension_sizes_ and jump_values_ to see how the position bits work.
  //
  // BOUNDARY ALGORITHM:
  // Consider the bit-representation of piece_number. For three-dimensional space, 
  // it is three bits 101, perhaps. This corresponds to a square piece. Its boundaries 
  // come in 2 varieties: with piece_number 001 and with piece_number 100. In general 
  // the set of piece numbers can be arrived at by considering the 'single bit demotions'. 
  // 1101 has single bit demotions of 1100, 1001, and 0101, for example.
  // Corresponding to each single bit demotion are two boundary pieces. One has the same
  // full_cube_number as the original and gets a sign of -1. The other has a full_cube_number 
  // that is offset by the "jump_values" corresponding to the bit position.
  // Of course we have to check these 'alleged' boundary pieces to see if they actually 
  // exist first!
  //
  // COBOUNDARY ALGORITHM:
  // Consider the bit-representation of piece_number. For three-dimensional space, 
  // it is three bits 001, perhaps. This corresponds to an edge. Its boundaries come 
  // in 4 varieties: with piece_number 101 and with piece_number 011. In general the 
  // set of piece numbers can be arrived at by considering the 'single bit promotions'. 
  // 0101 has single bit promotions of 0111 and 1101, for example.
  // Corresponding to each single bit promotion are two coboundary pieces. One has the
  // same full_cube_number as the original and gets a sign of -1. The other has a 
  // full_cube_number that is offset by NEGATIVE of the "jump_values" corresponding to 
  // the bit position. Of course we have to check these 'alleged' coboundary pieces to
  // see if they actually exist first!
  
  inline void CubicalComplex::boundary ( Chain * output, const Index input, int dim ) const {
    // std::cout << "CUBICALBOUNDARY\n";
    // Boundary of a 0-dimensional object is trivial 
    int bd_dim = dim - 1;
    output -> dimension () = bd_dim;
    if ( dim == 0 ) return; 
    uint64_t work_bit = 1;
    Ring positive = Ring (1); 
    Ring negative = - positive;
    bool sign = false;
    uint64_t address = indexToCell ( input , dim );
    for ( int d = 0; d < dimension (); work_bit <<= 1, ++ d ) {
      /* Can we demote this bit? If not, "continue". */
      if ( not ( address & work_bit) ) continue;
      sign = not sign;
      /* Alter address to refer to a boundary in the current full cube */
      address = address ^ work_bit;
      /* Insert the piece in the current full cube */
      if ( bitmap ( address ) )
        (*output) += Term ( cellToIndex ( address, bd_dim ), sign ? positive : negative );
      /* Insert the piece in the appropriate neighboring full cube */
      uint64_t offset_address = address + ( jump_values_ [ d ] << dimension () );
      if ( bitmap ( offset_address ) )
        (*output) += Term ( cellToIndex ( offset_address, bd_dim ), sign ? negative : positive );
      /* Recover original address */
      address = address ^ work_bit; 
    } /* for */
  } /* CubicalComplex::boundary */
  
  inline void CubicalComplex::coboundary ( Chain * output, const Index input, int dim ) const {
    int cbd_dim = dim + 1;
    output -> dimension () = cbd_dim;
    if ( dim == dimension () ) return;
    uint64_t work_bit = 1;
    Ring positive = Ring ( 1 );
    Ring negative = - positive;
    bool sign = true;
    uint64_t address = indexToCell ( input , dim );
    
    //std::cout << " address = " << address << "\n";
    //std::cout << " shiftaddress = " << (address >> dimension () ) << "\n";
    
    for ( int d = 0; d < dimension (); work_bit <<= 1, ++ d ) {
      /* Can we promote this bit? If not, "continue". */
      if ( address & work_bit ) { 
        sign = not sign;
        continue;
      }
      address = address ^ work_bit;
      if ( bitmap ( address ) )
        (*output) += Term ( cellToIndex ( address, cbd_dim ), sign ? positive : negative );
      uint64_t offset_address = address - ( jump_values_ [ d ] << dimension () );
      if ( bitmap ( offset_address ) )
        (*output) += Term ( cellToIndex ( offset_address, cbd_dim ), sign ? negative : positive );
      address = address ^ work_bit; 
    } /* for */
  } /* CubicalComplex::coboundary */
  
  inline void CubicalComplex::loadFromFile ( const char * FileName ) {
    char text_buffer[512];
    char *ptr;
    std::ifstream input_file ( FileName ); 
    if ( not input_file . good () ) {
      std::cout << "CubicalComplex::loadFromFile. Fatal Error. " 
      << FileName << " not found.\n";
      exit ( 1 ); } /* if */
    int index = 0; 
    
    /* Find first line of text with a "(" */
    while ( not input_file . eof () ) {
      input_file . getline ( text_buffer, 512, '\n' );
      index = 0;
      while ( text_buffer[index] != '(' && text_buffer[index] != 0 ) index++; 
      if ( text_buffer [ index ] == 0 ) continue; 
      else break; 
    } /* while */
    /* Obtain dimension from first line of text */
    /*	(Count number of commas and add one -- that is dimension.) */
    dimension () = 1;
    index = 0;
    while ( text_buffer [ index ] != 0 ) {
      if( text_buffer [ index ] == ',' ) dimension () = dimension () + 1;
      index ++ ; 
    } /* while */
    
    std::vector<int> min_entry (dimension (), 0);
    std::vector<int> max_entry (dimension (), 0);
    std::vector<uint32_t> user_dimension_sizes (dimension (), 0);
    
    /* Return to beginning of file */
    input_file . clear ();
    input_file . seekg ( 0, std::ios::beg );
    
    /* Now scan through every line of text and read in full cubes */ {
      std::vector<int> cube_coordinates( dimension (), 0);
      while ( not input_file . eof () ) {
        input_file . getline ( text_buffer, 512, '\n' );
        index = 0; 
        while ( text_buffer[index] != '(' && text_buffer[index] != 0 ) index++; 
        if ( text_buffer [ index ] == 0 ) continue;
        ++ index; 
        /* Read the coordinates of the cube from the line */
        for ( int d = 0; d < dimension (); ++ d ) {
          ptr = text_buffer + index;
          while ( text_buffer[index] != ',' && text_buffer[index] != ')') index++;
          text_buffer[index] = 0; 
          index++;
          cube_coordinates[d] = atoi(ptr); 
        } /* for */
        /* Update min_entry and max_entry */
        for ( int d = 0; d < dimension (); ++ d ) {
          if ( cube_coordinates [ d ] < min_entry [ d ] )
            min_entry [ d ] = cube_coordinates [ d ];
          if ( cube_coordinates [ d ] > max_entry [ d ] )
            max_entry [ d ] = cube_coordinates [ d ];     
        } /* for */
      } /* while */
    } /* scope */
    
    for ( int d = 0; d < dimension (); ++ d ) {
      user_dimension_sizes [ d ] = max_entry [ d ] - min_entry [ d ] + 1;
    } /* for */
    
    /* Initialize */
    initialize ( user_dimension_sizes );
    
    /* Return to beginning of file */
    input_file . clear ();
    input_file . seekg ( 0, std::ios::beg );
    
    /* Now scan through every line of text and read in full cubes */
    std::vector<uint32_t> cube_coordinates( dimension (), 0);
    while ( not input_file . eof () ) {
      input_file . getline ( text_buffer, 512, '\n' );
      index = 0; 
      while ( text_buffer[index] != '(' && text_buffer[index] != 0 ) index++; 
      if ( text_buffer [ index ] == 0 ) continue;
      ++ index; 
      /* Read the coordinates of the cube from the line */
      for ( int d = 0; d < dimension (); ++ d ) {
        ptr = text_buffer + index;
        while ( text_buffer[index] != ',' && text_buffer[index] != ')') index++;
        text_buffer[index] = 0; 
        ++ index;
        cube_coordinates[d] = atoi(ptr) - min_entry[d]; 
      } /* for */
      /* Now Add the Cube to the complex. */
      addFullCube ( cube_coordinates ); 
    } /* while */
    finalize ();
    /* We are done reading. Close the file. */
    input_file . close ();
  } /* CubicalComplex::loadFromFile */
  
  inline void CubicalComplex::initialize ( const std::vector<uint32_t> & user_dimension_sizes ) {
    dimension () = user_dimension_sizes . size ();
    dimension_sizes_ . resize ( dimension (), 0 );
    jump_values_ . resize ( dimension (), 0 );
    uint64_t number_of_full_cubes = 1;
    for ( int d = 0; d < dimension (); ++ d ) { 
      dimension_sizes_ [ d ] = ( 2 + user_dimension_sizes [ d ] );
      jump_values_ [ d ] = number_of_full_cubes;
      number_of_full_cubes *= dimension_sizes_ [ d ]; 
      /* +2 for lower-left and upper-right buffers */
    } /* for */
    mask_ = ( ( (uint64_t) 1 ) << dimension () ) - (uint64_t) 1;
    
    //bitmap_ . clear ();
    //bitmap_ . resize ( number_of_full_cubes << dimension (), false );
    
  } /* CubicalComplex::initialize */
  
  inline std::vector < CubicalComplex::Cell >
  CubicalComplex::fullCube ( const std::vector<uint32_t> & cube_coordinates ) const {
    std::vector < CubicalComplex::Cell > result;
    std::vector<uint32_t> neighbor_coordinates( dimension (), 0);
    /* Calculate the number of the read cube */
    uint64_t full_cube_number = 0;
    for ( int d = 0; d < dimension (); ++ d ) 
      full_cube_number += jump_values_ [d] * ( (uint64_t) cube_coordinates [d] + 1 ); // lower-left buffer +1
    /* Insert the pieces from all neighboring cubes */
    for ( int neighbor_index = 0; neighbor_index < ( 1 << dimension () ); ++ neighbor_index ) {
      neighbor_coordinates = cube_coordinates;
      int temp = neighbor_index;
      uint64_t offset = 0;
      bool flag = false;
      for ( int d = 0;  d < dimension () ; ++ d ) {
        if ( temp % 2 == 0 ) {
          neighbor_coordinates [ d ] ++;
          offset += jump_values_ [ d ]; 
        } /* if */
        temp = temp >> 1; 
        if ( neighbor_coordinates [ d ] == dimension_sizes_ [ d ] ) flag = true; 
      } /* for */
      if ( flag ) continue;
      /* this is inefficient */
      for( int piece_index = 0; piece_index < 1 << dimension (); piece_index ++ ) 
        if ( ( piece_index & ~neighbor_index ) == 0)  /* clever bitwise test */  {
          /* Figure out the dimension of the cell */
          uint32_t cell_dimension = 0;
          for ( int bit_index = 0; bit_index < dimension (); ++ bit_index ) {
            if ( piece_index & ( 1 << bit_index ) ) ++ cell_dimension; 
          } /* for */
          /* insert the cell */
          //std::cout << " CC addFullCube: bitmap_ [ " << ( ( full_cube_number + offset ) << dimension () ) + piece_index << " ] = true\n";
          result . push_back ( ( ( full_cube_number + offset ) << dimension () ) + piece_index );
        } /* if */
    } /* for */
    //std::cout << " . \n";
    return result; 
  }
  
  /// fullCube
  /// returns a list of "Indexes" corresponding to the cube coordinates
  inline std::vector < std::vector < Index > >
  CubicalComplex::fullCubeIndexes ( const std::vector<uint32_t> & cube_coordinates ) const {
    std::vector < CubicalComplex::Cell > cubecells = fullCube ( cube_coordinates );
    std::vector < std::vector < Index > > result ( dimension () + 1 );
    BOOST_FOREACH ( Cell address, cubecells ) {
      int dim = __builtin_popcount ( mask_ & address );
      if ( bitmap ( address ) == false ) {
        std::cout << "The address " << address << " does not hold a cell.\n";
        exit ( 1 );
      }
      result [ dim ] . push_back ( cellToIndex ( address, dim ) );
    }
    return result;
  }
  
  
  inline void CubicalComplex::addFullCube ( const std::vector<uint32_t> & cube_coordinates ) {
    BOOST_FOREACH ( const Cell cell, fullCube ( cube_coordinates ) ) 
    insert ( cell );
  } /* CubicalComplex::addFullCube */
  
  inline void CubicalComplex::removeFullCube ( const std::vector<uint32_t> & cube_coordinates ) {
    BOOST_FOREACH ( const Cell cell, fullCube ( cube_coordinates ) ) 
    erase ( cell );
  } /* CubicalComplex::removeFullCube */
  
  inline void CubicalComplex::finalize ( void ) {
    //std::cout << "CALLING CC FINALIZE\n";
    //startInserting ();
    BOOST_FOREACH ( uint64_t address, cells_ ) {
      int d = __builtin_popcount ( ( uint32_t) ( address & mask_ ) );
      insertCell ( address, d );
    }
    /*
     for ( uint64_t i = 0; i < bitmap_ . size (); ++ i ) {
     if ( bitmap_ [ i ] ) {
     int d = __builtin_popcount ( ( uint32_t) ( i & mask ) );
     //std::cout << " CC: insert (" << i << ", " << d << ")\n";
     insertCell ( i, d );
     }
     }
     */
    //finishedInserting ();
    //std::cout << "EXIT CC FINALIZE\n";
  } /* CubicalComplex::finalize */
  
  inline bool CubicalComplex::bitmap ( const uint64_t address ) const {
    if ( cells_ . count ( address ) != 0 ) return true;
    return false;
  }
  inline void CubicalComplex::insert ( const uint64_t address ) {
    cells_ . insert ( address );
  }
  
  inline void CubicalComplex::erase ( const uint64_t address ) {
    cells_ . erase ( address );
  }
  
  inline Index CubicalComplex::cubeIndex ( const std::vector<uint32_t> & cube_coordinates ) const {
    uint64_t full_cube_number = 0;
    for ( int d = 0; d < dimension (); ++ d ) 
      full_cube_number += jump_values_ [d] * ( (uint64_t) cube_coordinates [d] + 1 ); // lower-left buffer +1
    return cellToIndex ( (full_cube_number << dimension ()) + mask_, dimension () );
  }
  
  inline std::vector<uint32_t> CubicalComplex::indexToCube ( Index i ) const {
    uint64_t address = indexToCell ( i, dimension () );
    return addressToCube ( address );
  }
  
  inline std::vector<uint32_t> CubicalComplex::addressToCube ( uint64_t address ) const {
    int D = dimension ();
    std::vector<uint32_t> result ( D );
    address >>= D;
    Index temp = address; //debug
    for ( int d = 0; d < D; ++ d ) {
      int pos = address % dimension_sizes_ [ d ];
      address /= dimension_sizes_ [ d ];
      // DEBUG
      if ( pos == 0 ) {
        std::cout << temp << "\n";
        std::cout << dimension_sizes_ [ 0 ] << "\n";
      }
      result [ d ] = pos - 1; // subtract 1 to ignore wrap layer
    }
    return result;
  }
  
  inline Rect & CubicalComplex::bounds ( void ) {
    return bounds_;
  }
  
  inline const Rect & CubicalComplex::bounds ( void ) const {
    return bounds_;
  }
  
  template < class InsertIterator >
  void
  CubicalComplex::coverHelper ( InsertIterator & ii,
                               uint64_t /* DANGER */ partial,
                               const std::vector < uint32_t > & low, 
                               const std::vector < uint32_t > & high,
                               int d ) const {
    //std::cout << d << ": coverHelper " << partial << "\n";
    if ( d == 0 ) {
      uint64_t address = partial << dimension ();
      address |= mask_;
      // DEBUG
      /*
       std::vector < uint32_t > cube = addressToCube ( address );
       for ( int k = 0; k < (int) cube . size (); ++ k ) {
       std::cout << cube [ k ] << " / " << dimension_sizes_ [ k ] << " ; ";
       }
       std::cout << "\n";
       Rect geo = geometryOfCube ( cube );
       std::cout << geo << "\n";
       */
      
      if ( not bitmap ( address ) ) return;
      * ii ++ = cellToIndex ( address, dimension () );
      return;
    }
    -- d;
    //std::cout << "    For d = " << d << ", adding low[d]*jv[d] = " << low[d] * jump_values_ [ d ] << ", where low[d]=" << low[d] << "\n";
    partial += ( 1 + low[d] ) * jump_values_ [ d ];
    for ( uint32_t i = low [ d ]; i <= high [ d ]; ++ i ) {
      coverHelper ( ii, partial, low, high, d );
      //std::cout << "    Hopping over at dim = " << d << " by " << jump_values_ [ d ] << "\n";
      partial += jump_values_ [ d ];
    }
  }
  
  template < class InsertIterator >
  void CubicalComplex::cover ( InsertIterator & ii, const Rect & p ) const {
    //std::cout << ">>> CUBICAL COMPLEX: COVER ----: " << p << "\n";
    // Convert prism to scale of dimension sizes
    // Be careful about the padding.
    int D = dimension ();
    std::vector < uint32_t > low ( D );
    std::vector < uint32_t > high ( D );
    for ( int d = 0; d < D; ++ d ) {
      double lowbound = bounds () . lower_bounds [ d ];
      double highbound = bounds () . upper_bounds [ d ];
      double scale = ( (double) (dimension_sizes_ [ d ] - 2) ) / (highbound - lowbound);
      double lower = p . lower_bounds [ d ] - lowbound;
      if ( lower < 0 ) lower = 0;
      if ( lower + lowbound > highbound ) lower = highbound;
      double upper = p . upper_bounds [ d ] - lowbound;
      if ( upper < 0 ) upper = 0;
      if ( upper + lowbound > highbound ) upper = highbound;
      low [ d ] = (uint32_t) ( scale * lower );
      high [ d ] = (uint32_t) ( scale * upper );
      //std::cout << " low[" << d << "] = " << low [ d ] << ", high["<<d<<"] = " << high [ d ] << "\n";
      //std::cout << "low grid line = " << lowbound + (low [ d ] - 1 ) / scale << "\n";
      //std::cout << "high grid line = " << lowbound + (high [ d ] ) / scale << "\n";
    }
    coverHelper ( ii, 0, low, high, D );
  }
  
  inline std::vector < Index > 
  CubicalComplex::cover ( const Rect & p ) const {
    std::vector < Index > result;
    std::insert_iterator < std::vector < Index > > ii ( result, result . end () );
    cover ( ii, p );
    return result;
  }
  
  template < class InsertIterator > 
  void CubicalComplex::cover ( InsertIterator & ii, 
                               const Prism & p ) const {
    // Conversion from box to rect helper variables
    std::vector<Real> stepsize ( dimension () );
    for ( int d = 0; d < dimension (); ++ d ) {
      stepsize [ d ] = bounds () . upper_bounds [ d ] -
      bounds () . lower_bounds [ d ];
      stepsize [ d ] /= dimension_sizes_ [ d ] - 2;
    }
    // number of d-cells
    uint32_t N = size ( dimension () );
    // Prepare main loop
    typedef std::vector < std::pair<uint32_t, uint32_t > > Box;
    std::stack < Box > workstack;
    Box entire ( dimension (), std::pair<uint32_t, uint32_t> (0,0) );
    for ( int d = 0; d < dimension (); ++ d ) {
      entire [ d ] . second = dimension_sizes_ [ d ] - 2;
    }
    workstack . push ( entire );
    // Work loop. Hierarchically investigates.
    while ( not workstack . empty () ) {
      // Pull box from stack
      Box b = workstack . top ();
      workstack . pop ();
      // Check for intersection with prism p
      //   step 1) create Rect r corresponding to Box b
      Rect r ( dimension () );
      for ( int d = 0; d < dimension (); ++ d ) {
        r . lower_bounds [ d ] = bounds () . lower_bounds [ d ] +
        b [ d ] . first * stepsize [ d ];
        r . upper_bounds [ d ] = bounds () . lower_bounds [ d ] +
        b [ d ] . second * stepsize [ d ];
      }
      //   step 2) check for intersection
      //     if there is none, we continue and do not subdivide
      if ( not p . intersects ( r ) ) { 
        //std::cout << " Rectangle " << r << " does not intersect prism " << p << "\n";
        continue;
      }
      // Find best split
      int best_split_dim;
      int biggest_width = 0;
      for ( int d = 0; d < dimension (); ++ d ) {
        int width = b [ d ] . second - b [ d ] . first;
        if ( width > biggest_width ) {
          best_split_dim = d;
          biggest_width = width;
        }
      }
      // If the best split is trivial, then b corresponds to
      // a cube. Toss it into result.
      if ( biggest_width == 1 ) {
        std::vector<uint32_t> cube_coordinates ( dimension () );
        for ( int d = 0; d < dimension (); ++ d ) {
          cube_coordinates [ d ] = b [ d ] . first;
        }
        // No cube will be stored more than once using 
        // the insert iterator.
        Index i = cubeIndex ( cube_coordinates );
        // cubeIndex returns an error code "N"
        // where N = size ( dimension () ) if the cube isn't
        // in the complex
        if ( i != N )  { 
          * ii ++ = i;
        } //else {
          //std::cout << " cubeIndex not valid\n";
        //}
        continue;
      }
      // A split it possible. We produce the two children
      // and push them to the stack.
      // we copy to bL, and rewrite bR over b.
      Box bL = b;
      // [a_k, a_k + floor ( (b_k - a_k) / 2 ) )
      // [ a_k + floor ( (b_k - a_k)/2 ), b_k ),
      // where k = best_split_dim
      bL [ best_split_dim ] . second = 
        b [ best_split_dim ] . first + biggest_width / 2;
      b [ best_split_dim ] . first = 
        bL [ best_split_dim ] . second;
      workstack . push ( bL );
      workstack . push ( b );
    }
  }
  
  inline std::vector < Index > 
  CubicalComplex::cover ( const Prism & p ) const {
    std::vector < Index > result;
    std::insert_iterator < std::vector < Index > > ii ( result, result . end () );
    cover ( ii, p );
    return result;
  }
  
  
  template < class InsertIterator, class T > 
  void CubicalComplex::cover ( InsertIterator & ii, 
                              const std::vector < T > & V ) const {
    BOOST_FOREACH ( const T & t, V ) {
      cover ( ii, t );
    }
  }
  
  template < class T >
  std::vector < Index > 
  CubicalComplex::cover ( const std::vector < T > & V ) const {
    std::vector < Index > result;
    std::insert_iterator < std::vector < Index > > ii ( result, result . end () );
    cover ( ii, V );
    return result;
  }
  
  inline void
  CubicalComplex::insertRectHelper 
  (std::vector < uint32_t > & selected,
   const std::vector < uint32_t > & low, 
   const std::vector < uint32_t > & high,
   int d ) {
    if ( d == dimension () ) {
      addFullCube ( selected ); 
    } else {
      for ( uint32_t x = low [ d ]; x <= high [ d ]; ++ x ) {
        selected [ d ] = x;
        insertRectHelper ( selected, low, high, d + 1 ); 
      }
    }
    
  }
  
  inline void CubicalComplex::insertRect ( const Rect & p ) {
    //std::cout << ">>> CUBICAL COMPLEX: COVER ----: " << p << "\n";
    // Convert prism to scale of dimension sizes
    // Be careful about the padding.
    int D = dimension ();
    std::vector < uint32_t > low ( D );
    std::vector < uint32_t > high ( D );
    for ( int d = 0; d < D; ++ d ) {
      double lowbound = bounds () . lower_bounds [ d ];
      double highbound = bounds () . upper_bounds [ d ];
      double scale = ( (double) (dimension_sizes_ [ d ] - 2) ) / (highbound - lowbound);
      double lower = p . lower_bounds [ d ] - lowbound;
      if ( lower < 0 ) lower = 0;
      if ( lower + lowbound > highbound ) lower = highbound;
      double upper = p . upper_bounds [ d ] - lowbound;
      if ( upper < 0 ) upper = 0;
      if ( upper + lowbound > highbound ) upper = highbound;
      low [ d ] = (uint32_t) ( scale * lower );
      high [ d ] = (uint32_t) ( scale * upper );
    }
    std::vector < uint32_t > selected = low;
    insertRectHelper ( selected, low, high, 0 );
  }
  
  inline Rect CubicalComplex::geometryOfCube ( const std::vector < uint32_t > & cube ) const {
    int D = dimension ();
    Rect result ( D );
    for ( int d = 0; d < D; ++ d ) {
      double lowbound = bounds () . lower_bounds [ d ];
      double highbound = bounds () . upper_bounds [ d ];
      double scale = (highbound - lowbound) / ( (double) dimension_sizes_ [ d ] - 2 );
      result . lower_bounds [ d ] = lowbound + scale * cube [ d ];
      result . upper_bounds [ d ]  = result . lower_bounds [ d ] + scale;
    }
    return result;
  }
  
  inline Rect CubicalComplex::geometry ( Index i, int dim ) const {
    //std::cout << "CubicalComplex::geometry ( " << i << ", " << dim << ")\n";
    int D = dimension ();
    if ( dim < D ) {
      uint64_t address = indexToCell ( i, dim );
      Rect result = geometryOfCube ( addressToCube ( address ) );
      uint64_t bit = 1;
      for ( int d = 0; d < D; ++ d ) {
        if ( not ( address & bit )  ) 
          result . upper_bounds [ d ] = result . lower_bounds [ d ];
        bit <<= 1;
      }
      
      return result;
    }
    std::vector < uint32_t > cube = indexToCube ( i );
    
    // DEBUG
    Rect result = geometryOfCube ( cube );
    for ( int d = 0; d < D; ++ d ) {
      if ( result . upper_bounds [ d ] > bounds () . upper_bounds [ d ] ) {
        std::cout << "CubicalComplex::geometry ( " << i << ", " << dim << ")\n";
        std::cout << result << "\n";
        std::cout << size ( dim ) << "\n";
        for ( int k = 0; k < D; ++ k ) std::cout << cube [ k ] << ", ";
        std::cout << "\n";
        abort ();
      }
      
    }
    return geometryOfCube ( cube );
  }
  
} // namespace chomp

#endif

