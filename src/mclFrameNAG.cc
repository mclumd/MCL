#include "mclFrame.h"
#include "mclLogging.h"
#include "makeDot.h"
#include "../include/umbc/settings.h"

using namespace metacog;
using namespace umbc;

mclConcreteResponse *selectConcreteNodeCBA(mclFrame *f);

void mclFrame::assess() {
  uLog::annotate(MCLA_MSG,"[mclFrameNAG]:: entering assess phase");

  // should compute frame properties and possibly do merging
  mclOntology* fo = getFailureCore();
  mclNode*    mpn = fo->maxPNode();

  if (mpn) {
    sprintf(uLog::annotateBuffer,
	    "[mclFrameNAG]:: most probable node is %s, [def]p=%.2f",
	    mpn->entityBaseName().c_str(), mpn->p_true());
    uLog::annotateFromBuffer(MCLA_MSG);
  }
  else {
    uLog::annotate(MCLA_WARNING,"[mclFrameNAG]:: no maximum P node?");
  }
}

mclMonitorResponse* mclFrame::guide() {
  uLog::annotate(MCLA_MSG,"[mclFrameNAG]:: entering guide for " + entityName());
  return generateResponse();
}

mclConcreteResponse *selectConcreteNodeCBA(mclFrame *f) {
  mclOntology *ro = f->getResponseCore();
  mclConcreteResponse *bn = NULL;
  double   cb = 0.0001;
  for (list<mclNode *>::iterator ni = ro->firstNode();
       ni != ro->endNode();
       ni++) {
    mclConcreteResponse *mcr = dynamic_cast<mclConcreteResponse *>(*ni);
    if (mcr != NULL) {
      sprintf(umbc::uLog::annotateBuffer,
	      "[mclFrameNAG]:: %s p=%.2f cost=%.2f u=%.2f",
	      mcr->entityName().c_str(), mcr->p_true(), mcr->cost(),
	      (mcr->p_true() / mcr->cost()));
      umbc::uLog::annotateFromBuffer(MCLA_VRB);
      if ((mcr->p_true() / mcr->cost()) > cb) {
	bn=mcr;
	cb=mcr->p_true() / mcr->cost();
      }
    }
  }
  // will return NULL if nothing in the failure ontology is active
  return bn;
}

mclMonitorResponse *mclFrame::generateResponse() {
  mclConcreteResponse *maxnode=selectConcreteNodeCBA(this);
  if (maxnode == NULL) {
    setState(FRAME_DEADEND);
    sprintf(uLog::annotateBuffer,
	    "Frame %s has entered the DEAD_END state.",
	    entityName().c_str());
    uLog::annotateFromBuffer(MCLA_WARNING);
    log("frame is out of responses.");
    return new mclMonitorNoMoreOptions(frame_id,veg_key);
  }
  else {
    // eventually...
    // mclMonitorResponse *mmr1 = maxnode->node2response(this);
    mclMonitorResponse *mmr = 
      new mclMonitorCorrectiveResponse(frame_id,veg_key,maxnode->respCode());
    // mmr->setResponse("MCL recommendation is to "+respName(maxnode->respCode()));
    bool none,mult;
    string restr=symbols::reverse_lookup("crc",maxnode->respCode(),&none,&mult);
    char x[32];
    sprintf(x,"%d",maxnode->respCode());
    string xs = x;
    if (none)
      mmr->setResponse("MCL response = ("+
		       xs+","+
		       maxnode->entityBaseName()+","+
		       "?unknown response?)");
    else if (mult)
      mmr->setResponse("MCL response = ("+
		       xs+","+
		       maxnode->entityBaseName()+","+
		       restr+")");
    else
      mmr->setResponse("MCL response = ("+
		       xs+","+
		       maxnode->entityBaseName()+","+
		       restr+")");

    // might not be semantically safe to do these next few ops      
    setState(FRAME_RESPONSE_ISSUED);
    set_active_response(maxnode);
    string m="issuing response: "+maxnode->entityBaseName();
    log(m);
    if (active_response != NULL)
      uLog::annotate(MCLA_MSG,
		     "[mclFrameNAG]:: set active response to "+
		     active_response->entityName());
    else
      uLog::annotate(MCLA_MSG,
		     "[mclFrameNAG]:: active response somehow not set...");
    // this uses the Dot/dump feature in writeAll for diagnostics
    if (settings::getSysPropertyBool("mcl.logFrames",false)) {
      char b[12];
      sprintf(b,"-%d-%d",refails,successes);
      writeAll("dumps/"+entityName()+b+".dot",this);
    }
    return mmr;
  }
}

