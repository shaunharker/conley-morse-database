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

#include "database/structures/ParameterSpace.h"

#include "database/structures/Grid.h"
#include "database/structures/SuccinctGrid.h"
#include "database/structures/PointerGrid.h"
#include "database/structures/UniformGrid.h"
#include "database/structures/EdgeGrid.h"

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

// Conley Index Data
struct CI_Data {
  std::vector < std::string > conley_index;
  bool operator == ( const CI_Data & rhs ) const {
    if ( conley_index . size () != rhs . conley_index . size () ) return false;
    for ( uint64_t i = 0; i < conley_index . size (); ++ i ) {
      if ( conley_index [ i ] != rhs . conley_index [ i ] ) return false;
    }
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("CI", conley_index);
  }
};
inline std::size_t hash_value(CI_Data const& ci) {
  std::size_t seed = 0;
  BOOST_FOREACH ( std::string s, ci . conley_index ) {
    boost::hash_combine(seed, s );
  }
  return seed;
}

/*****************/
/*   RECORDS     */
/*****************/

struct Annotation_Record {
  std::set<uint64_t> string_indices;
  bool operator == ( const Annotation_Record & rhs ) const {
    if ( string_indices . size () != rhs . string_indices . size () ) return false;
    std::set<uint64_t>::iterator it1, it2;
    it1 = string_indices . begin ();
    it2 = rhs . string_indices . begin ();
    for ( uint64_t i = 0; i < string_indices . size (); ++ i ) {
      if ( *it1 != *it2 ) return false;
      ++ it1;
      ++ it2;
    }
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("SS", string_indices);
  }
};
inline std::size_t hash_value(Annotation_Record const& stringset) {
  std::size_t seed = 0;
  BOOST_FOREACH ( uint64_t s, stringset . string_indices ) {
    boost::hash_combine(seed, s );
  }
  return seed;
}

// MORSE RECORD

class MorseGraphRecord {
public:
  uint64_t dag_index;
  uint64_t annotation_index;
  std::vector<uint64_t> annotation_index_by_vertex;
  // Constructor
  MorseGraphRecord ( void ) {};
  MorseGraphRecord ( uint64_t dag_index,
                     uint64_t annotation_index,
                     const std::vector<uint64_t> & annotation_index_by_vertex ):
                     dag_index(dag_index), 
                     annotation_index(annotation_index), 
                     annotation_index_by_vertex(annotation_index_by_vertex) {};

  bool operator == ( const MorseGraphRecord & rhs ) const {
    if ( dag_index != rhs . dag_index ) return false;
    if ( annotation_index != rhs . annotation_index ) return false;
    if ( annotation_index_by_vertex . size () 
         != rhs . annotation_index_by_vertex . size () ) return false;
    for ( uint64_t i = 0; i < annotation_index_by_vertex . size (); ++ i ) {
      if ( annotation_index_by_vertex [ i ] != 
           rhs . annotation_index_by_vertex [ i ] ) return false;
    }
    return true;
  }
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("DAG", dag_index );
    ar & boost::serialization::make_nvp("A", annotation_index);
    ar & boost::serialization::make_nvp("ABV", annotation_index_by_vertex);

  }
};
inline std::size_t hash_value(MorseGraphRecord const& mg) {
  std::size_t seed = 0;
  boost::hash_combine(seed, mg . dag_index );
  boost::hash_combine(seed, mg . annotation_index );
  BOOST_FOREACH ( uint64_t a, mg . annotation_index_by_vertex ) {
    boost::hash_combine(seed, a );
  }
  return seed;
}


class ParameterRecord {
public:
  // Data
  uint64_t parameter_index;
  uint64_t morsegraph_index;
  std::vector<uint64_t> morseset_sizes;

  // Constructor
  ParameterRecord ( void ) {};
  ParameterRecord ( uint64_t parameter_index, uint64_t morsegraph_index )
  : parameter_index ( parameter_index ), morsegraph_index ( morsegraph_index ) {};
  
  ParameterRecord ( uint64_t parameter_index, 
                    uint64_t morsegraph_index,
                    const std::vector<uint64_t> & morseset_sizes )
    : parameter_index ( parameter_index ), 
      morsegraph_index ( morsegraph_index ),
      morseset_sizes ( morseset_sizes ) {};
  bool operator < ( const ParameterRecord & rhs ) const {
    return parameter_index < rhs . parameter_index;
  }
  friend class boost::serialization::access;
    template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("PI", parameter_index);
    ar & boost::serialization::make_nvp("MG", morsegraph_index);
    ar & boost::serialization::make_nvp("MSS", morseset_sizes);
  }
};
// CLUTCHING RECORD

class ClutchingRecord {
public:
  uint64_t parameter_index_1;
  uint64_t parameter_index_2;
  uint64_t bg_index;
  
  ClutchingRecord ( void ) {};
  ClutchingRecord ( uint64_t parameter_index_1, uint64_t parameter_index_2, uint64_t bg_index )
  : parameter_index_1 ( parameter_index_1 ), parameter_index_2 ( parameter_index_2 ), bg_index ( bg_index ) {};
  
  bool operator < ( const ClutchingRecord & rhs ) const {
    return std::make_pair ( parameter_index_1, parameter_index_2 ) <
    std::make_pair ( rhs . parameter_index_1, rhs . parameter_index_2 );
  }
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("PI1",parameter_index_1);
    ar & boost::serialization::make_nvp("PI2",parameter_index_2);
    ar & boost::serialization::make_nvp("BG",bg_index);
  }
};

