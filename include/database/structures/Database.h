/* Database */

#ifndef CMDP_DATABASE
#define CMDP_DATABASE

#include <cstddef>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "boost/serialization/shared_ptr.hpp"
#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

#include "database/structures/Grid.h"
#include "database/structures/SuccinctGrid.h"
#include "database/structures/PointerGrid.h"
#include "database/structures/MorseGraph.h"

#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/*****************/
/*   DATA        */
/*****************/

// Directed Acyclic Graphs
struct DAG_Data {
  int num_vertices;
  std::vector < std::pair < int, int > > partial_order;
  // Constructor
  DAG_Data ( void ) {};
  DAG_Data ( const MorseGraph & cmg ) : num_vertices ( cmg . NumVertices () ) {
    for ( MorseGraph::EdgeIteratorPair startstop = cmg . Edges ();
     startstop . first != startstop . second; ++ startstop . first ) {
      partial_order . push_back ( std::make_pair ( startstop . first -> first,
        startstop . first -> second ) );
  }
}
bool operator == ( const DAG_Data & rhs ) const {
  if ( num_vertices != rhs . num_vertices ) return false;
  if ( partial_order . size () != rhs . partial_order . size () ) return false;
  for ( uint64_t i = 0; i < partial_order . size (); ++ i ) {
    if ( partial_order [ i ] != rhs . partial_order [ i ] ) return false;
  }
  return true;
}
friend class boost::serialization::access;
  template<class Archive>
void serialize(Archive& ar, const unsigned int version) {
  ar & boost::serialization::make_nvp("N", num_vertices);
  ar & boost::serialization::make_nvp("PO", partial_order);
}
};

inline std::size_t hash_value(DAG_Data const& dag) {
  std::size_t seed = 0;
  boost::hash_combine(seed, dag . num_vertices);
  typedef std::pair<int,int> intpair;
  BOOST_FOREACH ( const intpair & ip, dag . partial_order ) {
    boost::hash_combine(seed, ip . first );
    boost::hash_combine(seed, ip . second );
  }
  return seed;
}


// Bipartite Graphs
struct BG_Data {
  std::vector < std::pair < int, int > > edges;
  bool operator == ( const BG_Data & rhs ) const {
    if ( edges . size () != rhs . edges . size () ) return false;
    for ( uint64_t i = 0; i < edges . size (); ++ i ) {
      if ( edges [ i ] != rhs . edges [ i ] ) return false;
    }
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("E", edges);
  }
};

inline std::size_t hash_value(BG_Data const& bg) {
  std::size_t seed = 0;
  typedef std::pair<int,int> intpair;
  BOOST_FOREACH ( const intpair & ip, bg . edges ) {
    boost::hash_combine(seed, ip . first );
    boost::hash_combine(seed, ip . second );
  }
  return seed;
}

// Convex Sets
struct CS_Data {
  std::vector < int > vertices;
  bool operator == ( const CS_Data & rhs ) const {
    if ( vertices . size () != rhs . vertices . size () ) return false;
    for ( uint64_t i = 0; i < vertices . size (); ++ i ) {
      if ( vertices [ i ] != rhs . vertices [ i ] ) return false;
    }
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("V", vertices);
  }
};

inline std::size_t hash_value(CS_Data const& cs) {
  std::size_t seed = 0;
  BOOST_FOREACH ( int i, cs . vertices ) {
    boost::hash_combine(seed, i );
  }
  return seed;
}

/*****************/
/*   RECORDS     */
/*****************/

// MORSE RECORD

class MorseRecord {
public:
  uint64_t grid_element;
  uint64_t dag_index;
  
  // Constructor
  MorseRecord ( void ) {};
  MorseRecord ( uint64_t grid_element, uint64_t dag_index )
  : grid_element ( grid_element ), dag_index ( dag_index ) {};
  bool operator < ( const MorseRecord & rhs ) const {
    return grid_element < rhs . grid_element;
  }
  friend class boost::serialization::access;
    template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("GE", grid_element);
    ar & boost::serialization::make_nvp("DAG", dag_index);
  }
};

// CLUTCHING RECORD

class ClutchingRecord {
public:
  uint64_t grid_element_1;
  uint64_t grid_element_2;
  uint64_t bg_index;
  
  ClutchingRecord ( void ) {};
  ClutchingRecord ( uint64_t grid_element_1, uint64_t grid_element_2, uint64_t bg_index )
  : grid_element_1 ( grid_element_1 ), grid_element_2 ( grid_element_2 ), bg_index ( bg_index ) {};
  
