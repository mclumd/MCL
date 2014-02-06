#include "testPNL.h"
#include "mcl.h"
#include "mclOntology.h"
#include "oNodes.h"
#include "linkFactory.h"
#include "configManager.h"
#include "mclLogger.h"

#include "glueFactory.h"

#include <iostream>
#include <list>
using namespace std;

#ifndef COMPILE_FOR_LINKING
int main(int argc, char **argv) {
  cfg::loadConfig("default");
  mcl         *m = new mcl();
  mclFrame    *f = new mclFrame(m);

  *mclLogger::mclLog << "*** TESTING PNL NETWORK FOR NODE RETRIEVAL AND P TABLES" << endl;
  f->getIndicationCore()->findNamedNode("resource")->dumpEntity(mclLogger::mclLog);
  f->getFailureCore()->findNamedNode("modelError")->dumpEntity(mclLogger::mclLog);
  f->getResponseCore()->findNamedNode("runSensorDiagnostic")->dumpEntity(mclLogger::mclLog);

  *mclLogger::mclLog << endl << "*** TESTING PNL NETWORK FOR REACHABILITY" << endl;
  if (!testConnectivity(f))
    exit(-1);

  *mclLogger::mclLog << endl << "*** TESTING PNL NETWORK FOR REACHABILITY" << endl;
  if (!testConnectivity(f))
    exit(-1);

  if (!testNodes(f))
    exit(-1);

  // *mclLogger::mclLog << endl << "*** TESTING PNL NETWORK FOR MISSING / UNIFORM PRIORS" << endl;

  *mclLogger::mclLog << "*** TESTING FOR OPENPNL EVIDENCE CODE" << endl;

  if (!testEvidence()) {
    *mclLogger::mclLog << "failed during evidence test." << endl;
    exit(-1);
  }
  

  return 0;
}
#endif

bool testNodes(mclFrame* f) {
  if (!testNode(f,"spatial")) return(false);
  if (!testNode(f,"moveFailure")) return(false);
  if (!testNode(f,"sensorVerifiedBroken")) return(false);
  if (!testNode(f,"sensorStuck")) return(false);
  if (!testNode(f,"plantResponse")) return(false);
  if (!testNode(f,"resetEffector")) return(false);  
  return true;
}

bool testNode(mclFrame* f,string node) {
  
  *mclLogger::mclLog << "MPD(" << node << ")= ";
  double x[2];
  f->findNamedNode(node)->mpd(x);
  *mclLogger::mclLog << x[0] << "t " << x[1] << "f" << endl;  
  
  return true;
  
}

bool testEvidence() {
  glueFactory::addAutoGlue(PNL_GLUE_IDENTIFIER);
  mcl         *mx = new mcl();
  mclFrame    *fx = new mclFrame(mx);

  mclNode *m1 = ((mclNode*)(fx->findNamedNode("plantResponse")));
  mclNode *m2 = ((mclNode*)(fx->findNamedNode("resetEffector")));
  mclNode *m3 = ((mclNode*)(fx->findNamedNode("resetSensor")));
  cout << "PLNT(1): " << m1->mpv() << "(" << m1->p_true() << ")" << endl;
  cout << "EFCT(1): " << m2->mpv() << "(" << m2->p_true() << ")" << endl;
  cout << "SENS(1): " << m3->mpv() << "(" << m3->p_true() << ")" << endl;
  m2->set_evidence(true);
  cout << "PLNT(2): " << m1->mpv() << "(" << m1->p_true() << ")" << endl;
  cout << "EFCT(2): " << m2->mpv() << "(" << m2->p_true() << ")" << endl;
  cout << "SENS(2): " << m3->mpv() << "(" << m3->p_true() << ")" << endl;
  m3->set_evidence(false);
  cout << "PLNT(3): " << m1->mpv() << "(" << m1->p_true() << ")" << endl;
  cout << "EFCT(3): " << m2->mpv() << "(" << m2->p_true() << ")" << endl;
  cout << "SENS(3): " << m3->mpv() << "(" << m3->p_true() << ")" << endl;

  // fx->dumpEntity(&cout);
  return true;
}

