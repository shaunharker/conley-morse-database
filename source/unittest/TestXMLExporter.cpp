#include <database/structures/XMLExporter.h>
#include <database/program/Configuration.h>

#include <cppcutter.h>

namespace TestXMLExporter {
using namespace chomp;

std::string ReadTextFromFile(std::string filename) {
  std::ifstream input(filename);
  std::string retval;
  std::string s;
  while (std::getline(input, s)) {
    retval += s;
    retval += "\n";
  }
  return retval;
}

void test_EmptyDatabase() {
  XMLExporter exporter;
  std::string xml_for_empty_database = ReadTextFromFile("empty_database.xml");
  
  cppcut_assert_equal(xml_for_empty_database, exporter.ExportString());
}

void test_Config() {
  XMLExporter exporter;
  Configuration config;
  std::ifstream config_file("config.xml");
  config.LoadFromStream(&config_file);
  exporter.Config(config);

  std::string xml_with_only_config =
      ReadTextFromFile("only_config_database.xml");
  cppcut_assert_equal(xml_with_only_config, exporter.ExportString());

  std::stringstream ss;
  exporter.ExportToStream(&ss);
  cppcut_assert_equal(xml_with_only_config, ss.str());
}

void test_ParameterRecord() {
  using std::make_pair;
  XMLExporter exporter;
  ParameterBoxRecord records[2];

  records[0].id_ = 3;
  records[0].ge_.lower_bounds_ = std::vector<double>{ -0.02, 0.00 };
  records[0].ge_.upper_bounds_ = std::vector<double>{ -0.01, 0.01 };
  records[0].num_morse_sets_ = 5;
  records[0].partial_order_ = std::list<std::pair<int, int> > {
    make_pair(0, 3), make_pair(2, 3), make_pair(3, 4),
  };

  records[1].id_ = 12;
  records[1].ge_.lower_bounds_ = std::vector<double>{ -0.01, -0.125 };
  records[1].ge_.upper_bounds_ = std::vector<double>{ 0.00, 0.125 };
  records[1].num_morse_sets_ = 5;
  records[1].partial_order_ = std::list<std::pair<int, int> > {
    make_pair(0, 2), make_pair(2, 3), make_pair(1, 4),
  };

  for (int i=0; i<2; ++i)
    exporter.AddParameterRecord(records[i]);

  std::string xml_with_only_param_records =
      ReadTextFromFile("only_parameter_records.xml");
  cppcut_assert_equal(xml_with_only_param_records, exporter.ExportString());
}

void test_ClutchRecord() {
  using std::make_pair;
  XMLExporter exporter;
  ClutchingRecord records[2];

  records[0].id1_ = 3;
  records[0].id2_ = 19;
  records[0].clutch_ = std::list<std::pair<int, int> > {
    make_pair(32, 12), 
  };

  records[1].id1_ = 3;
  records[1].id2_ = 101;
  records[1].clutch_ = std::list<std::pair<int, int> > {
    make_pair(32, 12), make_pair(0, 3),
  };

  BOOST_FOREACH (const ClutchingRecord& record, records) {
    exporter.AddClutchingRecord(record);
  }

  std::string xml_with_only_clutch_records =
      ReadTextFromFile("only_clutch_records.xml");
  cppcut_assert_equal(xml_with_only_clutch_records, exporter.ExportString());
}

void test_ConleyRecord() {
  XMLExporter exporter;
  ConleyRecord record;
  
  record.id_ = std::make_pair(4, 12);
  record.ci_.data().push_back(SparseMatrix<Ring>()); // H_0 = 0
  
  record.ci_.data().push_back(SparseMatrix<Ring>(1, 1)); // H_1 = Z/2Z
  record.ci_.data()[1].write(0, 0, Zp<2>(1));      // matrix = id

  record.ci_.data().push_back(SparseMatrix<Ring>(2, 2)); // H_2 = (Z/2Z)^2
  record.ci_.data()[2].write(0, 1, Zp<2>(1));
  
  exporter.AddConleyRecord(record);
  cppcut_assert_equal(ReadTextFromFile("only_conley_record.xml"),
                      exporter.ExportString());
                                       
}

}
