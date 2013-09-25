#ifndef CMDB_JOIN_H
#define CMDB_JOIN_H

#include <boost/shared_ptr.hpp>
#include "database/structures/TreeGrid.h"
#include "database/structures/Atlas.h"

template < class GridType, class InputIterator>
void join ( boost::shared_ptr<GridType> output, 
	          InputIterator start, 
	          InputIterator stop );

// Note: partial template specialization of a function
//       requires a trick, since the language only provides
//       for partial specialization of a class template

template < class GridType, class InputIterator>
struct joinImpl { 
	static void act( boost::shared_ptr<GridType> output, 
	    						 InputIterator start, 
	    						 InputIterator stop ) { 
		// Dynamic Dispatch
		if ( boost::shared_ptr<TreeGrid> ptr = 
			   boost::dynamic_pointer_cast<TreeGrid> ( output ) ) {
			return joinImpl<TreeGrid,InputIterator>::act ( ptr, start, stop );
		}
		if ( boost::shared_ptr<Atlas> ptr = 
			   boost::dynamic_pointer_cast<Atlas> ( output ) ) {
			return joinImpl<Atlas,InputIterator>::act ( ptr, start, stop );
		}
		std::cout << "Error: joinImpl specialization not written for this Grid class.\n";
		abort ();
	} 
};

template < class GridType, class InputIterator>
void join ( boost::shared_ptr<GridType> output, 
	          InputIterator start, 
	          InputIterator stop ) {
	return joinImpl<GridType,InputIterator>::act ( output, start, stop );
}

// joinImpl specializations:

template < class InputIterator >
struct joinImpl < TreeGrid, InputIterator > { 
	static void act ( boost::shared_ptr<TreeGrid> output, 
	    				  		InputIterator start, 
	    					  	InputIterator stop ) { 
		boost::shared_ptr<CompressedGrid> joinup 
			( TreeGrid::join ( start, stop ) );
  	output -> assign ( * joinup );
	} 
};

template < class InputIterator >
struct joinImpl < Atlas, InputIterator > { 
	static void act ( boost::shared_ptr<Atlas> output, 
	    						  InputIterator start, 
	    						  InputIterator stop ) { 

		//std::cout << "Atlas join.\n";
		output -> clear ();
		if ( start == stop ) return;
		boost::shared_ptr<Atlas> start_ptr = boost::dynamic_pointer_cast<Atlas> ( *start );
		// assert ( start_ptr );
		std::vector < Atlas::size_type > chart_ids;
		for ( Atlas::ChartIteratorPair it_pair = start_ptr -> charts (); 
			    it_pair . first != it_pair . second; ++ it_pair . first ) {
			chart_ids . push_back ( it_pair . first -> first );
		}

		BOOST_FOREACH ( Atlas::size_type chart_id, chart_ids ) {
			//std::cout << "Atlas join, top of loop, chart_id=" << chart_id << ".\n";

			std::vector<boost::shared_ptr<TreeGrid> > charts;
			//int atlas_debug_count = 0;
			for ( InputIterator it = start; it != stop; ++ it ) {
				//std::cout << "Looping through Atlas " << atlas_debug_count++ << ".\n";
				boost::shared_ptr<Atlas> it_ptr = 
					boost::dynamic_pointer_cast<Atlas> ( *it );
				if ( not it_ptr ) {
					std::cout << "Atlas::join error: not looping through a container of boost::shared_ptr<Atlas>\n";
					abort ();
				}
				charts . push_back ( it_ptr -> chart ( chart_id ) );
				if ( not boost::dynamic_pointer_cast<TreeGrid> ( it_ptr -> chart ( chart_id ) ) ) {
					std::cout << "Atlas::join error: just pushed back a nonconformant chart\n";
					abort ();
				}
			}
			//std::cout << "Atlas join about to reset\n";
			output -> chart ( chart_id ) . reset ( new PointerGrid );
			//std::cout << "Atlas join about to recurse on join\n";
			//std::cout << "Giving it " << charts . size () << " charts to join.\n";
			join ( output -> chart ( chart_id ), charts . begin (), charts . end () );
			//std::cout << "Atlas join returned from join recursion\n";
		}
	} 
};
#endif
