#include "oNodes.h"
#include "mclFrame.h"
#include "mclLogging.h"
#include "../include/umbc/exceptions.h"

using namespace metacog;

/******************************************  basic stuff */

double mclConcreteResponse::cost() { return cost_static; };

void mclConcreteResponse::resetCycleCounts() {
  failCountPad=0;
  successCountPad=0;
}

void mclConcreteResponse::noteJustIssued() {
  totalAttempts++;
  resetCycleCounts();
}

void mclConcreteResponse::noteCleanPass(bool* stateChangeOk) {
  inc_successfulAttempts();
  if (stateChangeOk != NULL) {
    // do we check for durative here? when do we assume success?
    umbc::uLog::annotate(MCLA_WARNING,
			 "[oNode]::success check is weak for noteCleanPass.");
    // okay, so we are assuming success right away... for now
    *stateChangeOk=true;
    // but in the future we will probably want this to be adaptive and/or
    // host-settable
  }
}

/******************************************  interactive stuff */

bool mcl_AD_InteractiveResponse::interpretFeedback(bool feedback) {
  for (vector<string>::iterator pi = positiveList.begin();
       pi != positiveList.end();
       pi++) {
    mclNode* pnode = myFrame()->findNamedNode(*pi);
    if (pnode == NULL) {
      umbc::exceptions::signal_exception("tried to find node "+*pi+" in interactive response but it does not exist.");
    }
    else if (feedback) {
      umbc::uLog::annotate(MCLA_MSG,"[mcl/oNode]::asserting A/D+ dependency "+*pi+" set to true.");
      pnode->set_evidence(true);
    }
    else {
      umbc::uLog::annotate(MCLA_MSG,"[oNode]::asserting A/D+ dependency "+*pi+" set to false.");
      pnode->set_evidence(false);
    }
  }
  for (vector<string>::iterator ni = negativeList.begin();
       ni != negativeList.end();
       ni++) {
    mclNode* nnode = myFrame()->findNamedNode(*ni);
    if (nnode == NULL)
      umbc::exceptions::signal_exception("tried to find node "+*ni+" in interactive response but it does not exist.");
    else if (feedback) {
      umbc::uLog::annotate(MCLA_MSG,"[oNode]::asserting A/D- dependency "+*ni+" set to false.");
      nnode->set_evidence(false);
    }
    else {
      umbc::uLog::annotate(MCLA_MSG,"[oNode]::asserting A/D- dependency "+*ni+" set to true.");
      nnode->set_evidence(true);
    }
  }
  return (true && mclInteractiveResponse::interpretFeedback(feedback));
}

string mclConcreteResponse::countsAsString() {
  char buff[256];
  sprintf(buff,"(fail=%d,succ=%d,ignore=%d,abort=%d)",
	  failedAttempts,successfulAttempts,ignoredAttempts,abortedAttempts);
  return (string)buff;
}


/******************************************  DOT file implementations */

string mclHostProperty::dotDescription() {
  return dotLabel()+" [shape=diamond,color=sienna,fontcolor=sienna];";
}

string mclHostInitiatedIndication::dotDescription() {
  return dotLabel()+" [shape=rectangle,style=filled,fillcolor=sienna];";
}

string mclConcreteIndication::dotDescription() {
  return dotLabel()+" [shape=ellipse,color=sienna,fontcolor=sienna];";
}

string mclGeneralIndication::dotDescription() {
  return dotLabel()+" [shape=ellipse,color=purple,fontcolor=purple];";
}

string mclIndicationCoreNode::dotDescription() {
  return dotLabel()+" [shape=ellipse,color=purple,peripheries=2,fontcolor=purple];";
}

string mclFailure::dotDescription() {
  return dotLabel()+" [shape=ellipse,color=red,fontcolor=red];";
}

string mclGeneralResponse::dotDescription() {
  return dotLabel()+" [shape=ellipse,color=green,fontcolor=green];";
}

string mclConcreteResponse::dotDescription() {
  return dotLabel()+" [shape=rectangle,color=sienna,fontcolor=sienna];";
}
