
#include <database/structures/XMLExporter.h>
#include <database/program/Configuration.h>

#include <string>
#include <iostream>

std::string usage =
    "usage: XMLExporter map-directory\n";

void PrintUsageAndExit() {
  std::cout << usage << std::endl;
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    PrintUsageAndExit();

  std::string mapdir(argv[1]);
  
  Configuration config;
  config.loadFromFile(mapdir.c_str());
  
  Database database;
  database.load((mapdir + "/database.cmdb").c_str());

  XMLExporter exporter;
  exporter.Config(config);
  exporter.LoadFromDatabase(database);

  std::ofstream output((mapdir + "/database.xml").c_str());
  exporter.ExportToStream(&output);
  
  return 0;
}