  bool operator < ( const ClutchingRecord & rhs ) const {
    return std::make_pair ( grid_element_1, grid_element_2 ) <
    std::make_pair ( rhs . grid_element_1, rhs . grid_element_2 );
  }
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("GE1",grid_element_1);
    ar & boost::serialization::make_nvp("GE2",grid_element_2);
    ar & boost::serialization::make_nvp("BG",bg_index);
  }
};

// MORSE GRAPH CONTINUATION CLASS PIECE RECORD

struct MGCCP_Record {
  std::vector<Grid::GridElement> grid_elements;
  uint64_t dag_index;
 
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("GES", grid_elements);
    ar & boost::serialization::make_nvp("DAG", dag_index);

  }
};

// MORSE GRAPH CONTINUATION CLASS RECORD

struct MGCC_Record {
  std::vector < uint64_t > mgccp_indices;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("MGCCPS", mgccp_indices);
  }
};


// ISOLATING NEIGHBORHOOD CONTINUATION CLASS PIECE RECORD 

struct INCCP_Record {
  uint64_t cs_index;
  uint64_t mgccp_index;
 
 bool operator == ( const INCCP_Record & rhs ) const {
    if ( cs_index != rhs . cs_index ) return false;
    if ( mgccp_index != rhs . mgccp_index ) return false;
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("CS", cs_index);
    ar & boost::serialization::make_nvp("MGCCP", mgccp_index);
  }
};
inline std::size_t hash_value(INCCP_Record const& cs) {
  std::size_t seed = 0;
  boost::hash_combine(seed, cs . cs_index );
  boost::hash_combine(seed, cs . mgccp_index );
  return seed;
}

// ISOLATING NEIGHBORHOOD CONTINUATION CLASS RECORD
struct INCC_Record {
  std::vector < uint64_t > inccp_indices;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("INCCPS", inccp_indices);
  }
};

/********************/
/*   RAW DATABASE   */
/********************/

class Database {
private:
  friend class boost::serialization::access;


  // RAW RECORDS
  std::vector < MorseRecord > morse_records_;
  std::vector < ClutchingRecord > clutch_records_;



  boost::shared_ptr<Grid> parameter_space_;

  // We use an unordered_map instead of unordered_set in case
  // the hash function isn't the same on two machines, one saving the database
  // and the other loading.
  std::vector < DAG_Data > dag_data_;
  std::vector < BG_Data > bg_data_;
  std::vector < CS_Data > cs_data_;

  boost::unordered_map < DAG_Data, uint64_t > dag_index_;
  boost::unordered_map < BG_Data, uint64_t > bg_index_;
  boost::unordered_map < CS_Data, uint64_t > cs_index_;

  boost::unordered_map<INCCP_Record, uint64_t> inccp_index_;

  std::vector < uint64_t > pb_to_mgccp_;
  std::vector < uint64_t > mgccp_to_mgcc_;
  std::vector < uint64_t > inccp_to_incc_;
  std::vector < boost::unordered_set < uint64_t > > incc_to_mgcc_;
  std::vector < uint64_t > mgcc_sizes_;
  std::vector < uint64_t > incc_sizes_;
  std::vector < boost::unordered_set < uint64_t > > mgcc_nb_;
  std::vector < chomp::ConleyIndex_t > incc_conley_;



  // DERIVED RECORDS
  std::vector < MGCCP_Record > MGCCP_records_;
  std::vector < INCCP_Record > INCCP_records_;
  std::vector < MGCC_Record > MGCC_records_;
  std::vector < INCC_Record > INCC_records_;

public:
  Database ( void ) {}
  void merge ( const Database & other );

  void insert ( boost::shared_ptr<Grid> grid );
  void insert ( const Grid::GridElement & p, const DAG_Data & dag );
  void insert ( const Grid::GridElement & p1, const Grid::GridElement & p2, const BG_Data & bg );

      //void insert ( const ConleyRecord & record );

  void postprocess ( void );

  void save ( const char * filename );   
  void load ( const char * filename );
  
  const Grid & parameter_space ( void ) const { return *parameter_space_;}

  const std::vector < MorseRecord > & morse_records ( void ) const;
  const std::vector < ClutchingRecord > & clutch_records ( void ) const;

