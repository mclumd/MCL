#include "mclFrame.h"
#include "ontologyRecord.h"
#include "ontology_reader.h"
#include "oNodes.h"
#include "mcl.h"
#include "makeDot.h"
#include "mclLogging.h"
#include "../include/umbc/settings.h"
#include "../include/umbc/text_utils.h"
#include "../include/umbc/exceptions.h"

const string THIS_FILE = "[mclFrame]:: ";

using namespace metacog;

resRefType mclFrame::FRAME_ID_COUNTER = 1;

mclFrame::mclFrame(mcl *m,mclFrameEntryVector& fev) : 
  mclEntity(),core(NULL),myMCL(m),state(FRAME_NEW),
  active_response(NULL),veg_key(fev.vEG),veg_parent_key(fev.parentKey()),
  refails(0),successes(0),last_update(0),dirty(false),
  default_glue_key(glueFactory::getDefaultGlueKey()) { 
  // nervous about initFrame() as it didn't seem to be there....
  initFrame();
  frame_id = (FRAME_ID_COUNTER++);
  // the frame gets a copy of the property vector associated with either...
  // ... the violated expectation group
  if (veg_key) {
    if (m->getExpGroup(veg_key,true)) 
      m->getExpGroup(veg_key,true)->getPV()->copyValues(myPV);
    else {
      umbc::exceptions::signal_exception("creating mclFrame with missing but supplied VEG key.");
    }
  }
  // ... or the default MCL property vector (in the case of an HII)
  else {
    // this will happen with HIIs
    m->getDefaultPV()->copyValues(myPV);
  }
  add_new_fev(fev);
  touch();
}

void mclFrame::touch() { last_update = MCL()->tickNumber(); }

void mclFrame::add_new_fev(mclFrameEntryVector& fev) {
  string m=fev.describe()+" being added to fev record.";
  log(m);
#ifdef FEVLIST_IS_A_SET
  // this part checks for matches to the FEV in the existing set
  // and exits if there is one
  for (list<mclFrameEntryVector*>::iterator fevi = fevList.begin();
       fevi != fevList.end();
       fevi++) {
    if (fev.equals(*fevi)) return;
  }
#else
  fevList.push_back(fev.clone());
#endif
}

mclNode** mclFrame::allNodes() {
  const int rvn = nodeCount();
  mclNode** rv = new mclNode *[rvn];
  int nc = 0;
  for (int q=0;q<numOntologies();q++) {
    mclOntology* o = getCoreOntology(q);
    for (nodeList::iterator oi = o->firstNode();
	 oi != o->endNode();
	 oi++) {
      rv[nc] = *oi;
      nc++;
    }
  }
  return rv;
}

vector<mclNode*> mclFrame::allNodesV() {
  vector<mclNode*> allMCLnodes;
  for (int oi = 0; oi < 3; oi++) {
    mclOntology* to = getCoreOntology(oi);
    for (nodeList::iterator toni = to->firstNode();
	 toni != to->endNode();
	 toni++) {
      allMCLnodes.push_back(*toni);
    }
  }
  return allMCLnodes;
}

mclNode* mclFrame::findNamedNode(string nodename) {
  mclNode* rv=NULL;
  rv=getIndicationCore()->findNamedNode(nodename);
  if (rv != NULL) return rv;
  rv=getFailureCore()->findNamedNode(nodename);
  if (rv != NULL) return rv;
  rv=getResponseCore()->findNamedNode(nodename);
  return rv;
}

void mclFrame::processFeedback(bool hostReply) {
  touch();
  mclInteractiveResponse* arair = 
    dynamic_cast<mclInteractiveResponse*>(active_response);
  if (arair == NULL) {
    if (active_response == NULL)
      umbc::uLog::annotate(MCLA_ERROR,"[mcl/mclFrame]:: Error ?> failed to process feedback with no active response in Frame");
    else
      umbc::uLog::annotate(MCLA_ERROR,"[mcl/mclFrame]:: Error ?> failed to process feedback for non-interactive node "+active_response->entityName());
  }
  else {
    // this does the feedback part for the node...
    arair->interpretFeedback(hostReply);
    // now update the frame
    setState(FRAME_UPDATED);
    umbc::uLog::annotate(MCLA_MSG,"[mcl/mclFrame]:: feedback attributed to "+active_response->entityName()+" and frame/response are set to UPDATED.");
    set_active_response(NULL);
  }
}