// MORSE GRAPH CONTINUATION CLASS PIECE RECORD

struct MGCCP_Record {
  std::vector<uint64_t> parameter_indices;
  uint64_t morsegraph_index;
 
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("PIS", parameter_indices);
    ar & boost::serialization::make_nvp("MG", morsegraph_index);

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


/// ISOLATING NEIGHBORHOOD CONTINUATION CLASS PIECE RECORD 
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

/// ISOLATING NEIGHBORHOOD CONTINUATION CLASS RECORD
struct INCC_Record {
  /// inccp_indices is a vector of INCCP record indices
  std::vector < uint64_t > inccp_indices;

  /// smallest reps is a set of pairs representing INCC representatives
  ///   the format is (morse set size, (parameter index, convex set index))
  std::set<std::pair<uint64_t,std::pair<uint64_t,uint64_t> > > smallest_reps;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & boost::serialization::make_nvp("INCCPS", inccp_indices);
    ar & boost::serialization::make_nvp("SMALLREP", smallest_reps );
  }
};


/****************/
/*   DATABASE   */
/****************/

class Database {
private:
  friend class boost::serialization::access;
  // raw data
  std::vector < ParameterRecord > parameter_records_;
  std::vector < ClutchingRecord > clutch_records_;
  boost::shared_ptr < ParameterSpace > parameter_space_;
  // data_/index_ pairs
  std::vector < std::string > string_data_;
  std::vector < Annotation_Record > annotation_data_;
  std::vector < MorseGraphRecord > morsegraph_data_;
  std::vector < DAG_Data > dag_data_;
  std::vector < BG_Data > bg_data_;
  std::vector < CS_Data > cs_data_;
  std::vector < CI_Data > ci_data_;
  boost::unordered_map < std::string, uint64_t > string_index_;
  boost::unordered_map < Annotation_Record, uint64_t > annotation_index_;
  boost::unordered_map < MorseGraphRecord, uint64_t > morsegraph_index_;
  boost::unordered_map < DAG_Data, uint64_t > dag_index_;
  boost::unordered_map < BG_Data, uint64_t > bg_index_;
  boost::unordered_map < CS_Data, uint64_t > cs_index_;
  boost::unordered_map < CI_Data, uint64_t > ci_index_;
  // continuation data
  boost::unordered_map < INCCP_Record, uint64_t > inccp_index_;
  std::vector < uint64_t > pb_to_mgccp_;
  std::vector < uint64_t > mgccp_to_mgcc_;
  std::vector < uint64_t > inccp_to_incc_;
  std::vector < boost::unordered_set < uint64_t > > incc_to_mgcc_;
  std::vector < uint64_t > mgcc_sizes_;
  std::vector < uint64_t > incc_sizes_;
  std::vector < boost::unordered_set < uint64_t > > mgcc_nb_;
  std::vector < uint64_t > incc_conley_;
  std::vector < MGCCP_Record > MGCCP_records_;
  std::vector < INCCP_Record > INCCP_records_;
  std::vector < MGCC_Record > MGCC_records_;
  std::vector < INCC_Record > INCC_records_;

public:
  Database ( void ) {}

  /// merge
  ///    merge the contents of another database into this one
  void merge ( const Database & other );

  /// insert 
  ///    insert parameter space
  void insert ( boost::shared_ptr<ParameterSpace> parameter_space );

  /// insert
  ///    insert directed acyclic graph data
  uint64_t insert ( const DAG_Data & dag );

  /// insert
  ///    insert a string
  uint64_t insert ( const std::string & s );

  /// insert
  ///    insert an annotation record
  uint64_t insert ( const Annotation_Record & annotation );

  /// insert
  ///    insert an annotation
  uint64_t insert ( const std::set<std::string> & annotation );

  /// insert
  ///    insert a morse graph record
  uint64_t insert ( const MorseGraphRecord & mgr );

  /// insert
  ///    insert a parameter record
  void insert ( const ParameterRecord & pr );

  /// insert
  ///    Attach Morse Graph to parameter
  void insert ( uint64_t p, const MorseGraph & mg );

  /// insert
  ///    insert bipartite graph data
  uint64_t insert ( const BG_Data & bg );

  /// insert
  ///    insert convex set data
  uint64_t insert ( const CS_Data & cs );

  /// insert
  ///    insert conley index data
  uint64_t insert ( const CI_Data & cs );

  /// insert
  ///    insert a clutching graph between two parameters p1 and p2
  void insert ( uint64_t p1, uint64_t p2, const BG_Data & bg );

  /// insert
  ///    attach Conley Index data to an isolating neighborhood continuation class
  void insert ( uint64_t incc, const CI_Data & ci );


  void postprocess ( void );
  void makeAttractorsMinimal ( void );
  void performTransitiveReductions ( void );
  void save ( const char * filename );   
  void load ( const char * filename );
  
  const ParameterSpace & parameter_space ( void ) const { return *parameter_space_;}

