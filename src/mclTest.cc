// #include "testPNL.h"
#include "mcl.h"
#include "mclOntology.h"
#include "mclFrame.h"
#include "oNodes.h"
#include "linkFactory.h"
#include "configManager.h"
#include "mclLogger.h"

#include "glueFactory.h"

#include "mclProperties.h"

#include <iostream>
using namespace std;

string mclt_fail="pass"; // the default value is pass...
bool testPVs();
bool testHII();
bool testInteractive();
void queryTest();
bool testConnectivity(mclFrame* f);

int main(int argc, char **argv) {

  // testBV();

  if (argc > 1)
    cfg::loadConfig(argv[1]);
  else
    cfg::loadConfig("default");

  glueFactory::clearAutoGlue();
  glueFactory::addAutoGlue(HG_GLUE_IDENTIFIER);
  glueFactory::addAsDefaultGlue(SMILE_GLUE_IDENTIFIER);

  *mclLogger::mclLog << "*** TEST FOR MULTIPLE FRAMES/MCLs." << endl;

  mcl         *m = new mcl();
  mclFrame    *f = new mclFrame(m,NULL,NULL);

  mcl         *m2 = new mcl();
  mclFrame    *f2= new mclFrame(m2,(void*)0x1,NULL);
  mclFrame    *f3= new mclFrame(m2,(void*)0x2,NULL);
  
  // property tests

  *mclLogger::mclLog << "*** TEST HARNESS FOR PROPERTY VECTOR CODE" << endl;

  if (!testPVs()) {
    *mclLogger::mclLog << "error: " << mclt_fail << endl;
    exit(-1);
  }
  else *mclLogger::mclLog << "passed." << endl;

  *mclLogger::mclLog << endl << "*** TESTING NETWORK FOR REACHABILITY" << endl;
  if (!testConnectivity(f))
    exit(-1);
  else *mclLogger::mclLog << "passed." << endl;

  //   if (!testNodes(f))
  //     exit(-1);
  //   else *mclLogger::mclLog << "passed." << endl;

  //   *mclLogger::mclLog << "*** TESTING FOR OPENPNL EVIDENCE CODE" << endl;

  //   if (!testEvidence()) {
  //     *mclLogger::mclLog << "failed during evidence test." << endl;
  //     exit(-1);
  //   }
  //   else *mclLogger::mclLog << "passed." << endl;

  *mclLogger::mclLog << "*** ENTERING QUERY TEST MODE" << endl;
  
  queryTest();

  *mclLogger::mclLog << "*** TESTING HII/ACTIVATE-DIRECT CODE" << endl;

  if (!testHII()) {
    *mclLogger::mclLog << "failed during HII test." << endl;
    exit(-1);
  }  
  else *mclLogger::mclLog << "passed." << endl;

  *mclLogger::mclLog << "*** TESTING INTERACTIVE REPAIR CODE" << endl;

  if (!testInteractive()) {
    *mclLogger::mclLog << "failed during interactive test." << endl;
    exit(-1);
  }  
  else *mclLogger::mclLog << "passed." << endl;

  *mclLogger::mclLog << "all tests passed." << endl;

  return 0;
}

bool testPVs() {
  glueFactory::printGlueConfig(&cout);
  *mclLogger::mclLog << "supposed to be " << PCI_MAX << " properties, " << CRC_MAX
       << " capabilities, total PV length=" << PC_VECTOR_LENGTH << endl;
  for (int i=0;i<PC_VECTOR_LENGTH;i++) {
    printf("%2d %24s %d\n",
	   i,
	   mclPropertyVector::pnames[i].c_str(),
	   mclPropertyVector::defaults[i]);
    if (mclPropertyVector::pnames[i].length() == 0) {
      *mclLogger::mclLog << "EMPTY STRING IN PROPERTY VECTOR DEFAULTS..." 
			 << "PCV LENGTH = " << dec << PC_VECTOR_LENGTH
			 << endl;
      mclt_fail="mismatch in pv defaults";
      return false;
    }
  }
  return true;
}

bool testHII() {
  glueFactory::printGlueConfig(&cout);
  mcl         *m = new mcl();
  mclFrame    *f = new mclFrame(m,NULL,NULL);
  string    hiin = "sensorVerifiedBroken";
  string     dnn = "resetSensor";
  mclNode* hii = f->findNamedNode(hiin);
  mclNode*  dn = f->findNamedNode(dnn);
  *mclLogger::mclLog << " ~> default HII(" << hiin << ")" << endl;
  hii->dumpEntity(mclLogger::mclLog);
  *mclLogger::mclLog << " ~> diagnostic node(" << dnn << ")" << endl;
  dn->dumpEntity(mclLogger::mclLog);
  m->signalHII(hiin,f);
  *mclLogger::mclLog << " ~> default HII(" << hiin << ")" << endl;
  hii->dumpEntity(mclLogger::mclLog);
  *mclLogger::mclLog << " ~> diagnostic node(" << dnn << ")" << endl;
  dn->dumpEntity(mclLogger::mclLog);
  return true;
}

