#ifndef MCL_ONTOLOGY_READER_HEADER
#define MCL_ONTOLOGY_READER_HEADER

#include "mclFrame.h"
#include <string>

using namespace std;

namespace metacog {
  typedef map<string,mclOntology*> ontologymap_t;

  namespace ontology_reader {
    ontologyVector* read_ontologies(string basename,mclFrame* f);
    bool read_ontologies_from_file(const char * f_name,
				   ontologymap_t& oif);
    string basename2filename(string basename);
    bool   legit_ontologyname(string basename);

  };

};

#endif