  const std::vector < DAG_Data > & dagData ( void ) const 
    { return dag_data_; }
  const std::vector < BG_Data > & bgData ( void ) const 
    { return bg_data_; }
  const std::vector < CS_Data > & csData ( void ) const 
    { return cs_data_; }
  uint64_t dagIndex ( DAG_Data const& item ) const 
    { if ( dag_index_ . count ( item ) == 0 ) return dagData().size(); return dag_index_ . find (item) -> second; }
  uint64_t bgIndex ( BG_Data const& item ) const 
    { if ( bg_index_ . count ( item ) == 0 ) return bgData().size(); return bg_index_ . find (item) -> second; }
  uint64_t csIndex ( CS_Data const& item ) const 
    { if ( cs_index_ . count ( item ) == 0 ) return csData().size(); return cs_index_ . find (item) -> second; }
  const std::vector<boost::unordered_set<uint64_t> > & mgcc_nb ( void ) const 
    { return mgcc_nb_;}

  uint64_t inccpIndex ( INCCP_Record const& item ) const 
    { if ( inccp_index_ . count ( item ) == 0 ) return INCCP_Records().size(); return inccp_index_ . find (item) -> second; }


  const std::vector < uint64_t > & pb_to_mgccp ( void ) const { return pb_to_mgccp_; }
  const std::vector < uint64_t > & mgccp_to_mgcc ( void ) const { return mgccp_to_mgcc_; }
  const std::vector < uint64_t > & inccp_to_incc ( void ) const { return inccp_to_incc_; }
  const std::vector < boost::unordered_set < uint64_t > > & incc_to_mgcc ( void ) const { return incc_to_mgcc_; }
  const std::vector < uint64_t > & mgcc_sizes ( void ) const { return mgcc_sizes_; }
  const std::vector < uint64_t > & incc_sizes ( void ) const { return incc_sizes_; }
  const std::vector < boost::unordered_set < uint64_t > > & mgcc_nb ( void ) const { return mgcc_nb_; }
  const std::vector < chomp::ConleyIndex_t> & incc_conley ( void ) const { return incc_conley_; }


  const std::vector < MGCCP_Record > & MGCCP_Records ( void ) const; 
  const std::vector < INCCP_Record > & INCCP_Records ( void ) const; 
  const std::vector < MGCC_Record > & MGCC_Records ( void ) const;  
  const std::vector < INCC_Record > & INCC_Records ( void ) const;    

  //const std::vector < ConleyRecord > & conley_records ( void ) const;
      template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
        //std::cout << "serialize: check for parameter space\n";
    bool has_space = ( parameter_space_ != NULL );
    ar & boost::serialization::make_nvp("HASPARAMETERSPACE", has_space);
    if ( has_space ) {
      ar & boost::serialization::make_nvp("PARAMETERSPACE", parameter_space_);
    }
    ar & boost::serialization::make_nvp("DAGDATA", dag_data_);
    ar & boost::serialization::make_nvp("BGDATA", bg_data_);
    ar & boost::serialization::make_nvp("CSDATA", cs_data_);
    ar & boost::serialization::make_nvp("DAGINDEX", dag_index_);
    ar & boost::serialization::make_nvp("BGINDEX", bg_index_);
    ar & boost::serialization::make_nvp("CSINDEX", cs_index_);
    ar & boost::serialization::make_nvp("MORSERECORDS", morse_records_);
    ar & boost::serialization::make_nvp("CLUTCHRECORDS", clutch_records_);
    ar & boost::serialization::make_nvp("MGCCP", MGCCP_records_);
    ar & boost::serialization::make_nvp("INCCP", INCCP_records_);
    ar & boost::serialization::make_nvp("MGCC", MGCC_records_);
    ar & boost::serialization::make_nvp("INCC", INCC_records_);
    ar & boost::serialization::make_nvp("INCCPINDEX", inccp_index_);
    ar & boost::serialization::make_nvp("PBTOMGCCP",pb_to_mgccp_);
    ar & boost::serialization::make_nvp("MGCCPTOMGCC", mgccp_to_mgcc_);
    ar & boost::serialization::make_nvp("INCCPTOINCC",inccp_to_incc_);
    ar & boost::serialization::make_nvp("INCCTOMGCC", incc_to_mgcc_);
    ar & boost::serialization::make_nvp("MGCCSIZES",mgcc_sizes_);
    ar & boost::serialization::make_nvp("INCCSIZES", incc_sizes_);
    ar & boost::serialization::make_nvp("MGCCNB", mgcc_nb_);
    ar & boost::serialization::make_nvp("INCCCONLEY",incc_conley_);

  }
};

    /*******************/
    /*   DEFINITIONS   */
    /*******************/

#include <iostream>
#include <fstream>
    //#include <boost/archive/text_oarchive.hpp>
    //#include <boost/archive/text_iarchive.hpp>