bool testInteractive() {
  glueFactory::printGlueConfig(&cout);
  mcl         *m = new mcl();
  mclFrame    *f = new mclFrame(m,NULL,NULL);
  f->dbg_forceFrameState("runSensorDiagnostic",FRAME_RESPONSE_ISSUED);

  string    fpn = "sensorVerifiedBroken";
  string    fnn = "sensorVerifiedWorking";
  string    dnn = "resetSensor";
  mclNode*  pn = f->findNamedNode(fpn);
  mclNode*  nn = f->findNamedNode(fnn);
  mclNode*  dn = f->findNamedNode(dnn);

  pn->dumpEntity(mclLogger::mclLog);
  nn->dumpEntity(mclLogger::mclLog);
  dn->dumpEntity(mclLogger::mclLog);

  *mclLogger::mclLog << "providing feedback..." << endl;
  m->processHostFeedback(true,f);

  pn->dumpEntity(mclLogger::mclLog);
  nn->dumpEntity(mclLogger::mclLog);
  dn->dumpEntity(mclLogger::mclLog);

  return true;
}

void queryTest() {
  glueFactory::printGlueConfig(&cout);
  bool go = true;
  mcl         *m = new mcl();
  mclFrame    *f = new mclFrame(m,NULL,NULL);
  char buff[128];
  buff[0]='\0';
  cout << "type a blank line to exit" << endl;
  while (go) {
    cout << "mclTest :> ";
    cin.getline(buff,127);
    if ((strlen(buff)==0) || (strcmp(buff,"quit")==0)) {
      go=false;
    }
    else {
      tokenMachine tm(buff);
      string cmd = tm.nextToken();
      if (cmd == "help") {
	cout << " assert <nodename>" << endl;
	cout << " unset  <nodename>" << endl;
	cout << " show   <nodename>" << endl;
	cout << " cpt    <nodename>" << endl;
	cout << "type a blank line to query mode..." << endl;
      }
      else {
	if (!tm.moreTokens())
	  cout << " all commands (except help) require an argument" << endl;
	else {
	  string nn = tm.nextToken();
	  mclNode* pn = f->findNamedNode(nn);
	  if (pn == NULL) 
	    cout << " not a legal node name." << endl;
	  else {
	    if (cmd == "assert") {
	      pn->set_evidence(true);
	      pn->dumpEntity(mclLogger::mclLog);
	    }
	    else if (cmd == "unset") {
	      pn->set_evidence(false);
	      pn->dumpEntity(mclLogger::mclLog);
	    }
	    else if (cmd == "show")
	      pn->dumpEntity(mclLogger::mclLog);
	    // else if (cmd == "cpt") {
	    // textFunctions::dumpTokArrStr(pn->cpt());
	    // }
	    else cout << " unknown command" << endl;
	  }
	}
      }
    }
  }
}

bool testConnectivity(mclFrame* f) {
  glueFactory::printGlueConfig(&cout);
  list<mclNode *> unvisited,visited,spent;
  
  for (nodeList::iterator ini = f->getIndicationCore()->firstNode();
       ini != f->getIndicationCore()->endNode();
       ini++) {
    unvisited.push_back(*ini);
  }
  for (nodeList::iterator fni = f->getFailureCore()->firstNode();
       fni != f->getFailureCore()->endNode();
       fni++) {
    unvisited.push_back(*fni);
  }
  for (nodeList::iterator rni = f->getResponseCore()->firstNode();
       rni != f->getResponseCore()->endNode();
       rni++) {
    unvisited.push_back(*rni);
  }

  *mclLogger::mclLog << "VISITED(" << visited.size() << ")" << endl;
  *mclLogger::mclLog << "UNVISITED(" << unvisited.size() << ")" << endl;

  visited.push_back(unvisited.front());
  unvisited.pop_front();

  *mclLogger::mclLog << "VISITED(" << visited.size() << ")" << endl;
  *mclLogger::mclLog << "UNVISITED(" << unvisited.size() << ")" << endl;
  
  *mclLogger::mclLog << "PRIMARY CLIQUE: " << visited.front()->entityBaseName() << ",";

  while (!unvisited.empty()) {
    if (visited.empty()) {
      *mclLogger::mclLog << endl << endl
	   << "*** Ontologies are not fully linked." << endl; 
      *mclLogger::mclLog << "REMAINING: ";
      while (!unvisited.empty()) {
	*mclLogger::mclLog << unvisited.front()->entityBaseName() << ",";
	unvisited.pop_front();
      }
      *mclLogger::mclLog << endl;
      return false;
    }
    else {
      mclNode* thisNode = visited.front();
      visited.pop_front();
      spent.push_back(thisNode);

      for (llIterator inLI = thisNode->inLink_begin();
	   inLI != thisNode->inLink_end();
	   inLI++) {
	// *mclLogger::mclLog << "<";
	unvisited.remove((*inLI)->sourceNode());
	if ((find(spent.begin(),spent.end(),(*inLI)->sourceNode())==spent.end()) &&
	    (find(visited.begin(),visited.end(),(*inLI)->sourceNode())==visited.end())) {
	  visited.push_back((*inLI)->sourceNode());
	  *mclLogger::mclLog << "                "
	       << (*inLI)->sourceNode()->entityBaseName() << "," << endl;
	}
      }

      for (llIterator outLI = thisNode->outLink_begin();
	   outLI != thisNode->outLink_end();
	   outLI++) {
	// *mclLogger::mclLog << ">";
	unvisited.remove((*outLI)->destinationNode());
	if ((find(spent.begin(),spent.end(),(*outLI)->destinationNode())==spent.end()) &&
	    find(visited.begin(),visited.end(),(*outLI)->destinationNode()) == visited.end()) {
	  visited.push_back((*outLI)->destinationNode());
	  *mclLogger::mclLog << "                "
	       << (*outLI)->destinationNode()->entityBaseName() << "," << endl;
	}
      }

    }
  }

  *mclLogger::mclLog << endl << endl
		     << "*** Ontologies appear fully connected." << endl;
  return true;
}
