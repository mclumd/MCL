#include "mclFrame.h"
#include "mclLogging.h"
#include "mcl.h"
#include "Exceptions.h"
#include "../include/umbc/text_utils.h"

using namespace metacog;

void mclFrame::reentry_recurrence(mclExp* exp, mclFrameEntryVector& fev) {
  using namespace umbc;
  add_new_fev(fev);

  uLog::annotate(MCLA_MSG,"[mclFrame]:: frame re-entered on RECURRENCE.");

  switch (state) {
    
  case FRAME_RESPONSE_TAKEN:
    // do not reLink on recurrence
    // if (exp != NULL) MCL()->linkToIndicationFringe(this,exp); 
    
    // - decide whether or not to add the frame as pending
    if (active_response == NULL)
      throw IllegalFrameState("RESPONSE_TAKEN specified but active_response is NULL");
    else {
      uLog::annotate(MCLA_MSG,"[mclFrame]:: frame re-entered with active response "+active_response->entityBaseName());
      sprintf(uLog::annotateBuffer,"frame statistics for %s = %s",
	      active_response->entityBaseName().c_str(),
	      active_response->countsAsString().c_str());
      uLog::annotateFromBuffer(MCLA_MSG);
      active_response->inc_failedAttempts();
      if (active_response->isNotWorking()) {
	active_response->set_evidence(false);
	state=FRAME_UPDATED;
	MCL()->markForReassess(this);
	string m=
	  active_response->entityBaseName()+" has been judged nonworking.";
	log(m);
      }
      else {
	uLog::annotate(MCLA_MSG,"[mclFrame]:: active response has been granted a stay of execution.");
      }
    }
    
    break;

  case FRAME_RESPONSE_SUCCEEDED:
    // okay, there is a lot that could go here...
    // a recency check to make sure the response actually did work
    // for example
    //
    // but, for now, just re-enter assess
    MCL()->markForReassess(this);
    break;

  case FRAME_RESPONSE_IGNORED:
    // okay, here we ignore the violation, but should we be timing it?
    if (active_response) {
      uLog::annotate(MCLA_MSG,
		     "[mclFrame]:: host requested the violation be ignored.");
      active_response->inc_ignoredAttempts();
    }
    else {
      NoActiveResponseToAddress("reentry_recurrence");
    }
    break;
    
  case FRAME_RESPONSE_ISSUED:
    // maybe the host is doing something, but if it is, it's broken
    // with protocol... advice is IGNORE
    uLog::annotate(MCLA_MSG,"[mclFrame]:: waiting for ACK from host for "+active_response->entityBaseName()+", MCL will ignore recurrence.");
    // probably should count here...
    break;

  default:
    uLog::annotate(MCLA_ERROR,"[mclFrame]:: recurrence occurred in unhandled state: "+textFunctions::num2string(state));
    break;

  }
}

void mclFrame::reentry_ignoring() {
  if (active_response != NULL) {
    string m=active_response->entityBaseName()+" has been ignored by the host.";
    log(m);
    // now what? do we set a timer? how do we handle ignores?
    state=FRAME_RESPONSE_IGNORED;
    active_response->inc_ignoredAttempts();
  }
  else {
    throw NoActiveResponseToAddress("SuggestionIgnored");
  }
}

void mclFrame::reentry_implementing() {
  if (active_response != NULL) {
    string m=active_response->entityBaseName()+" is being implemented by the host.";
    log(m);
    state = FRAME_RESPONSE_TAKEN;
  }
  else {
    throw NoActiveResponseToAddress("SuggestionImplemented");
  } 
}

mclMonitorResponse* mclFrame::reentry_declining() {
  if (active_response != NULL) {
    string m=
      active_response->entityBaseName()+" has been declined by the host.";
    log(m);
    active_response->set_evidence(false);
    assess();
    return guide();
  }
  else {
    throw NoActiveResponseToAddress("SuggestionDeclined");
  } 
}

mclMonitorResponse* mclFrame::reentry_aborting() {
  if (active_response != NULL) {
    string m=
      active_response->entityBaseName()+" has been aborted by the host.";
    log(m);
    // is this consistent with the semantics? it is the same as declined...
    active_response->set_evidence(false);
    assess();
    return guide();
  }
  else {
    throw NoActiveResponseToAddress("ResponseAborted");
  } 
}

void mclFrame::reentry_success() {
  // there should be some logic in here specific to the active_response
  // but for now, note success and change frame state
  if (active_response != NULL) {
    bool sco = false;
    active_response->noteCleanPass(&sco);
    if (sco) {
      state=FRAME_RESPONSE_SUCCEEDED;
      string m=
	active_response->entityBaseName()+" has apparently succeeded.";
      log(m);
    }
  }
  else {
    throw NoActiveResponseToAddress("reentry_success");
  }   
}

mclMonitorResponse* mclFrame::reentry_failing() {
  if (active_response != NULL) {
    string m=
      active_response->entityBaseName()+" is marked a failure by the host.";
    log(m);
    // is this consistent with the semantics? it is the same as declined...
    active_response->set_evidence(false);
    assess();
    return guide();
  }
  else {
    throw NoActiveResponseToAddress("ResponseFailed");
  } 
}
