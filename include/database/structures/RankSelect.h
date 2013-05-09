#ifndef RANKSELECT_H
#define	RANKSELECT_H

#include "boost/serialization/serialization.hpp"
#include "boost/serialization/vector.hpp"

#include <inttypes.h>
#include "sdsl/rank_support_v5.hpp"
#include "sdsl/select_support_mcl.hpp"
#include "sdsl/util.hpp"

// Class that implements Rank/Select operations

/**
 * @file
 * @author Arnaud Goullet
 * @version 1.0
 * 
 * @description
 * Implementation of Rank/Select operations on a binary sequence.
 */

class RankSelect {
public:

  typedef uint64_t size_type;

  /**
   * Constructor with a single element equal to 1 
   */
  RankSelect ( ) {
    bits_ . resize ( 1 );
    bits_[0] = 1;
    rank_ . init ( &bits_ );
    select_ . init ( &bits_ );
  }

  /**
   * Constructor from a given bit sequence
   * @param bits
   */
  RankSelect ( const std::vector < bool > & bits ) {
    // should be optimized
    bits_ . resize ( bits . size ( ) );
    for ( unsigned int i = 0; i < bits.size ( ); ++ i ) bits_[i] = bits[i];

    //std::copy(bits.begin(), bits.end(), bits_.begin());

    rank_ . init ( &bits_ );
    select_ . init ( &bits_ );
  }

  RankSelect& operator= ( const RankSelect& other ) {
    bits_ = other . bits_;
    rank_ . init ( &bits_ );
    select_ . init ( &bits_ );
    return *this;
  }
  /**
   * 
   * @param i
   * @return the rank of the bit sequence at a given position. 
   * Here the rank is defined as the number of 1's on [0,i]  
   */
  // by default rank_ . rank ( i ) will return # 1's on [0,i-1]
  // so we modify it to return # 1's on [0,i]

  size_type rank ( size_type i ) const {
    return rank_ . rank ( i + 1 );
  }

  /**
   * 
   * @param i
   * @return the position of the i-th 1 in the bit sequence
   */
  size_type select ( size_type i ) const {
    return select_ . select ( i );
  }

  /**
   * 
   * @param position
   * @return the value of the bit at a given position
   */
  bool bits ( size_type position ) const {
    return bits_ [ position ];
  }
   
  /**
   * 
   * @return the complete bit sequence over which the rank/select operates on.  
   */
  std::vector < bool > bits_sequence ( void ) const {
    std::vector < bool > output;
    for ( unsigned int i = 0; i < bits_.size ( ); ++ i ) output . push_back ( bits_[i] );
    return output;
  }

  /**
   * Display the bit sequence.
   */
  void info ( void ) const {
    std::cout << "bits : ";
    for ( unsigned int i = 0; i < bits_.size ( ); ++ i ) std::cout << bits_[i] << " ";
    std::cout << "\n";
  }

  /**
   * Display the memory usage.
   */
  void memory_usage ( void ) const {
    double bits_mem_size ( sdsl::util::get_size_in_bytes ( bits_ ) );
    double rank_mem_size ( sdsl::util::get_size_in_bytes ( rank_ ) );
    double select_mem_size ( sdsl::util::get_size_in_bytes ( select_ ) );
    std::cout << "\nMemory usage for Rank/Select support class: ";
    std::cout << 8 * ( bits_mem_size + rank_mem_size + select_mem_size ) / bits_.size ( ) << " bits per node\n";
  }

  /**
   * Return the memory usage in bytes.
   */
  uint64_t memory ( void ) const {
    return sdsl::util::get_size_in_bytes ( bits_ ) +
    sdsl::util::get_size_in_bytes ( rank_ ) +
    sdsl::util::get_size_in_bytes ( select_ );
  }

private:
  sdsl::bit_vector bits_;
  sdsl::rank_support_v5 < > rank_;
  sdsl::select_support_mcl < > select_;
  
  
  friend class boost::serialization::access;
  template<class Archive>
  void save ( Archive & ar , const unsigned int version ) const {
    std::vector < bool > bit_sequence;
    for ( size_type i = 0; i < bits_.size(); ++ i ) bit_sequence . push_back ( bits_ [ i ] );
    ar & bit_sequence;   
  }

  template<class Archive>
  void load ( Archive & ar , const unsigned int version ) {
    vector < bool > bit_sequence;
    ar & bit_sequence;
    bits_ . resize ( bit_sequence . size() );
    for ( size_type i = 0; i < bit_sequence.size(); ++ i ) bits_ [ i ] =  bit_sequence [ i ];
    rank_ . init ( &bits_ );
    select_ . init ( &bits_ );
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER ( );
  
};



#endif	/* RANKSELECT_H */