#include <cstdio> // remove
#include <boost/config.hpp>
#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{
  using ::remove;
}
#endif
#include <boost/archive/tmpdir.hpp>
//#include <boost/archive/xml_oarchive.hpp>
//#include <boost/archive/xml_iarchive.hpp>
#include "boost/foreach.hpp"


    // database merging
inline void Database::merge ( const Database & other ) {
  std::vector < uint64_t > dag_reindex ( other . dag_data_ . size () );
  std::vector < uint64_t > bg_reindex ( other . bg_data_ . size () );

  for ( uint64_t i = 0; i < other . dag_data_ . size (); ++ i ) {
    const DAG_Data & item = other . dag_data_ [ i ];
    if ( dag_index_ . count ( item ) ) {
      dag_reindex [ i ] = dag_index_ [ item ];
    } else {
      dag_reindex [ i ] = dag_index_ [ item ] = dag_data_ . size ();
      dag_data_ . push_back ( item );
    }
  }
  for ( uint64_t i = 0; i < other . bg_data_ . size (); ++ i ) {
    const BG_Data & item = other . bg_data_ [ i ];
    if ( bg_index_ . count ( item ) ) {
      bg_reindex [ i ] = bg_index_ [ item ];
    } else {
      bg_reindex [ i ] = bg_index_ [ item ] = bg_data_ . size ();
      bg_data_ . push_back ( item );
    }
  }

  BOOST_FOREACH ( MorseRecord record, other . morse_records_ ) {
    record . dag_index = dag_reindex [ record . dag_index ];
    morse_records_ . push_back ( record );
  }
  BOOST_FOREACH ( ClutchingRecord record, other . clutch_records_ ) {
    record . bg_index = bg_reindex [ record . bg_index ];
    clutch_records_ . push_back ( record );
  }
}

    // record insertion
inline void Database::insert ( boost::shared_ptr<Grid> grid ) {
  parameter_space_ = grid;
}

inline void Database::insert ( const Grid::GridElement & p, const DAG_Data & dag ) {
  if ( dag_index_ . count ( dag ) == 0 ) {
    dag_index_ [ dag ] = dag_data_ . size ();
    dag_data_ . push_back ( dag );
  }
  morse_records_ . push_back ( MorseRecord ( p, dag_index_ [ dag ] ) );
}

inline void Database::insert ( const Grid::GridElement & p1, const Grid::GridElement & p2, const BG_Data & bg ) {
   if ( bg_index_ . count ( bg ) == 0 ) {
    bg_index_ [ bg ] = bg_data_ . size ();
    bg_data_ . push_back ( bg );
  }
  clutch_records_ . push_back ( ClutchingRecord ( p1, p2, bg_index_ [ bg ] ) );
}

// POSTPROCESSING ALGORITHM

class ContiguousIntegerUnionFind {
public:
  ContiguousIntegerUnionFind ( void ) {}
  ContiguousIntegerUnionFind ( uint64_t N ) {
    parent . resize ( N );
    rank . resize ( N );
    for ( uint64_t i = 0; i < N; ++ i ) {
      parent [ i ] = i;
      rank [ i ] = 0;
    }
  }
  
  uint64_t MakeSet ( void ) {
    parent . push_back ( parent . size () );
    rank . push_back ( 0 );
    return parent . size () - 1;
  }

  uint64_t Union(uint64_t x, uint64_t y) {
    uint64_t xRoot = Find ( x );
    uint64_t yRoot = Find ( y );
    if ( xRoot == yRoot ) return xRoot;
    if ( rank[xRoot] < rank[yRoot] ) std::swap(xRoot,yRoot);
    if ( rank[xRoot] == rank[yRoot] ) ++ rank[xRoot];
    return parent[yRoot]=xRoot;
  }
  
  uint64_t Find (uint64_t x) const {
    if ( parent[x] != x ) parent[x] = Find ( parent[x] );
    return parent[x];
  }

  std::vector < std::vector < uint64_t > > 
  Components ( void ) const {
    boost::unordered_map < uint64_t, uint64_t > rep_to_component;
    std::vector < std::vector < uint64_t > > components;
    uint64_t N = parent . size ();
    uint64_t num_components = 0;
    for ( uint64_t i = 0; i < N; ++ i ) {
      uint64_t rep = Find ( i );
      if ( rep_to_component . count ( rep ) == 0 ) {
        rep_to_component [ rep ] = num_components ++;
        components . push_back ( std::vector < uint64_t > () );
      }
      components [ rep_to_component [ rep ] ] . push_back ( i );
    }
    return components;
  }

private:
  mutable std::vector<uint64_t> parent;
  std::vector<uint64_t> rank;
};


