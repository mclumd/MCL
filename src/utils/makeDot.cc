#include "makeDot.h"
#include "mcl.h"
#include "mclFrame.h"
#include "oNodes.h"
#include "linkFactory.h"
#include "../../include/umbc/settings.h"
#include "../../include/umbc/logger.h"

#include <fstream>
#include <iostream>
#include <list>

using namespace std;
using namespace metacog;

void metacog::writeOntology(mclOntology *m) {
  writeOntology(m->entityBaseName()+".dot",m);
}

void metacog::writeOntology(string filename,mclOntology *m) {
  string is = ontology_configurator::configPath(m,filename);
}

void metacog::writeOntology(string filename,mclOntology *m,string path) {
  string is = path+"/"+filename;
  string os = is+".bak";
  cout << "moving old dot file to " << os << endl;
  ifstream i(is.c_str(),ios_base::in|ios_base::binary);
  ofstream o(os.c_str(),ios_base::out|ios_base::binary);
  o << i.rdbuf();  

  cout << "writing new dot file to " << is << endl;
  ofstream no(is.c_str(),ios_base::out);
  no << "digraph " << m->entityBaseName() << " {" << endl;
  no << "  size=\"8,10\"" << endl;

  // start with nodes...
  for (nodeList::iterator ni1 = m->firstNode();
       ni1 != m->endNode();
       ni1++) {
    no << "  " << (*ni1)->dotDescription() << endl;
  }

  // then add links...
  for (nodeList::iterator ni2 = m->firstNode();
       ni2 != m->endNode();
       ni2++) {
    for (llIterator ogli = (*ni2)->inLink_begin();
	 ogli != (*ni2)->inLink_end();
	 ogli++) {
      no << "  " << (*ogli)->dotDescription() << endl; 

    }
  }

  no << "}" << endl;
}

void metacog::writeAll(mclFrame *f) {
  writeAll("mclOntologies.dot",f);
}

// void metacog::writeAll(mclFrame *f,ontologymap_t& oif) {
// }

void metacog::writeAll(string filename,mclFrame *f) {
  string is = ontology_configurator::configPath(f,filename);
  string os = is+".bak";
  cout << "moving old dot file to " << os << endl;
  ifstream i(is.c_str(),ios_base::in|ios_base::binary);
  ofstream o(os.c_str(),ios_base::out|ios_base::binary);
  o << i.rdbuf();  

  cout << "writing new dot file to " << is << endl;
  ofstream no(is.c_str(),ios_base::out);
  no << "digraph MCL {" << endl;
  no << "  size=\"16,20\"" << endl;
  // no << "  rankdir=LR" << endl;
  no << "  compound=true" << endl;

  // start with nodes...
  for (int oi = 0;oi < 3;oi++) {
    mclOntology* m = f->getCoreOntology(oi);
    no << "  subgraph cluster_" << m->entityBaseName() << " {" << endl;
    no << "    label = \"" << m->entityBaseName() << "\";" << endl;
    for (nodeList::iterator ni1 = m->firstNode();
	 ni1 != m->endNode();
	 ni1++) {
      // cout << "[writing " << (*ni1)->className() << ":" 
      // << (*ni1)->entityBaseName() << "]" << endl;
      no << "    " << (*ni1)->dotDescription() << endl;
    }
    no << "  }" << endl;
  }

  // then add links...
  for (int oi = 0;oi < 3;oi++) {
    mclOntology* m = f->getCoreOntology(oi);
    for (nodeList::iterator ni2 = m->firstNode();
	 ni2 != m->endNode();
	 ni2++) {
      for (llIterator ogli = (*ni2)->inLink_begin();
	   ogli != (*ni2)->inLink_end();
	   ogli++) {
	no << "  " << (*ogli)->dotDescription() << endl; 
	
      }
    }
  }

  no << "}" << endl;
}

#ifdef EXECUTABLE
#include "ontology_reader.h"

int main(int argc, char **argv) {
  metacog::ontologymap_t myonts;
  umbc::uLog::setAnnotateMode(umbc::UMBCLOG_XTERM);
  if (argc < 2)
    umbc::uLog::annotate(umbc::UMBCLOG_ERROR,
			 "usage: "+(string)argv[0]+" <ontologyset>");
  else {
    string bn = (string)argv[1];
    string fn = metacog::ontology_reader::basename2filename(bn);
    umbc::uLog::annotate(umbc::UMBCLOG_MSG,"reading: "+fn);
    metacog::ontology_reader::read_ontologies_from_file(fn.c_str(),myonts);
    umbc::uLog::annotate(umbc::UMBCLOG_MSG,
			 "ontology reader done?");
    umbc::settings::quiet = true;
    umbc::settings::setSysProperty("mcl.ignoreMissingFrameref",true);
    for (ontologymap_t::iterator i = myonts.begin();
	 i != myonts.end();
	 i++) {
      writeOntology((string)argv[1]+"_"+i->first+".dot",i->second,"docs/ontologies/");
    }
  }
  // writeAll(f);

}
#endif
