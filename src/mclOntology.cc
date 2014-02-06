#include "output.h"
#include "mclOntology.h"
#include "mclFrame.h"
#include "mcl.h"
#include "../include/umbc/logger.h"
#include "mclLogging.h"

using namespace metacog;

const string THIS_FILE = "[mclOntology]::";

mclNode *mclOntology::findNamedNode(string n) {
  for (list<mclNode *>::iterator ni = nodes.begin();
       ni != nodes.end();
       ni++) {
    if ((*ni)->matchesName(n))
      return (*ni);
  }
  return NULL;
}

mcl *mclOntology::MCL() { return myFrame()->MCL(); };

void mclOntology::addNode(mclNode *m) { 
  nodes.push_back(m); 
} 

void mclOntology::dumpEntity(ostream *strm) {
  dumpOntology(strm,this);
}

// void mclOntology::update() {
//   for (list<mclNode *>::iterator ni = nodes.begin();
//        ni != nodes.end();
//        ni++) {
//     (*ni)->ensure_();
//   }
// }

mclNode* mclOntology::maxPNode() {
  umbc::uLog::annotate(MCLA_MSG,THIS_FILE+" computing maxP node...");
  double   maxP=-1;
  mclNode* maxN=NULL;
  for (list<mclNode *>::iterator ni = nodes.begin();
       ni != nodes.end();
       ni++) {
    if ((*ni)->p_true() > maxP) {
      maxP=(*ni)->p_true();
      maxN=(*ni);
    }
  }
  return maxN;
}

void mclOntology::autoActivateProperties(mclPropertyVector& mpv) {
  for (nodeList::iterator nli = firstNode();
       nli != endNode();
       nli++) {
    mclHostProperty* tn = dynamic_cast<mclHostProperty*>(*nli); 
    if (tn != NULL) {
      string ooc;
      if (mpv.testProperty(tn->propCode(),PC_YES)) {
	// was (MCL()->testProperty(tn->propCode(),PC_YES))
	ooc = "off";
	tn->set_evidence(true);
      }	
      else {
	ooc = "off";
	tn->set_evidence(false);
      }
      sprintf(umbc::uLog::annotateBuffer,
	      "[mcl/ontology]::checking %s for prop (0x%x) activation...%s",
	      (*nli)->entityName().c_str(),
	      tn->propCode(),
	      ooc.c_str());
      umbc::uLog::annotateFromBuffer(MCLA_DBG);
    }
  }
}