inline bool is_identity ( const DAG_Data & dag1, const DAG_Data & dag2, const BG_Data & bg ) {
  for ( uint64_t i = 0; i < bg . edges . size (); ++ i ) {
    if ( bg . edges [ i ] . first != bg . edges [ i ] . second ) return false;
  }
  if ( boost::hash<DAG_Data>() ( dag1 ) != boost::hash<DAG_Data>() ( dag2 ) ) return false;
  return true;
}

inline bool is_isomorphism ( const DAG_Data & dag1, const DAG_Data & dag2, const BG_Data & bg ) {
  if ( dag1 . num_vertices != dag2 . num_vertices ) return false;
  boost::unordered_map < int, int > forward;
  boost::unordered_map < int, int > backward;
  typedef std::pair < int, int > Edge;
  size_t num_clutch_edges = 0;
  BOOST_FOREACH ( const Edge & e, bg . edges ) {
    if ( forward . count ( e . first ) != 0 ) return false;
    if ( backward . count ( e . second ) != 0 ) return false;
    forward [ e . first ] = e . second;
    backward [ e . second ] = e . first;
    ++ num_clutch_edges;
  }
  if ( num_clutch_edges != (uint64_t) dag1 . num_vertices ) return false;
  if ( num_clutch_edges != (uint64_t) dag2 . num_vertices ) return false;
  boost::unordered_set < Edge > po1;
  boost::unordered_set < Edge > po2;
  BOOST_FOREACH ( const Edge & e, dag1 . partial_order ) po1 . insert ( e );
  BOOST_FOREACH ( const Edge & e, dag2 . partial_order ) po2 . insert ( e );
  BOOST_FOREACH ( const Edge & e, dag1 . partial_order ) {
    Edge checkme ( forward [ e . first ], forward [ e . second ] );
    if ( po2 . count ( checkme ) == 0 ) return false;
  }
  BOOST_FOREACH ( const Edge & e, dag2 . partial_order ) {
    Edge checkme ( backward [ e . first ], backward [ e . second ] );
    if ( po1 . count ( checkme ) == 0 ) return false;
  }
  return true;
}

