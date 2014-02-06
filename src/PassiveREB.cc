#include "reentrancy.h"
#include "mclFrame.h"

using namespace metacog;

bool PassiveREB::selectForSuccess(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score) {
  if (frame->matchesReferent(fev.vRef)) {
    if (score) *score=1.0;
    return true;
  }
  else {
    // okay, the conditions for success-re-entry are:
    // 1) same vEG key
    // 2) frame is most recently violated frame
    // 3) frame has outstanding advice
    if ((fev.vEG == frame->get_vegKey()) &&
	(frame->isMostRecentFrame()) &&
	(frame->in_advice_state())) {
      if (score) *score=1.0;
      return true;
    }
    return false;
  }
}

bool PassiveREB::selectForFailure(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score) {
  if (frame->matchesReferent(fev.vRef)) {
    if (score) *score=1.0;
    return true;
  }
  else if ((fev.vEG == frame->get_vegKey()) &&
	   (frame->evSignatureExists(fev.vEVS)))  {
    if (score) *score=1.0;
    fev.vECode = REENTRY_RECURRENCE;
    return true;
  }
  else return false;
}

string PassiveREB::describe() {
  return "{ReEntrantBehavior 'passive'}";
}

string PassiveREB::name() {
  return "passive";
}