  const std::vector < ParameterRecord > & parameter_records ( void ) const;
  const std::vector < ClutchingRecord > & clutch_records ( void ) const;
  const std::vector < MorseGraphRecord > & morsegraphData ( void ) const 
    { return morsegraph_data_; }
  const std::vector < std::string > & stringData ( void ) const 
    { return string_data_; }
  const std::vector < Annotation_Record > & annotationData ( void ) const 
    { return annotation_data_; }
  const std::vector < DAG_Data > & dagData ( void ) const 
    { return dag_data_; }
  const std::vector < BG_Data > & bgData ( void ) const 
    { return bg_data_; }
  const std::vector < CS_Data > & csData ( void ) const 
    { return cs_data_; }
  const std::vector < CI_Data > & ciData ( void ) const 
    { return ci_data_; }
  uint64_t morsegraphIndex ( MorseGraphRecord const& item ) const 
    { if ( morsegraph_index_ . count ( item ) == 0 ) return morsegraphData().size(); return morsegraph_index_ . find (item) -> second; }
  uint64_t stringIndex ( std::string const& item ) const 
    { if ( string_index_ . count ( item ) == 0 ) return stringData().size(); return string_index_ . find (item) -> second; }
  uint64_t annotationIndex ( Annotation_Record const& item ) const 
    { if ( annotation_index_ . count ( item ) == 0 ) return annotationData().size(); return annotation_index_ . find (item) -> second; }
  uint64_t dagIndex ( DAG_Data const& item ) const 
    { if ( dag_index_ . count ( item ) == 0 ) return dagData().size(); return dag_index_ . find (item) -> second; }
  uint64_t bgIndex ( BG_Data const& item ) const 
    { if ( bg_index_ . count ( item ) == 0 ) return bgData().size(); return bg_index_ . find (item) -> second; }
  uint64_t csIndex ( CS_Data const& item ) const 
    { if ( cs_index_ . count ( item ) == 0 ) return csData().size(); return cs_index_ . find (item) -> second; }
  uint64_t ciIndex ( CI_Data const& item ) const 
    { if ( ci_index_ . count ( item ) == 0 ) return ciData().size(); return ci_index_ . find (item) -> second; }
  uint64_t inccpIndex ( INCCP_Record const& item ) const 
    { if ( inccp_index_ . count ( item ) == 0 ) return INCCP_Records().size(); return inccp_index_ . find (item) -> second; }


  const std::vector < uint64_t > & pb_to_mgccp ( void ) const { return pb_to_mgccp_; }
  const std::vector < uint64_t > & mgccp_to_mgcc ( void ) const { return mgccp_to_mgcc_; }
  const std::vector < uint64_t > & inccp_to_incc ( void ) const { return inccp_to_incc_; }
  const std::vector < boost::unordered_set < uint64_t > > & incc_to_mgcc ( void ) const { return incc_to_mgcc_; }
  const std::vector < uint64_t > & mgcc_sizes ( void ) const { return mgcc_sizes_; }
  const std::vector < uint64_t > & incc_sizes ( void ) const { return incc_sizes_; }
  const std::vector < boost::unordered_set < uint64_t > > & mgcc_nb ( void ) const { return mgcc_nb_; }
  const std::vector < uint64_t > & incc_conley ( void ) const { return incc_conley_; }


  const std::vector < MGCCP_Record > & MGCCP_Records ( void ) const; 
  const std::vector < INCCP_Record > & INCCP_Records ( void ) const; 
  const std::vector < MGCC_Record > & MGCC_Records ( void ) const;  
  const std::vector < INCC_Record > & INCC_Records ( void ) const;    

  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    bool has_space = (bool) parameter_space_;
    ar & boost::serialization::make_nvp("HASPARAMETERSPACE", has_space);
    if ( has_space ) {
      ar & boost::serialization::make_nvp("PARAMETERSPACE", parameter_space_);
    }
    ar & boost::serialization::make_nvp("STRINGDATA", string_data_);
    ar & boost::serialization::make_nvp("ANNOTEDATA", annotation_data_);
    ar & boost::serialization::make_nvp("MGDATA", morsegraph_data_);
    ar & boost::serialization::make_nvp("DAGDATA", dag_data_);
    ar & boost::serialization::make_nvp("BGDATA", bg_data_);
    ar & boost::serialization::make_nvp("CSDATA", cs_data_);
    ar & boost::serialization::make_nvp("CIDATA", ci_data_);
    ar & boost::serialization::make_nvp("STRINGINDEX", string_index_);
    ar & boost::serialization::make_nvp("ANNOTEINDEX", annotation_index_);
    ar & boost::serialization::make_nvp("MGINDEX", morsegraph_index_);
    ar & boost::serialization::make_nvp("DAGINDEX", dag_index_);
    ar & boost::serialization::make_nvp("BGINDEX", bg_index_);
    ar & boost::serialization::make_nvp("CSINDEX", cs_index_);
    ar & boost::serialization::make_nvp("CIINDEX", ci_index_);
    ar & boost::serialization::make_nvp("PARAMETERRECORDS", parameter_records_);
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
   bool is_identity ( const MorseGraphRecord & mgr1, 
                      const MorseGraphRecord & mgr2, 
                      const BG_Data & bg );

   bool is_isomorphism ( const MorseGraphRecord & mgr1, 
                         const MorseGraphRecord & mgr2, 
                         const BG_Data & bg );
};

    /*******************/
    /*   DEFINITIONS   */
    /*******************/

