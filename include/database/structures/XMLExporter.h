#include <database/structures/Database.h>
#include <database/program/Configuration.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/utility.hpp>
#include <ostream>

class XMLExporter: boost::noncopyable {
public:
  XMLExporter();
  void Config(const Configuration& config);
  void AddParameterRecord(const ParameterBoxRecord& record);
  void AddClutchingRecord(const ClutchingRecord& record);
  void AddConleyRecord(const ConleyRecord& record);
  void LoadFromDatabase(const Database& database);
  std::string ExportString();
  void ExportToStream(std::ostream* output);
private:
  boost::property_tree::ptree pt;
};

