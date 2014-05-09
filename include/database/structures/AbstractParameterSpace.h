#ifndef CMDB_ABSTRACT_PARAMETERSPACE
#define CMDB_ABSTRACT_PARAMETERSPACE

#include <iostream>
#include <exception>
#include <vector>

#include "database/structures/ParameterSpace.h"

#include "boost/shared_ptr.hpp"
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
	virtual boost::shared_ptr<Parameter> 
	parameter ( ParameterIndex v ) const;
	
	/// search
	///    Given a parameter, find the vertex associated with it
	///    (This can be used to find a parameter which might contain the other)
	virtual uint64_t 
	search ( boost::shared_ptr<Parameter> parameter ) const;
	
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
  	ar & boost::serialization::base_object<Parameter>(*this);
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

inline boost::shared_ptr<Parameter> 
AbstractParameterSpace::parameter ( ParameterIndex v ) const {
	std::cerr << "AbstractParameterSpace::parameter is not meant to be called (missing downcast?)\n";
	throw std::logic_error("AbstractParameterSpace::parameter is not meant to be called (missing downcast?)\n");
}
	
inline uint64_t 
AbstractParameterSpace::search ( boost::shared_ptr<Parameter> parameter ) const {
	std::cerr << "AbstractParameterSpace::search is not meant to be called (missing downcast?)\n";
	throw std::logic_error("AbstractParameterSpace::search is not meant to be called (missing downcast?)\n");
}

inline void 
AbstractParameterSpace::computeAdjacencyLists ( void ) {
	size_ = size ();
	adjacency_lists_ . resize ( size () );
	BOOST_FOREACH ( ParameterIndex v, *this ) {
		adjacency_lists_ [ v ] = adjacencies ( v );
	}
}

	
#endif