#include <iostream>
#include <fstream>
    //#include <boost/archive/text_oarchive.hpp>
    //#include <boost/archive/text_iarchive.hpp>
#include <cstdio> 
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
// We assume only "raw" records:
// parameter records
// clutching records
// dag data
// bg data
// string data
// annotation data
// morse graph data
inline void Database::merge ( const Database & other ) {
  std::vector < uint64_t > dag_reindex ( other . dag_data_ . size () );
  std::vector < uint64_t > bg_reindex ( other . bg_data_ . size () );
  std::vector < uint64_t > string_reindex ( other . string_data_ . size () );
  std::vector < uint64_t > annotation_reindex ( other . annotation_data_ . size () );
  std::vector < uint64_t > morsegraph_reindex ( other . morsegraph_data_ . size () );

  for ( uint64_t i = 0; i < other . dag_data_ . size (); ++ i ) {
    const DAG_Data & item = other . dag_data_ [ i ];
    dag_reindex [ i ] = insert ( item );
  }
  for ( uint64_t i = 0; i < other . bg_data_ . size (); ++ i ) {
    const BG_Data & item = other . bg_data_ [ i ];
    bg_reindex [ i ] = insert ( item );
  }
  for ( uint64_t i = 0; i < other . string_data_ . size (); ++ i ) {
    const std::string & item = other . string_data_ [ i ];
    string_reindex [ i ] = insert ( item );
  }
  for ( uint64_t i = 0; i < other . annotation_data_ . size (); ++ i ) {
    const Annotation_Record & item = other . annotation_data_ [ i ];
    Annotation_Record converted;
    BOOST_FOREACH ( uint64_t string_index, item . string_indices ) {
      converted . string_indices . insert ( string_reindex [ string_index ] );
    }
    annotation_reindex [ i ] = insert ( converted );
  }
  for ( uint64_t i = 0; i < other . morsegraph_data_ . size (); ++ i ) {
    const MorseGraphRecord & item = other . morsegraph_data_ [ i ];
    MorseGraphRecord converted;
    converted . dag_index = dag_reindex [ item . dag_index ];
    converted . annotation_index = annotation_reindex [ item . annotation_index ];
    BOOST_FOREACH ( uint64_t annotation_index, item . annotation_index_by_vertex ) {
      converted . annotation_index_by_vertex . 
        push_back ( annotation_reindex [ annotation_index ] );
    }
    morsegraph_reindex [ i ] = insert ( converted );
  }

  // records with repeats allowed
  BOOST_FOREACH ( ParameterRecord record, other . parameter_records_ ) {
    record . morsegraph_index = morsegraph_reindex [ record . morsegraph_index ];
    parameter_records_ . push_back ( record );
  }
  BOOST_FOREACH ( ClutchingRecord record, other . clutch_records_ ) {
    record . bg_index = bg_reindex [ record . bg_index ];
    clutch_records_ . push_back ( record );
  }
}

// record insertion
inline void Database::insert ( boost::shared_ptr<ParameterSpace> parameter_space ) {
  parameter_space_ = parameter_space;
}

inline uint64_t Database::insert ( const DAG_Data & dag ) {
  if ( dag_index_ . count ( dag ) == 0 ) {
    dag_index_ [ dag ] = dag_data_ . size ();
    dag_data_ . push_back ( dag );
  }
  return dag_index_ [ dag ];
}


inline uint64_t Database::insert ( const std::string & s ) {
  if ( string_index_ . count ( s ) == 0 ) {
    string_index_ [ s ] = string_data_ . size ();
    string_data_ . push_back ( s );
  }
  return string_index_ [ s ];
}

inline uint64_t Database::insert ( const Annotation_Record & ar ) {
  if ( annotation_index_ . count ( ar ) == 0 ) {
    annotation_index_ [ ar ] = annotation_data_ . size ();
    annotation_data_ . push_back ( ar );
  }
  return annotation_index_ [ ar ];
}

inline uint64_t Database::insert ( const std::set<std::string> & annotation ) {
  Annotation_Record ar;
  BOOST_FOREACH ( const std::string & s, annotation ) {
    ar . string_indices . insert ( insert ( s ) );
  }
  return insert ( ar );
}

inline uint64_t Database::insert ( const MorseGraphRecord & mgr ) {
  if ( morsegraph_index_ . count ( mgr ) == 0 ) {
    morsegraph_index_ [ mgr ] = morsegraph_data_ . size ();
    morsegraph_data_ . push_back ( mgr );
  }
  return morsegraph_index_ [ mgr ];
}

inline void Database::insert ( const ParameterRecord & pr ) {
  parameter_records_ . push_back ( pr );
}

inline void Database::insert ( uint64_t p, const MorseGraph & mg ) {
  // Create DAG Data and/or Retrieve dag_index 
  DAG_Data dag ( mg );
  uint64_t dag_index = insert ( dag );
  // Create Annotation Records and/or retrieve indices
  // Also, acquire morse set size information
  uint64_t annotation_index = insert ( mg . annotation () );
  uint64_t num_vertices = mg . NumVertices ();
  std::vector < uint64_t > annotation_index_by_vertex ( num_vertices );
  std::vector < uint64_t > morseset_sizes ( num_vertices );

  for ( uint64_t v = 0; v < num_vertices; ++ v ) {
    annotation_index_by_vertex [ v ] = insert ( mg . annotation ( v ) ); 
    morseset_sizes [ v ] = mg . grid ( v ) -> size ();
  }
  // Create MorseGraphRecord
  MorseGraphRecord mgr ( dag_index, annotation_index, annotation_index_by_vertex );
  uint64_t morsegraph_index = insert ( mgr );

  // Create ParameterRecord
  ParameterRecord pr ( p, morsegraph_index, morseset_sizes );
  insert ( pr );
}

