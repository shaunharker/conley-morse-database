
#ifndef ATLASGEO_H
#define ATLASGEO_H

#include "database/structures/Geo.h"
#include "database/structures/RectGeo.h"

// Geometrical Object for Atlas : Pair < int, Rect >
class AtlasGeo : public Geo {
	typedef uint64_t size_type;
public:

	AtlasGeo ( void ) {}
	AtlasGeo ( size_type id, RectGeo rect ) : id_(id), rect_(rect) {}

	const RectGeo & rect ( void ) const { return rect_; } 
	RectGeo & rect ( void ) { return rect_; }
	size_type id ( void ) const { return id_; }
	size_type & id ( void ) { return id_; }

	void info ( void ) { 
		std::cout << "Chart identification : " << id_ << ", with Rect : " << rect_ << "\n";
	}

private: 
	size_type id_;
	RectGeo rect_;
};

#endif
