#ifndef ATLAS_H
#define ATLAS_H

#include <iostream>
#include <stdint.h>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "database/structures/Grid.h"
#include "database/structures/TreeGrid.h"
#include "database/structures/PointerGrid.h"
//#include "database/structures/Chart.h"
#include "database/structures/Geo.h"
#include "database/structures/AtlasGeo.h"

class Atlas : public Grid { 

public:
	typedef uint64_t GridElement;
  typedef boost::counting_iterator < GridElement > iterator;
  typedef iterator const_iterator;
  typedef uint64_t size_type;
  typedef boost::shared_ptr<TreeGrid> Chart;

  // Contructor/ Desctructor
  Atlas ( void ) { }
  ~Atlas ( void ) { }

  // Builders

  // General Methods
  virtual Atlas * clone ( void ) const;
  virtual void subdivide ( void );
  virtual Grid * subgrid ( const std::deque < GridElement > & grid_elements ) const;
  virtual std::vector<GridElement> subset ( const Grid & other ) const;
  virtual boost::shared_ptr<Geo> geometry ( GridElement ge ) const;  
  virtual std::vector<Grid::GridElement> cover ( const Geo & geo ) const;
  using Grid::geometry;
  using Grid::cover;

  // Features
  typedef boost::unordered_map<size_type, Chart>::const_iterator ChartIterator;
  typedef std::pair<ChartIterator, ChartIterator> ChartIteratorPair;
  typedef std::pair <size_type, Chart > IdChartPair;
  std::pair<ChartIterator, ChartIterator> charts ( void ) const;

  Chart & chart ( size_type chart_id );  
  const Chart & chart ( size_type chart_id ) const;

  void clear ( void );

  void importCharts ( const char * inputfile );
  
  void add_chart ( const int & id, const RectGeo & rect);

  void list_charts ( void ) {
    boost::unordered_map < size_type, Chart >::const_iterator it;
    std::cout << "\nList of charts :\n";
    for ( it=charts_.begin(); it!=charts_.end(); ++it ) {
      std::cout << "index = " << it -> first << " , ";
      std::cout << "bounds = " << it -> second -> bounds ( ) << " , "; 
      std::cout << "number of GridElements = " << it -> second -> size ( ) << "\n";
    }  
  }
  virtual uint64_t memory ( void ) const {
    uint64_t result = 0;
    BOOST_FOREACH ( const IdChartPair & chartpair, charts () ) {
      result += chartpair . second -> memory ();
    }
    return result;
  }

private:
  boost::unordered_map < size_type, Chart > charts_; 
	// to find quickly (sort of) which chart the gridElement belongs to
	// useful for the method geometry
  // TO BE CHANGED
	std::vector < size_type > csum_;
  void update_csum_ ( void ) {
    boost::unordered_map < size_type, Chart >::const_iterator it;
    size_type sum = 0;
    csum_ . clear ( );
    for ( it=charts_.begin(); it!=charts_.end(); ++it ) {
      csum_ . push_back ( sum += it -> second -> size ( ) );
    }  
    //std::cout << "DEBUG update_csum_ " << csum_ . back () << "\n";
    size_ = csum_ . back ();
  }

  // Need functions to convert the GridElement from Chart indexing to Atlas indexing
  GridElement Chart_to_Atlas_GridElement_ ( const GridElement & chart_ge, const size_type & chart_id ) const;
  std::pair < size_type, GridElement > Atlas_to_Chart_GridElement_ ( const GridElement & atlas_ge ) const;

};


inline Atlas * Atlas::clone ( void ) const {
  Atlas * newAtlas = new Atlas;
  for ( ChartIteratorPair it_pair = charts (); 
        it_pair . first != it_pair . second;
        ++ it_pair . first ) {
    boost::shared_ptr<TreeGrid> chart_ptr ( (TreeGrid *) (it_pair . first -> second -> clone ()) );
    newAtlas -> chart ( it_pair . first -> first ) = chart_ptr;    
  }
  //std::cout << "Atlas::clone " << newAtlas -> charts_ . size () << "\n";
  newAtlas -> update_csum_ ();
  return newAtlas;
}


