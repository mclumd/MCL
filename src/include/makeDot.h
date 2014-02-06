#include "mclOntology.h"

using namespace std;

namespace metacog {

  void writeOntology(mclOntology *m);
  void writeOntology(string filename,mclOntology *m);
  void writeOntology(string filename,mclOntology *m,string path);
  
  void writeAll(mclFrame *f);
  void writeAll(string filename,mclFrame *f);

};