inline uint64_t Database::insert ( const BG_Data & bg ) {
  if ( bg_index_ . count ( bg ) == 0 ) {
    bg_index_ [ bg ] = bg_data_ . size ();
    bg_data_ . push_back ( bg );
  }
  return bg_index_ [ bg ];
}

inline uint64_t Database::insert ( const CS_Data & cs ) {
  if ( cs_index_ . count ( cs ) == 0 ) {
    cs_index_ [ cs ] = cs_data_ . size ();
    cs_data_ . push_back ( cs );
  }
  return cs_index_ [ cs ];
}

inline uint64_t Database::insert ( const CI_Data & ci ) {
  if ( ci_index_ . count ( ci ) == 0 ) {
    ci_index_ [ ci ] = ci_data_ . size ();
    ci_data_ . push_back ( ci );
  }
  return ci_index_ [ ci ];
}
inline void Database::insert ( uint64_t p1, 
                               uint64_t p2, 
                               const BG_Data & bg ) {
  uint64_t bg_index = insert ( bg );
  clutch_records_ . push_back ( ClutchingRecord ( p1, p2, bg_index ) );
}

inline void Database::insert ( uint64_t incc, const CI_Data & ci ) {
  uint64_t ci_index = insert ( ci );
  incc_conley_ . resize ( INCC_records_ . size () );
  incc_conley_ [ incc ] = ci_index;
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

inline bool Database::is_identity ( const MorseGraphRecord & mgr1, 
                                    const MorseGraphRecord & mgr2, 
                                    const BG_Data & bg ) {
  const DAG_Data & dag1 = dagData()[mgr1.dag_index];
  const DAG_Data & dag2 = dagData()[mgr2.dag_index];
  if ( dag1 . num_vertices != dag2 . num_vertices ) return false;
  if ( dag1 . num_vertices != bg . edges . size () ) return false;
  for ( uint64_t i = 0; i < bg . edges . size (); ++ i ) {
    if ( bg . edges [ i ] . first != bg . edges [ i ] . second ) return false;
  }
  return ( mgr1 == mgr2 );
}

inline bool Database::is_isomorphism ( const MorseGraphRecord & mgr1, 
                                       const MorseGraphRecord & mgr2, 
                                       const BG_Data & bg ) {
  const DAG_Data & dag1 = dagData()[mgr1.dag_index];
  const DAG_Data & dag2 = dagData()[mgr2.dag_index];
  // Check that it is a bijection
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
    if ( mgr1.annotation_index_by_vertex[e.first] != 
         mgr2.annotation_index_by_vertex[e.second] ) return false;
    ++ num_clutch_edges;
  }
  if ( num_clutch_edges != (uint64_t) dag1 . num_vertices ) return false;
  if ( num_clutch_edges != (uint64_t) dag2 . num_vertices ) return false;
  // Check that the partial order is respected
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
  typedef uint64_t ParameterIndex;

  uint64_t N = parameter_space_ -> size ();

  std::cout << "Database::postprocess\n";
  std::cout << " Number of parameters = " << N << "\n";
  std::cout << " Number of Morse Records = " << parameter_records () . size () << "\n";
  std::cout << " Number of DAGs = " << dagData () . size () << "\n";
  // Loop through morse records and create a temporary lookup
  // from parameter indices to dag codes

  // TODO: USE EXTERNAL MEMORY SORT TO AVOID RANDOM ACCESS PATTERN
  std::vector < int64_t > param_to_mgr ( N, -1 );
  BOOST_FOREACH ( const ParameterRecord & pr, parameter_records () ) {
    uint64_t morsegraph_index = pr . morsegraph_index;
    const MorseGraphRecord & mgr = morsegraphData() [ morsegraph_index ];
    if ( dagData()[ mgr . dag_index] . num_vertices == 0 ) {
      std::cout << "Warning: Parameter Record " << pr . parameter_index 
      << " is associated with DAG << " << mgr . dag_index << ", which has no vertices.\n";
      continue;
    }
    param_to_mgr [ pr . parameter_index ] = (int64_t) morsegraph_index;
  }

  // Process the clutching records sequentially, and create
  // a union-find structure on parameters

  //std::cout << "Database::postprocess Process clutching in sequence and create union-find\n";
  ContiguousIntegerUnionFind mgccp_uf (N);
  BOOST_FOREACH ( const ClutchingRecord & cr, clutch_records () ) {
    // Handle spurious records
    if ( param_to_mgr [ cr . parameter_index_1 ] == -1 ||
        param_to_mgr [ cr . parameter_index_2 ] == -1 ) {
      std::cout << "Warning: database has invalid clutching records.\n";
      continue;
    }

    if ( is_identity ( morsegraph_data_ [ param_to_mgr [ cr . parameter_index_1 ] ], 
                       morsegraph_data_ [ param_to_mgr [ cr . parameter_index_2 ] ],
                       bg_data_ [ cr . bg_index ] ) ) {
      mgccp_uf . Union ( cr . parameter_index_1, cr . parameter_index_2 );
    }
  }

  // Now we use the union-find structure mgccp_uf to make
  // "Morse Graph Continuation Class Pieces"
  //std::cout << "Database::postprocess create MGCCP \n";

  boost::unordered_map < ParameterIndex, uint64_t > rep_to_mgccp;
  BOOST_FOREACH ( ParameterIndex pi, * parameter_space_ ) {
    if ( param_to_mgr [ pi ] == - 1) continue; // ignore uncomputed parameters
    ParameterIndex mgccp_rep = mgccp_uf . Find ( pi );
    if ( rep_to_mgccp . count ( mgccp_rep ) == 0 ) {
     rep_to_mgccp [ mgccp_rep ] = MGCCP_records_ . size ();
     MGCCP_records_ . push_back ( MGCCP_Record () );
    }
    uint64_t mgccp_index = rep_to_mgccp [ mgccp_rep ];
    MGCCP_records_ [ mgccp_index ] . parameter_indices . push_back ( pi );
    MGCCP_records_ [ mgccp_index ] . morsegraph_index = param_to_mgr [ pi ];
  }

  // clear: mgcc_uf, rep_to_mgccp

  pb_to_mgccp_ . resize ( parameter_space () . size (), MGCCP_Records () . size () );
  for ( uint64_t mgccp_index = 0; mgccp_index < MGCCP_Records () . size (); ++ mgccp_index ) {
    const MGCCP_Record & mgccp_record = MGCCP_Records () [ mgccp_index ];
    BOOST_FOREACH ( ParameterIndex pi, mgccp_record . parameter_indices ) {
      pb_to_mgccp_ [ pi ] = mgccp_index;
    }
  }

  // Create singleton INCCP records regardless of continuation
  //std::cout << "Database::postprocess create INCCP \n";

  ContiguousIntegerUnionFind incc_uf;
  for ( uint64_t mgccp_index = 0; mgccp_index < MGCCP_Records () . size (); ++ mgccp_index ) {
    const MGCCP_Record & mgccp_record = MGCCP_Records () [ mgccp_index ];
    const MorseGraphRecord & mgr = morsegraphData() [ mgccp_record . morsegraph_index ];
    const DAG_Data & dag = dag_data_ [ mgr . dag_index ];
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
      if ( inccp_index_ . count ( inccp_record ) == 0 ) {
          inccp_index_ [ inccp_record ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp_record );
          incc_uf . MakeSet ();
      }
    }
  }
  // Process the clutching records sequentially, and create a union-find
  // structure on MGCC pieces. Also, analyze the bipartite graph
  // connected components to create INCCPs
  //std::cout << "Database::postprocess create mgcc_uf\n";

  ContiguousIntegerUnionFind mgcc_uf ( MGCCP_records_ . size () );
  BOOST_FOREACH ( const ClutchingRecord & cr, clutch_records () ) {
    if ( param_to_mgr [ cr . parameter_index_1 ] == -1 ||
        param_to_mgr [ cr . parameter_index_2 ] == -1 ) {
      std::cerr << "Warning: database has invalid clutching records.\n";
      continue;
    }
    uint64_t mgccp1 = pb_to_mgccp_ [ cr . parameter_index_1 ];
    uint64_t mgccp2 = pb_to_mgccp_ [ cr . parameter_index_2 ];

    const MorseGraphRecord & mgr1 = morsegraph_data_ [ param_to_mgr [ cr . parameter_index_1 ] ];
    const MorseGraphRecord & mgr2 = morsegraph_data_ [ param_to_mgr [ cr . parameter_index_2 ] ];

    BG_Data & bg = bg_data_ [ cr . bg_index ];

    if ( mgcc_uf . Find ( mgccp1 ) == mgcc_uf . Find ( mgccp2 ) ) continue;
    if ( is_isomorphism ( mgr1, mgr2, bg ) ) {
      mgcc_uf . Union ( mgccp1, mgccp2 );
    }
    // Analyze ClutchingRecord
    // (1) Generate a list of pairs of convex sets which are matched by the BG
    const DAG_Data & dag1 = dag_data_ [ mgr1 . dag_index ];
    const DAG_Data & dag2 = dag_data_ [ mgr2 . dag_index ];
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
        
        uint64_t cs1_index = insert ( cs1 );
        uint64_t cs2_index = insert ( cs2 );

        // Check if annotations match (continue if they do not)
        // WARNING, USES SINGLETON ASSUMPTION
        uint64_t v1 = * dag1_components [ i ] . begin ();
        uint64_t v2 = * dag2_components [ i ] . begin ();
        if ( mgr1.annotation_index_by_vertex[v1] 
             != mgr2.annotation_index_by_vertex[v2] ) continue;
        // END ANNOTATION MATCHING

        // Produce INCCP Records if necessary
        INCCP_Record inccp1, inccp2;
        inccp1 . cs_index = cs1_index;
        inccp1 . mgccp_index = mgccp1;
        inccp2 . cs_index = cs2_index;
        inccp2 . mgccp_index = mgccp2;

        if ( inccp_index_ . count ( inccp1 ) == 0 ) {
          inccp_index_ [ inccp1 ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp1 );
          incc_uf . MakeSet ();
        }
        if ( inccp_index_ . count ( inccp2 ) == 0 ) {
          inccp_index_ [ inccp2 ] = INCCP_records_ . size ();
          INCCP_records_ . push_back ( inccp2 );
          incc_uf . MakeSet ();
        }
        // Call Union operation
        uint64_t inccp1_index = inccp_index_ [ inccp1 ];
        uint64_t inccp2_index = inccp_index_ [ inccp2 ];
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

  // Now we use the union-find structure on INCC Pieces to create the INCC records
  std::vector < std::vector < uint64_t > > incc_components = incc_uf . Components ();
  for ( uint64_t i = 0; i < incc_components . size (); ++ i ) {
    INCC_records_ . push_back ( INCC_Record () );
    INCC_Record & incc = INCC_records_ . back ();
    BOOST_FOREACH ( uint64_t x, incc_components [ i ] ) incc . inccp_indices . push_back ( x );
  }



  mgcc_sizes_ . resize ( MGCC_Records () . size (), 0 );
  mgccp_to_mgcc_ . resize ( MGCCP_Records () . size () );

  for ( uint64_t mgcc_index = 0; mgcc_index < MGCC_Records () . size (); ++ mgcc_index ) {
    MGCC_Record const& mgcc_record = MGCC_Records () [ mgcc_index ];
    BOOST_FOREACH ( uint64_t mgccp_index, mgcc_record . mgccp_indices ) {
      mgccp_to_mgcc_ [ mgccp_index ] = mgcc_index;
      MGCCP_Record const& mgccp_record = MGCCP_Records () [ mgccp_index ];
      mgcc_sizes_ [ mgcc_index ] += mgccp_record . parameter_indices . size ();
    }
  }


  // incc_sizes_: stores sizes of incc's  (Note: doubling counting is possible with interesting continuations)
  // incc_to_mgcc_: lookup set of mgccs via incc
  // inccp_to_incc_: lookup incc via inccp
  incc_sizes_ . resize ( INCC_Records () . size (), 0 );
  incc_to_mgcc_ . resize ( INCC_Records() . size () );
  inccp_to_incc_ . resize ( INCCP_Records() . size () );
  for ( uint64_t incc_index = 0; incc_index < INCC_Records () . size (); ++ incc_index ) {
    INCC_Record const& incc_record = INCC_Records () [ incc_index ];
    BOOST_FOREACH ( uint64_t inccp_index, incc_record . inccp_indices ) {
      INCCP_Record const& inccp_record = INCCP_Records () [ inccp_index ];
      inccp_index_ [ inccp_record ] = inccp_index;
      uint64_t mgccp_index = inccp_record . mgccp_index;
      incc_to_mgcc_ [ incc_index ] . insert ( mgccp_to_mgcc_ [ mgccp_index ] );
      incc_sizes_ [ incc_index ] += MGCCP_Records () [ mgccp_index ] . parameter_indices . size ();
      inccp_to_incc_ [ inccp_index ] = incc_index;
    }
  }


  // calculate correct smallest_reps field for INCC_Records
  BOOST_FOREACH ( const ParameterRecord & pr, parameter_records () ) {
    ParameterIndex pi = pr . parameter_index;
    uint64_t morsegraph_index = pr . morsegraph_index;
    const MorseGraphRecord & mgr = morsegraphData() [ morsegraph_index ];
    uint64_t mgccp_index = pb_to_mgccp_ [ pi ];
    // Iterate through INCCP records associated with MGCCP 
    //   (unforunately this is complicated)
    uint64_t n = dagData()[ mgr . dag_index] . num_vertices;
    for ( uint64_t i = 0; i < n; ++ i ) {
      CS_Data cs;
      cs . vertices . push_back ( i );
      if ( cs_index_ . count ( cs ) == 0 ) {
        cs_index_ [ cs ] = cs_data_ . size ();
        cs_data_ . push_back ( cs );
      }
      uint64_t cs_index = cs_index_ [ cs ];
      INCCP_Record inccp_record;
      inccp_record . cs_index = cs_index;
      inccp_record . mgccp_index = mgccp_index;
      // Fetch INCC associated with INCCP
      uint64_t inccp_index = inccp_index_ [ inccp_record ];
      uint64_t incc_index = inccp_to_incc_ [ inccp_index ];
      INCC_Record & incc_record = INCC_records_ [ incc_index ];
      uint64_t morseset_size = pr . morseset_sizes [ i ];
      incc_record . smallest_reps . 
        insert ( std::make_pair ( morseset_size, std::make_pair ( pi, i ) ) );
      // UNIMPLEMENTED FEATURE: make number of smallest reps held configurable
      if ( incc_record . smallest_reps . size () > 16 ) {
        incc_record . smallest_reps . erase ( * incc_record . smallest_reps . rbegin () );
      }
    }
  }


  
  // mgcc_nb: stored adjacency structure of mgcc's
  mgcc_nb_ . resize ( MGCC_Records () . size () );
  BOOST_FOREACH ( ParameterIndex pb, parameter_space () ) {
    if ( pb_to_mgccp_[pb] == MGCCP_Records () . size () ) continue;
    std::vector<ParameterIndex> nbs = parameter_space () . adjacencies ( pb );
    BOOST_FOREACH ( ParameterIndex nb, nbs ) {
      if ( nb == pb ) continue;
      if ( pb_to_mgccp_[nb] == MGCCP_Records () . size () ) continue;
      mgcc_nb_ [ mgccp_to_mgcc_[pb_to_mgccp_[pb]] ] . insert ( mgccp_to_mgcc_[pb_to_mgccp_[nb]] );
    }
  }

  // Get rid of the bulky parts
  parameter_records_ . clear ();
  clutch_records_ . clear ();
}