#ifndef NO_DEBUG
void mclFrame::dbg_forceFrameState(string node,int state) {
  mclConcreteResponse* maxnode = 
    dynamic_cast<mclConcreteResponse*>(findNamedNode(node));
  if (maxnode == NULL)
    umbc::exceptions::signal_exception("DBG error:> "+node+" does not exist as concrete response.");
  else {
    setState(FRAME_RESPONSE_ISSUED);
    set_active_response(maxnode);
  }
}
#endif

void mclFrame::setCore(ontologyVector* nv) {
	if (core != NULL) {
		delete core;
		core=NULL;
	}
	core = nv;
}

void mclFrame::initFrame() {
  // core = mclGenerateOntologies(this);
  umbc::uLog::annotate(MCLA_MSG,THIS_FILE+"intializing frame from '"+MCL()->getOntologyBase()+"'");
  setCore(ontology_reader::read_ontologies(MCL()->getOntologyBase(),this));
  glueFactory::autoCreateGlue(this);
  if (umbc::settings::getSysPropertyBool("mcl.autoConfigCPTs",true)) {
    ontology_configurator::apply_config(this);
    // cfg::applyCPTconfig(this);
    // cfg::applyRCconfig(this);
  }
}

////////////////////////////////////////////////////////////////////
// ONTOLOGY STATE STUFF
////////////////////////////////////////////////////////////////////

void mclFrame::record_ostate_if_dirty() {
  if (dirty) {
    dirty=false;
    ontologyRecord* orec = new ontologyRecord_map(this);
    ontology_records.push_back(orec);
  }
}

////////////////////////////////////////////////////////////////////
// FRAME TESTS
////////////////////////////////////////////////////////////////////

bool mclFrame::isMostRecentFrame() { 
  return (this == MCL()->mostRecentFrame()); 
}

bool mclFrame::evSignatureExists(string inputEVS) {
  for (list<mclFrameEntryVector*>::iterator fevi = fevList.begin();
       fevi != fevList.end();
       fevi++) {
    if (inputEVS == (*fevi)->vEVS)
      return true;
  }
  return false;
}

bool mclFrame::isSignatureExists(string inputIIS) {
  for (list<mclFrameEntryVector*>::iterator fevi = fevList.begin();
       fevi != fevList.end();
       fevi++) {
    if (inputIIS == (*fevi)->string_iis())
      return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
// LOG STUFF
////////////////////////////////////////////////////////////////////

void mclFrame::log(string& l) {
  char j[l.length() + 10];
  sprintf(j,"%d:%s",MCL()->tickNumber(),l.c_str());
  frameLog.push_back((string)j);
  umbc::uLog::annotate(MCLA_FRAMELOG,this->entityName()+":"+(string)j);
}

void mclFrame::log(const char* l) {
  char j[strlen(l) + 10];
  sprintf(j,"%d:%s",MCL()->tickNumber(),l);
  frameLog.push_back((string)j);
  umbc::uLog::annotate(MCLA_FRAMELOG,this->entityName()+":"+(string)j);
}

////////////////////////////////////////////////////////////////////
// DUMPING/PRINTING STUFF
////////////////////////////////////////////////////////////////////

void mclFrame::dumpEntity(ostream* o) {
  *o << "*********************************" << endl;
  *o << entityName() << ": state=" << getState() << endl;
  *o << "ACTIVE RESPONSE @" << active_response << endl;
  *o << "INDICATIONS :" << endl;
  getIndicationCore()->dumpEntity(o);
  *o << "FAILURES    :" << endl;
  getFailureCore()->dumpEntity(o);
  *o << "RESPONSES   :" << endl;
  getResponseCore()->dumpEntity(o);
}

void mclFrame::dumpLog(ostream& o) {
  o << entityName() << " log (state=" << getState() << ")" << endl;
  for (vector<string>::iterator fli = frameLog.begin();
       fli != frameLog.end();
       fli++) {
    o << " " << (*fli) << endl;
  }
}

string mclFrame::describe() {
  char buff[512];
  sprintf(buff,"{frame-0x%lx %s state[%d] fev[%d]}",
	  (unsigned long)this, default_glue_key.c_str(),
	  state,(int)fevList.size());
  return buff;
}