inline void Atlas::subdivide ( void ) { 
  // Subdivide each Chart and update the cumulative sum
  boost::unordered_map < size_type, Chart >::iterator it;
  for ( it=charts_.begin(); it!=charts_.end(); ++it ) {
    //std::cout << "DEBUG Atlas::subdivide SUBDIVIDING A CHART\n";
    it -> second -> subdivide ( );
  }  
  update_csum_ ( );
  //std::cout << "Atlas::subdivide -- size () == " << size () << "\n";

}

inline Grid * Atlas::subgrid ( const std::deque < GridElement > & grid_elements ) const {
  boost::unordered_map < size_type, std::deque < GridElement > > chart_grid_elements;
  BOOST_FOREACH ( GridElement ge, grid_elements ) {
    std::pair < size_type, GridElement > atlas_ge = Atlas_to_Chart_GridElement_ ( ge );
    chart_grid_elements [ atlas_ge . first ] . push_back ( atlas_ge . second );
  }
  Atlas * newAtlas = new Atlas;
  boost::unordered_map < size_type, Chart >::const_iterator it;
  for ( it=charts_.begin(); it!=charts_.end(); ++it ) {
    Grid * subchart = it -> second -> subgrid ( chart_grid_elements [ it -> first ] );
    newAtlas -> charts_ [ it -> first ] = 
      boost::shared_ptr<TreeGrid> ( (TreeGrid *) subchart );
  }  
  newAtlas -> update_csum_ ();
  return (Grid *) newAtlas;
}

inline std::vector<Grid::GridElement> Atlas::subset ( const Grid & other ) const {
  const Atlas & otherAtlas = dynamic_cast<const Atlas &> (other);
  std::vector<Grid::GridElement> result;
  boost::unordered_map < size_type, Chart >::const_iterator it;
  for ( it=charts_.begin(); it!=charts_.end(); ++it ) {
    std::vector<Grid::GridElement> chart_subset = 
      it -> second -> subset ( * otherAtlas . charts_ . find ( it -> first ) -> second );
    BOOST_FOREACH ( Grid::GridElement ge, chart_subset ) {
      result . push_back ( Chart_to_Atlas_GridElement_ ( ge, it -> first ) );
    }
  }
  return result;
}

inline boost::shared_ptr<Geo> Atlas::geometry ( Grid::GridElement ge ) const {
  std::pair < size_type, GridElement > chartge;
  chartge = Atlas_to_Chart_GridElement_ ( ge );
  RectGeo rect = * boost::dynamic_pointer_cast < RectGeo > 
    ( charts_ . find ( chartge . first ) -> second -> geometry ( chartge . second ) );
  return boost::shared_ptr<Geo> ( new AtlasGeo ( chartge.first, rect ) );
} /* Atlas::geometry */

inline std::vector<Grid::GridElement>
Atlas::cover ( const Geo & geo ) const { 
  // TODO: Make this work for a full Atlas.
  const AtlasGeo & geometric_region = dynamic_cast<const AtlasGeo &> ( geo );
  std::vector<Grid::GridElement> result;
  // index of the chart : geometric_region . first
  // Rect to be covered : geometric_region . second
  // cover the Rect in the given chart
  std::vector < GridElement > listge = 
    charts_ . find ( geometric_region . id() ) -> second -> cover ( geometric_region . rect() );
  // // shift the gridelement from chart to atlas and insert it to ii
  BOOST_FOREACH ( Grid::GridElement chart_ge, listge ) {
    GridElement newge = Chart_to_Atlas_GridElement_ ( chart_ge , geometric_region . id() );
    //std::cout << "DEBUG Atlas::cover -- pushing " << chart_ge 
    //  << ", " << geometric_region.id() << " -- " << newge << "\n";
    result . push_back ( newge );
  }
  return result;
}

inline void Atlas::add_chart ( const int & id, const RectGeo & rect ) {
  charts_ [ id ] = boost::shared_ptr<TreeGrid> ( new PointerGrid );
  charts_ [ id ] -> initialize ( rect );
  update_csum_ ( );
}