inline void Database::makeAttractorsMinimal ( void ) {
  // Loop through all INCCs
  //   Check Conley Index and see if it is an attractor
  //   If it is an attractor,
  //     Loop through all INCCPs
  //       Obtain the corresponding MGCCP
  //       Identify the convex set corresponding to the IN.
  //       Construct the Morse Graph corresponding to removing the out-edges of the convex set
  //           (apart from internal out-edges within convex set)
  //       Alter the MGCCP record to point to the new Morse Graph
  //     End Loop
  //   End If
  // End Loop
  uint64_t number_of_inccs = incc_conley_ . size ();
  for ( uint64_t incc = 0; incc < number_of_inccs; ++ incc ) {
    // Check if it is an attractor
    const std::vector<std::string> & conley_string = 
      ciData () [ incc_conley () [ incc ] ] . conley_index;
    if ( conley_string . size () == 0 ) continue;
    if ( conley_string [ 0 ] != "Trivial.\n" && 
         conley_string [ 0 ] != "Relative Homology computation timed out.\n" && 
         conley_string [ 0 ] != "Problem computing SNF.\n" ) {
      const INCC_Record & incc_record = INCC_Records () [ incc ];
      BOOST_FOREACH ( uint64_t inccp, incc_record . inccp_indices ) {
        //std::cout << "INCCP = " << inccp << "\n";
        //std::cout << conley_string [ 0 ] << "\n";
        const INCCP_Record & inccp_record = INCCP_Records () [ inccp ];
        uint64_t mgccp = inccp_record . mgccp_index;
        uint64_t cs = inccp_record . cs_index;
        const MGCCP_Record & mgccp_record = MGCCP_records_ [ mgccp ];
        MorseGraphRecord & mgr = morsegraph_data_ [ mgccp_record . morsegraph_index ];
        DAG_Data dag_data = dagData () [ mgr . dag_index ];
        const CS_Data & cs_data = csData () [ cs ];
        DAG_Data new_dag;
        new_dag . num_vertices = dag_data . num_vertices;
        boost::unordered_set < int > convex_set_vertices;
        BOOST_FOREACH( int v, cs_data . vertices ) convex_set_vertices . insert ( v );
        //std::cout << "CSV size = " << convex_set_vertices . size () << "\n";
        if ( convex_set_vertices . size () == 0 ) continue;
        //std::cout << "CS:";
        //BOOST_FOREACH( int v, cs_data . vertices ) std::cout << v << " ";
        //std::cout << "\n";
        for ( int i = 0; i < (int) dag_data . partial_order . size (); ++ i ) {
          std::pair<int,int> edge = dag_data . partial_order [ i ];
          // retain the edge only if it originates outside the convex set
          // or else targets the convex set
          // (eqivalently, throw out the edge only if it originates inside the convex set
          //   and targets outside the convex set)
          //std::cout << "Inspecting edge " << edge.first << ", " << edge.second << "\n";
          if ( (convex_set_vertices . count ( edge . first ) == 0)  ||
               (convex_set_vertices . count ( edge . second ) != 0)  ) {
            //std::cout << "Pushing edge " << edge.first << ", " << edge.second << "\n";
            new_dag . partial_order . push_back ( edge );
          }
        }
        // register the new dag
        uint64_t new_dag_index = insert ( new_dag ); 
        //std::cout << "Old dag index = " << old_dag_index << "\n";
        //std::cout << "New dag index = " << new_dag_index << "\n";
        mgr . dag_index = new_dag_index;
      }
    }
  }
}