inline void Database::postprocess ( void ) {
  uint64_t N = parameter_space_ -> size ();

  // Loop through morse records and create a temporary lookup
  // from grid elements to dag codes

  // TODO: USE EXTERNAL MEMORY SORT TO AVOID RANDOM ACCESS PATTERN
  std::vector < int64_t > grid_to_dag ( N, -1 );
  BOOST_FOREACH ( const MorseRecord & mr, morse_records () ) {
    grid_to_dag [ mr . grid_element ] = mr . dag_index;
  }

  // Process the clutching records sequentially, and create
  // a union-find structure on grid elements
  ContiguousIntegerUnionFind mgccp_uf (N);
  BOOST_FOREACH ( const ClutchingRecord & cr, clutch_records () ) {
    if ( is_identity ( dag_data_ [ grid_to_dag [ cr . grid_element_1 ] ], 
                       dag_data_ [ grid_to_dag [ cr . grid_element_2 ] ],
                       bg_data_ [ cr . bg_index ] ) ) {
      mgccp_uf . Union ( cr . grid_element_1, cr . grid_element_2 );
    }
  }

  // Now we use the union-find structure mgccp_uf to make
  // "Morse Graph Continuation Class Pieces"

  boost::unordered_map < Grid::GridElement, uint64_t > rep_to_mgccp;
  BOOST_FOREACH ( Grid::GridElement ge, * parameter_space_ ) {
    if ( grid_to_dag [ ge ] == - 1) continue; // ignore uncomputed grid elements
    Grid::GridElement mgccp_rep = mgccp_uf . Find ( ge );
    if ( rep_to_mgccp . count ( mgccp_rep ) == 0 ) {
     rep_to_mgccp [ mgccp_rep ] = MGCCP_records_ . size ();
     MGCCP_records_ . push_back ( MGCCP_Record () );
    }
    uint64_t mgccp_index = rep_to_mgccp [ mgccp_rep ];
    MGCCP_records_ [ mgccp_index ] . grid_elements . push_back ( ge );
    MGCCP_records_ [ mgccp_index ] . dag_index = grid_to_dag [ ge ];
  }

  // clear: mgcc_uf, rep_to_mgccp

  // Create singleton INCCP records regardless of continuation
  boost::unordered_map< INCCP_Record, uint64_t > INCCP_to_index;
  ContiguousIntegerUnionFind incc_uf;
  for ( uint64_t mgccp_index = 0; mgccp_index < MGCCP_Records () . size (); ++ mgccp_index ) {
    const MGCCP_Record & mgccp_record = MGCCP_Records () [ mgccp_index ];
    uint64_t dag_index = mgccp_record . dag_index;
    const DAG_Data & dag = dag_data_ [ dag_index ];
    uint64_t n = dag . num_vertices;
    for ( uint64_t i = 0; i < n; ++ i ) {
      CS_Data cs;
      cs . vertices . push_back ( i );
      if ( cs_index_ . count ( cs ) == 0 ) {
        cs_index_ [ cs ] = cs_data_ . size ();
        cs_data_ . push_back ( cs );
      }
      uint64_t cs_index = cs_index_ [ cs ];
      // Produce INCCP Records if necessary
      INCCP_Record inccp_record;
      inccp_record . cs_index = cs_index;
      inccp_record . mgccp_index = mgccp_index;
      if ( INCCP_to_index . count ( inccp_record ) == 0 ) {
          inccp_index_ [ inccp_record ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp_record );
          incc_uf . MakeSet ();
      }
    }
  }
  // Process the clutching records sequentially, and create a union-find
  // structure on MGCC pieces. Also, analyze the bipartite graph
  // connected components to create INCCPs

  ContiguousIntegerUnionFind mgcc_uf ( MGCCP_records_ . size () );
  BOOST_FOREACH ( const ClutchingRecord & cr, clutch_records () ) {
    uint64_t mgccp1 = grid_to_mgccp [ cr . grid_element_1 ];
    uint64_t mgccp2 = grid_to_mgccp [ cr . grid_element_2 ];
    DAG_Data & dag1 = dag_data_ [ grid_to_dag [ cr . grid_element_1 ] ];
    DAG_Data & dag2 = dag_data_ [ grid_to_dag [ cr . grid_element_2 ] ];
    BG_Data & bg = bg_data_ [ cr . bg_index ];

    if ( mgcc_uf . Find ( mgccp1 ) == mgcc_uf . Find ( mgccp2 ) ) continue;
    if ( is_isomorphism ( dag1, dag2, bg ) ) {
      mgcc_uf . Union ( mgccp1, mgccp2 );
    }
    // Analyze ClutchingRecord
    // (1) Generate a list of pairs of convex sets which are matched by the BG
    ContiguousIntegerUnionFind bg_connected_components ( dag1 . num_vertices + dag2 . num_vertices );
    typedef std::pair < int, int > Edge;
    BOOST_FOREACH ( Edge & edge, bg . edges ) {
      bg_connected_components . Union ( edge . first, edge . second + dag1 . num_vertices );
    }
    std::vector < std::vector < uint64_t > > bg_components = bg_connected_components . Components ();
    std::vector < std::vector < uint64_t > > dag1_components ( bg_components . size () );
    std::vector < std::vector < uint64_t > > dag2_components ( bg_components . size () );
    for ( uint64_t i = 0; i < bg_components . size (); ++ i ) {
      BOOST_FOREACH ( uint64_t x, bg_components [ i ] ) {
        if ( x < (uint64_t) dag1 . num_vertices ) dag1_components [ i ] . push_back ( x );
        else dag2_components [ i ] . push_back ( x - dag1 . num_vertices );
      }
      std::sort ( dag1_components [ i ] . begin (), dag1_components [ i ] . end () );
      std::sort ( dag2_components [ i ] . begin (), dag2_components [ i ] . end () );
      // HANDLE INCCP AND INCC_UF
      // For now use singleton condition, but write as if you were doing it more generally
      if ( dag1_components [ i ] . size () == 1 &&
           dag2_components [ i ] . size () == 1 ) {
        // Produce Convex Set Data records if necessary
        CS_Data cs1, cs2;
        BOOST_FOREACH ( uint64_t x, dag1_components [ i ] ) cs1 . vertices . push_back ( x );
        BOOST_FOREACH ( uint64_t x, dag2_components [ i ] ) cs2 . vertices . push_back ( x );
        
        if ( cs_index_ . count ( cs1 ) == 0 ) {
          cs_index_ [ cs1 ] = cs_data_ . size ();
          cs_data_ . push_back ( cs1 );
        }
        uint64_t cs1_index = cs_index_ [ cs1 ];

        if ( cs_index_ . count ( cs2 ) == 0 ) {
          cs_index_ [ cs2 ] = cs_data_ . size ();
          cs_data_ . push_back ( cs2 );
        }
        uint64_t cs2_index = cs_index_ [ cs2 ];

        // Produce INCCP Records if necessary
        INCCP_Record inccp1, inccp2;
        inccp1 . cs_index = cs1_index;
        inccp1 . mgccp_index = mgccp1;
        inccp2 . cs_index = cs2_index;
        inccp2 . mgccp_index = mgccp2;

        if ( INCCP_to_index . count ( inccp1 ) == 0 ) {
          inccp_index_ [ inccp1 ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp1 );
          incc_uf . MakeSet ();
        }
        if ( INCCP_to_index . count ( inccp2 ) == 0 ) {
          inccp_index_ [ inccp2 ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp2 );
          incc_uf . MakeSet ();
        }
        // Call Union operation
        uint64_t inccp1_index = INCCP_to_index [ inccp1 ];
        uint64_t inccp2_index = INCCP_to_index [ inccp2 ];
        incc_uf . Union ( inccp1_index, inccp2_index );
      }
    }
    /* TODO: Deal with non-trivial convex sets (bigger than singletons)
    boost::unordered_map < uint64_t, uint64_t > dag_to_cs1;
    boost::unordered_map < uint64_t, uint64_t > dag_to_cs2;
    boost::unordered_map < uint64_t, uint64_t > rep_to_cs
    for ( uint64_t i = 0; i < dag1 . num_vertices; ++ i ) {
      dag_to_cs1 [ i ] = bg_connected_components . Find ( i );
    }
    for ( uint64_t i = 0; i < dag2 . num_vertices; ++ i ) {
      dag_to_cs2 [ i ] = bg_connected_components . Find ( i + dag1 . num_vertices );
    }  
    */
  }  

  // Now we use the union-find structure on MGCC Pieces to create the MGCC records

  std::vector < std::vector < uint64_t > > mgcc_components = mgcc_uf . Components ();
  for ( uint64_t i = 0; i < mgcc_components . size (); ++ i ) {
    MGCC_records_ . push_back ( MGCC_Record () );
    MGCC_Record & mgcc = MGCC_records_ . back ();
    BOOST_FOREACH ( uint64_t x, mgcc_components [ i ] ) mgcc . mgccp_indices . push_back ( x );
  }

  std::vector < std::vector < uint64_t > > incc_components = incc_uf . Components ();
  for ( uint64_t i = 0; i < incc_components . size (); ++ i ) {
    INCC_records_ . push_back ( INCC_Record () );
    INCC_Record & incc = INCC_records_ . back ();
    BOOST_FOREACH ( uint64_t x, incc_components [ i ] ) incc . inccp_indices . push_back ( x );
  }



  //pb_to_mgcc

  pb_to_mgccp_ . resize ( parameter_space () . size (), MGCCP_Records () . size () );
  mgcc_sizes_ . resize ( MGCC_Records () . size (), 0 );
  mgccp_to_mgcc_ . resize ( MGCCP_Records () . size () );

  for ( uint64_t mgcc_index = 0; mgcc_index < MGCC_Records () . size (); ++ mgcc_index ) {
    MGCC_Record const& mgcc_record = MGCC_Records () [ mgcc_index ];
    BOOST_FOREACH ( uint64_t mgccp_index, mgcc_record . mgccp_indices ) {
      mgccp_to_mgcc_ [ mgccp_index ] = mgcc_index;
      MGCCP_Record & mgccp_record = MGCCP_Records () [ mgccp_index ];
      BOOST_FOREACH ( uint64_t pb, mgccp_record . grid_elements ) {
        pb_to_mgccp_ [ pb ] = mgccp_index;
        ++ mgcc_sizes_ [ mgcc_index ];
      }
    }
  }


  // incc_sizes_: stores sizes of incc's  (Note: doubling counting is possible with interesting continuations)
  // incc_to_mgcc_: lookup set of mgccs via incc
  // inccp_to_incc_: lookup incc via inccp
  incc_sizes_ . resize ( database . INCC_Records () . size (), 0 );
  incc_to_mgcc_ . resize ( database . INCC_Records() . size () );
  inccp_to_incc_ . resize ( database . INCCP_Records() . size () );
  for ( uint64_t incc_index = 0; incc_index < database . INCC_Records () . size (); ++ incc_index ) {
    INCC_Record const& incc_record = database . INCC_Records () [ incc_index ];
    BOOST_FOREACH ( uint64_t inccp_index, incc_record . inccp_indices ) {
      INCCP_Record const& inccp_record = database . INCCP_Records () [ inccp_index ];
      inccp_index_ [ inccp_record ] = inccp_index;
      uint64_t mgccp_index = inccp_record . mgccp_index;
      incc_to_mgcc_ [ incc_index ] . insert ( mgccp_to_mgcc_ [ mgccp_index ] );
      incc_sizes_ [ incc_index ] += database . MGCCP_Records () [ mgccp_index ] . grid_elements . size ();
      inccp_to_incc_ [ inccp_index ] = incc_index;
    }
  }

  
  // mgcc_nb: stored adjacency structure of mgcc's
  mgcc_nb_ . resize ( MGCC_Records () . size () );
  BOOST_FOREACH ( Grid::GridElement pb, parameter_space () ) {
    if ( pb_to_mgccp_[pb] == MGCCP_Records () . size () ) continue;
    std::vector<Grid::GridElement> nbs;
    std::insert_iterator<std::vector<Grid::GridElement> > ii ( nbs, nbs . begin () );
    chomp::Rect r = parameter_space () . geometry ( pb );
    parameter_space () . cover ( ii, r );
    BOOST_FOREACH ( Grid::GridElement nb, nbs ) {
      if ( nb == pb ) continue;
      if ( pb_to_mgccp_[nb] == MGCCP_Records () . size () ) continue;
      mgcc_nb_ [ mgccp_to_mgcc_[pb_to_mgccp_[pb]] ] . insert ( mgccp_to_mgcc_[pb_to_mgccp_[nb]] );
    }
  }

  // Get rid of the bulky parts
  morse_records_ . clear ();
  clutch_records_ . clear ();
}


    // record access