inline void Atlas::importCharts ( const char * inputfile ) {
  using boost::property_tree::ptree;
  ptree pt;
  std::ifstream input ( inputfile );
  read_xml(input, pt);
//
  unsigned int dimension = pt.get<int>("atlas.dimension");
  std::cout << "Dimension : " << dimension << "\n";
//
  std::vector < double > lower_bounds, upper_bounds;
  lower_bounds . resize ( dimension );
  upper_bounds . resize ( dimension );
//
  BOOST_FOREACH ( ptree::value_type & v, pt.get_child("atlas.listcharts") ) {
    // extract the strings
    std::string idstr = v . second . get_child ( "id" ) . data ( );
    std::string lbstr = v . second . get_child ( "lbounds" ) . data ( );
    std::string ubstr = v . second . get_child ( "ubounds" ) . data ( );
    //
    std::stringstream idss ( idstr );
    std::stringstream lbss ( lbstr );
    std::stringstream ubss ( ubstr );
    //
    int id;
    idss >> id;
    for ( unsigned int d = 0; d < dimension; ++ d ) {
      lbss >> lower_bounds [ d ];
      ubss >> upper_bounds [ d ];
    }
    // add the new chart 
    add_chart ( id, RectGeo(dimension,lower_bounds,upper_bounds) ); 
  }
}

std::pair<Atlas::ChartIterator, Atlas::ChartIterator> Atlas::charts ( void ) const {
  return std::make_pair ( charts_ . begin (), charts_ . end () );
}

Atlas::Chart & Atlas::chart ( size_type chart_id ) {
  return charts_ [ chart_id ];
}

const Atlas::Chart & Atlas::chart ( size_type chart_id ) const {
  return charts_ . find ( chart_id ) -> second;
}

void Atlas::clear ( void ) {
  charts_ . clear ();
  update_csum_ ();
}


// // Could be done differently, with some error checking (just in case)
// // here chart_ge, chart_id are known
// // atlas_ge is computed
inline Atlas::GridElement Atlas::Chart_to_Atlas_GridElement_ ( const GridElement & chart_ge, const size_type & chart_id ) const {
  boost::unordered_map < size_type, Chart >::const_iterator it;
  // Find the correct Chart 
  size_type indexchart;
  indexchart = 0;
  for ( it=charts_.begin(); it!=charts_.end(); ++it ) { 
    if ( it -> first == chart_id )  { 
      if ( indexchart == 0 ) {
        return chart_ge;
      } else {
        return chart_ge + csum_ [ indexchart - 1];  
      }
    }
    ++indexchart;
  }
  std::cout << "Invalid input to Chart_to_Atlas_GridElement_\n";
  abort ();
  return 0;
}

// Could be done differently, with some error checking (just in case)
// here chart_ge, chart_id are NOT known
// atlas_ge is provided
inline std::pair < Atlas::size_type, Atlas::GridElement > Atlas::Atlas_to_Chart_GridElement_ ( const GridElement & atlas_ge ) const {
  // DEBUG
  //std::cout << "DEGUG Atlas::Atlas_to_Chart_GridElement_ Size of atlas = " << size () << "\n";
  //std::cout << "DEBUG Atlas::Atlas_to_Chart_GridElement_ Number of charts = " << charts_ . size () << "\n";
  // END DEBUG

  // Find the correct Chart 
  boost::unordered_map < size_type, Chart >::const_iterator it;
  size_type indexchart;
  indexchart = 0;
  for ( it=charts_.begin(); it!=charts_.end(); ++it ) { 
    if ( atlas_ge < csum_ [ indexchart ] )  { 
      break; 
    }
    ++indexchart;
  }
  if ( indexchart == charts_ . size () ) {
    std::cout << "Atlas::Atlas_to_Chart_GridElement_ -- input out of range.\n";
    throw 1;
  }
  std::pair < size_type, GridElement > output;
  if ( atlas_ge >= csum_ [ 0 ] ) {
    output . second = atlas_ge - csum_ [ indexchart - 1 ];
  } else { 
    output . second = atlas_ge;
  }
  output . first = it -> first;

  return output;
}

#endif