inline void Database::performTransitiveReductions ( void ) {
  // tricky part: to update the dags, we need to update the lookup table too
  for ( uint64_t dag_index = 0; dag_index < dag_data_ . size (); ++ dag_index ) {
    DAG_Data & dag = dag_data_ [ dag_index ];
    dag_index_ . erase ( dag );
    boost::unordered_map < int, boost::unordered_set < int > > G, squared;
    for ( int i = 0; i < (int) dag . partial_order . size (); ++ i ) {
      std::pair<int,int> edge = dag.partial_order[i];
      if ( edge . second == edge . first ) continue;
      G[edge.first].insert(edge.second);
    }
    for ( int u = 0; u < dag . num_vertices; ++ u ) {
      BOOST_FOREACH( int v, G [ u ] ) {
        BOOST_FOREACH ( int w, G [ v ] ) {
          squared [ u ] . insert ( w );
        }
      }
    }
    std::vector<std::pair<int,int> > reduced;
    for ( int i = 0; i < (int)dag . partial_order . size (); ++ i ) {
      std::pair<int,int> edge = dag.partial_order[i];
      if ( squared [ edge.first ] . count ( edge . second ) == 0 )
        reduced . push_back ( edge );
    }
    dag . partial_order = reduced;
    dag_index_ [ dag ] = dag_index;
  }
}

// record access
inline const std::vector < ParameterRecord > & Database::parameter_records ( void ) const { return parameter_records_; }
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
  std::cout << "Database LOAD\n";
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

#endif
