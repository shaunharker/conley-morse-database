#ifndef CMDB_ABSTRACT_PARAMETERSPACE
#define CMDB_ABSTRACT_PARAMETERSPACE

#include <iostream>
#include <exception>
#include <vector>
#include <random>
#include "database/structures/ParameterSpace.h"

#include <memory>
#include "boost/serialization/serialization.hpp"
#include <boost/serialization/vector.hpp>
#include "boost/serialization/export.hpp"

class AbstractParameterSpace : public ParameterSpace {
public:
	typedef uint64_t ParameterIndex;

	/// Virtual Deconstructor
	virtual ~AbstractParameterSpace ( void ) {}

	/// adjacencies
	///    Return a vector of adjacent vertices.
	virtual std::vector<ParameterIndex>
	adjacencies ( ParameterIndex v ) const;
	
	/// size
	///    Return the number of vertices
	virtual uint64_t 
	size ( void ) const;

	/// parameter
	///    Return the parameter object associated with a vertex
	virtual std::shared_ptr<Parameter> 
	parameter ( ParameterIndex v ) const;
	
	/// search
	///    Given a parameter, find the vertex associated with it
	///    (This can be used to find a parameter which might contain the other)
	virtual uint64_t 
	search ( std::shared_ptr<Parameter> parameter ) const;
	
	/// computeAdjacencyLists
	///    This routine will call adjacency and populate the adjacency lists
	///    It is expected this will be used for serialization of derived classes
	///    where it is not expected the derived class will be recognized by the
	///    loading program.
	void computeAdjacencyLists ( void );

private:
	uint64_t size_;
	std::vector < std::vector < ParameterIndex > > adjacency_lists_;

	// Serialization
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
  	ar & boost::serialization::base_object<ParameterSpace>(*this);
  	ar & size_;
  	ar & adjacency_lists_;
  }
  
};

BOOST_CLASS_EXPORT_KEY(AbstractParameterSpace);

inline std::vector<AbstractParameterSpace::ParameterIndex> 
AbstractParameterSpace::adjacencies ( ParameterIndex v ) const {
	return adjacency_lists_ [ v ];
}
	
inline uint64_t 
AbstractParameterSpace::size ( void ) const {
	return size_;
}

inline std::shared_ptr<Parameter> 
AbstractParameterSpace::parameter ( ParameterIndex v ) const {
	std::cerr << "AbstractParameterSpace::parameter is not meant to be called (missing downcast?)\n";
	throw std::logic_error("AbstractParameterSpace::parameter is not meant to be called (missing downcast?)\n");
}
	
inline uint64_t 
AbstractParameterSpace::search ( std::shared_ptr<Parameter> parameter ) const {
	std::cerr << "AbstractParameterSpace::search is not meant to be called (missing downcast?)\n";
	throw std::logic_error("AbstractParameterSpace::search is not meant to be called (missing downcast?)\n");
}

inline void 
AbstractParameterSpace::computeAdjacencyLists ( void ) {
	size_ = size ();
	std::cout << "AbstractParameterSpace size = " << size () << "\n";
	// begin debug
  /*
	size_t edges = 0;
	size_t vertices = 0;
	std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, size());
	while ( 1 ) {
	//BOOST_FOREACH ( ParameterIndex v, *this ) {
		ParameterIndex v = dis(gen); 
		++ vertices;
		edges += adjacencies (v) . size ();
    if ( vertices % 1000000 == 0 ) std::cout << "Parameter Graph. V = " << vertices << " and E = " << edges << "\n";
  }
  std::cout << "Parameter Graph. V = " << vertices << " and E = " << edges << "\n";
  abort ();
  // end debugf
  */
  adjacency_lists_ . resize ( size () );
	BOOST_FOREACH ( ParameterIndex v, *this ) {
		adjacency_lists_ [ v ] = adjacencies ( v );
	}
}

	
#endif