inline const std::vector < MorseRecord > & Database::morse_records ( void ) const { return morse_records_; }
inline const std::vector < ClutchingRecord > & Database::clutch_records ( void ) const { return clutch_records_; }
inline const std::vector < MGCCP_Record > & Database::MGCCP_Records ( void ) const { return MGCCP_records_; }
inline const std::vector < INCCP_Record > & Database::INCCP_Records ( void ) const { return INCCP_records_; }
inline const std::vector < MGCC_Record > & Database::MGCC_Records ( void ) const { return MGCC_records_; }
inline const std::vector < INCC_Record > & Database::INCC_Records ( void ) const { return INCC_records_; }




    // file operations
inline void Database::save ( const char * filename ) {
  std::cout << "Database SAVE\n";
  std::ofstream ofs(filename);
  assert(ofs.good());
  boost::archive::binary_oarchive oa(ofs);
      //boost::archive::text_oarchive oa(ofs);
      //boost::archive::xml_oarchive oa(ofs);
  oa << boost::serialization::make_nvp("database", * this);
      //ofs . close ();
}

inline void Database::load ( const char * filename ) {
      //std::cout << "Database LOAD\n";
  std::ifstream ifs(filename);
  if ( not ifs . good () ) {
    std::cout << "Could not load " << filename << "\n";
    exit ( 1 );
  }

  boost::archive::binary_iarchive ia(ifs);
      //boost::archive::text_iarchive ia(ifs);
      //boost::archive::xml_iarchive ia(ifs);
      // read class state from archive
  ia >> boost::serialization::make_nvp("database",*this);
      //ifs . close ();
}



    /*  CODE FOR CONLEY INDEX THAT HAS BEEN REMOVED
     #include "chomp/ConleyIndex.h"
     
     ///
     BOOST_FOREACH ( const ConleyRecord & record, other . conley_records_ ) {
     insert ( record );
     }
     ///
     
     #if 0
     struct ConleyRecord {
     std::pair < int, int > id_;
     chomp::ConleyIndex_t ci_;
     bool operator < ( const ConleyRecord & rhs ) const;
     friend class boost::serialization::access;
     template<class Archive>
     void serialize(Archive& ar, const unsigned int version) {
     //std::cout << "Serialize conley record.\n";
     //std::cout << "Serialize ID\n";
     ar & boost::serialization::make_nvp("ID",id_);
     //std::cout << "Serialize Conley Index\n";
     
     ar & boost::serialization::make_nvp("CI",ci_);
     }
     };
     
     #endif
     bool ConleyRecord::operator < ( const ConleyRecord & rhs ) const {
     if ( id_ . first == rhs . id_ . first ) return id_ . second < rhs . id_ . second;
     return id_ . first < rhs . id_ . first;
     }
     
     void Database::insert ( const ConleyRecord & record ) {
     conley_records_ . insert ( record );
     }
     
     std::set < ConleyRecord > & Database::conley_records ( void ) {
     return conley_records_;
     }
     const std::set < ConleyRecord > & Database::conley_records ( void ) const {
     return conley_records_;
     }
     */

#endif
