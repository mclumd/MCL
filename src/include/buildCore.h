#include "mclOntology.h"
#include <vector>

/** \file
 *  \brief code for constructing the ontologies.
 */

namespace metacog {
  
  class mclFrame;
  
  typedef vector<mclOntology *> ontologyVector;
  
  ontologyVector *mclGenerateOntologies(mclFrame *f);

};
