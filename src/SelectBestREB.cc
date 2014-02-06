#include "reentrancy.h"
#include "mclFrame.h"

using namespace metacog;

bool SelectBestREB::selectFramesForReEntry(mclFrameEntryVector& fev,
					   frameVec& allFrames,
					   frameVec& selectionVector) {
  double best_score = -1;
  mclFrame* bestFrame  = NULL;
  bool succent = fev.isSuccessfulEntry();
  for (frameVec::iterator afi = allFrames.begin();
       afi != allFrames.end();
       afi++) {
    double current = -1;
    if (succent) {
      if (selectForSuccess(fev,(*afi),&current)) {
	if (current > best_score) {
	  best_score=current;
	  bestFrame=(*afi);
	}
      }
    }
    else {
      if (selectForFailure(fev,(*afi),&current)) {
	if (current > best_score) {
	  best_score=current;
	  bestFrame=(*afi);
	}
      }
    }
  }
  if (bestFrame) {
    selectionVector.push_back(bestFrame);
    return true;
  }
  else {
    return false;
  }
}
