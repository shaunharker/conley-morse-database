#include <database/structures/XMLExporter.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <sstream>

using namespace chomp;

static std::string BoundsToString(const std::vector<double>& vector) {
  using boost::adaptors::transformed;
  using boost::lexical_cast;
  return boost::join(vector | transformed(lexical_cast<std::string, double>), " ");
}
      
static void PutRect(boost::property_tree::ptree* pt, const Rect& rect) {
  pt->put("box.dimension", rect.lower_bounds.size());
  pt->put("box.lowerBounds", BoundsToString(rect.lower_bounds));
  pt->put("box.upperBounds", BoundsToString(rect.upper_bounds));
          
}

static void PutGridElementRecord(
    boost::property_tree::ptree* pt,
    const GridElementRecord& rect) {
  pt->put("box.dimension", rect.lower_bounds_.size());
  pt->put("box.lowerBounds", BoundsToString(rect.lower_bounds_));
  pt->put("box.upperBounds", BoundsToString(rect.upper_bounds_));
}

XMLExporter::XMLExporter() {
  pt.add("conleyMorseDatabase", "");
  pt.add("conleyMorseDatabase.model", "");
  pt.add("conleyMorseDatabase.computationConfig", "");
  pt.add("conleyMorseDatabase.boxRecords", "");
  pt.add("conleyMorseDatabase.clutchRecords", "");
  pt.add("conleyMorseDatabase.conleyRecords", "");
}

void XMLExporter::Config(const Configuration& config) {
  using namespace boost::property_tree;
  ptree& model_node = pt.get_child("conleyMorseDatabase.model");
  model_node.put("name", config.MODEL_NAME);
  model_node.put("description", config.MODEL_DESC);

  ptree& param_space_node =
      pt.put_child("conleyMorseDatabase.computationConfig.parameterSpace",
                   ptree());
  PutRect(&param_space_node, config.PARAM_BOUNDS);
  param_space_node.put("subdivisionDepth", config.PARAM_SUBDIV_DEPTH);

  ptree& phase_space_node =
      pt.put_child("conleyMorseDatabase.computationConfig.phaseSpace",
                   ptree());
  PutRect(&phase_space_node, config.PHASE_BOUNDS);
  phase_space_node.put("subdivision.minDepth", config.PHASE_SUBDIV_MIN);
  phase_space_node.put("subdivision.maxDepth", config.PHASE_SUBDIV_MAX);
  phase_space_node.put("subdivision.limit", config.PHASE_SUBDIV_LIMIT);
}

std::string StringOfPairInt(const std::pair<int, int>& pair) {
  return (boost::format("%1% %2%") % pair.first % pair.second).str();
}

void XMLExporter::AddParameterRecord(const ParameterBoxRecord& record) {
  using namespace boost::property_tree;
  typedef std::pair<int, int> order;
  
  ptree& param_box_record_node =
      pt.add_child("conleyMorseDatabase.boxRecords.parameterBoxRecord",
                   ptree());
  param_box_record_node.put("id", record.id_);
  PutGridElementRecord(&param_box_record_node, record.ge_);

  BOOST_FOREACH (const order &order, record.partial_order_) {
    param_box_record_node.add("partialOrders.order",
                              StringOfPairInt(order));
  }
}

void XMLExporter::AddClutchingRecord(const ClutchingRecord& record) {
  using namespace boost::property_tree;
  typedef std::pair<int, int> pair;
  
  ptree& clutching_record_node =
      pt.add_child("conleyMorseDatabase.clutchRecords.clutchingRecord", ptree());
  clutching_record_node.put("id1", record.id1_);
  clutching_record_node.put("id2", record.id2_);
  
  BOOST_FOREACH (const pair &pair, record.clutch_) {
    clutching_record_node.add("clutch.pair", StringOfPairInt(pair));
  }
}

std::string StringOfMatrix(const SparseMatrix<Ring>& matrix) {
  typedef SparseMatrix<Ring>::size_type size_type;
  size_type number_of_rows = matrix.number_of_rows();
  size_type number_of_columns = matrix.number_of_columns();
  std::stringstream ret("");
  
  for (int i=0; i<number_of_rows; i++) {
    for (int j=0; j<number_of_columns; j++) {
      if (!(i==0 && j==0))
        ret << " ";
      ret << matrix.read(i,j);
    }
  }
  return ret.str();
}

void XMLExporter::AddConleyRecord(const ConleyRecord& record) {
  using namespace boost::property_tree;
  
  ptree& conley_record_node =
      pt.add_child("conleyMorseDatabase.conleyRecords.conleyRecord", ptree());

  conley_record_node.put("id", StringOfPairInt(record.id_));

  for (unsigned int dim=0; dim < record.ci_.data().size(); dim++) {
    ptree& homology_map_node =
        conley_record_node.add_child("homologyMap", ptree());
    const SparseMatrix<Ring>& matrix = record.ci_.data()[dim];
    
    homology_map_node.put("dimension", dim);
    homology_map_node.put("numberOfGenerators", matrix.number_of_rows());
    homology_map_node.put("matrix", StringOfMatrix(matrix));
  }
}

void XMLExporter::LoadFromDatabase(const Database& database) {
  boost::for_each(database.box_records(),
                  boost::bind(&XMLExporter::AddParameterRecord, *this, _1));
  boost::for_each(database.clutch_records(),
                  boost::bind(&XMLExporter::AddClutchingRecord, *this, _1));
  boost::for_each(database.conley_records(),
                  boost::bind(&XMLExporter::AddConleyRecord, *this, _1));
}
    
std::string XMLExporter::ExportString() {
  std::stringstream output;
  ExportToStream(&output);
  return output.str();
}

void XMLExporter::ExportToStream(std::ostream* output) {
  using namespace boost::property_tree;
  static const int indent = 2;
  write_xml(*output, pt, xml_writer_make_settings(' ', indent));
}
