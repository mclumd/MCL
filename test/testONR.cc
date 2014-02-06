#include "ontology_reader.h"
#include <string>
#include <iostream>

using namespace std;

int main(int argc,char ** argv) {
  if (argc < 2) {
    cerr << "usage: " << argv[0] << " <config_file>" << endl;
    return 1;
  }
  metacog::ontologymap_t mymap;
  metacog::ontology_reader::read_ontologies_from_file(argv[1],mymap);
}
