/* **************************************************
 * Implement Renetrant Behavior for Dissertation
 *
 * Dean Earl Wright
 * Fall 2010
 *
 **************************************************** 

F0: No MCL (Passive)
F1: Always the same frame
F2: Always use a new frame
F3: Random
F4: reuse frame if EVS equal
F5: IIS equal
F6: both IIS and EVS equal
F7: EVS equal but IIS different
F8: IIS equal but EVS different
F9: Current algorithm (Passive)

 **************************************************** */ 

#include "reentrancy.h"
#include "mclFrame.h"
#include "DWtestREB.h"
#include "stdlib.h"
#include "mclLogging.h"

using namespace std;
using namespace umbc;
using namespace metacog;

bool DWtestREB::selectFramesForReEntry(mclFrameEntryVector& fev,
                                       frameVec& allFrames,
                                       frameVec& selectionVector) {
            
  // 1. Start with no frame selected for reentry
  mclFrame* bestFrame  = NULL;
  frameVec candidates;  
  frameVec::iterator fvi;
  int i;

  // 2. Log the request
#ifndef NO_DEBUG

  // 2a. Output header with method number
  char char_num[31];  
  //snprintf(char_num, 30, "%d", method_number);
  uLog::annotate(ULAT_NORMAL,"[DWtestREB]::selectFramesForRetry("+describe()+")");

  // 2b. Output Frame Entry Vectory
  string desc_fev = fev.describe();
  uLog::annotate(ULAT_NORMAL,"[DWtestREB]::"+desc_fev);
  
  // 2c. Output number of frames in allFrames
  snprintf(char_num, 30, "%d", (int)allFrames.size());
  uLog::annotate(ULAT_NORMAL,"[DWtestREB]::allFrames("+((string)char_num)+")");
  
  // 2d. Loop for all of the Frames
  for (fvi = allFrames.begin();
       fvi != allFrames.end();
       fvi++) {
       
       // 2e. Output the frame
       string desc_frame  = (*fvi)->describe();
       uLog::annotate(ULAT_NORMAL,"[DWtestREB]::"+desc_frame);
  } // end for
#endif
  
  // 3. Select canidate frames
  bool single = false;
  if (method_number == 0 || method_number == 9) single = true;
  selectCandidateFramesForReEntry(fev, allFrames, candidates, single);
                                 
  // 4. Select frame (if any) based on behavior
  switch (method_number) {

    // F0: No MCL (Passive)
    // F9: Current algorithm (Passive)
    case 0: 
    case 9: 
      for (fvi = candidates.begin();
           fvi != candidates.end();
           fvi++) {
        selectionVector.push_back(*fvi);
      } // end for
      return selectionVector.empty();
      
    // F1: Always the same frame
    case 1: 
      if (!candidates.empty())
        bestFrame=candidates[0];
      break;
      
    // F2: Always use a new frame
    case 2: 
      break;
      
    // F3: Random frame
    case 3:
      i = random_frame(fev, candidates, true);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;
    
    // F4: reuse frame if EVS equal
    case 4: 
      i = random_frame(fev, candidates);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;
      
    // F5: IIS equal
    case 5: 
      i = random_frame(fev, candidates);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;
      
    // F6: both IIS and EVS equal
    case 6: 
      i = random_frame(fev, candidates);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;
      
    // F7: EVS equal but IIS different
    case 7: 
      i = random_frame(fev, candidates);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;
      
    // F8: IIS equal but EVS different 
    case 8: 
      i = random_frame(fev, candidates);
      if (i >= 0) 
        bestFrame = candidates[i];
      break;

    default: //Invalid method_number
      break;

  } // end switch
  
  // 5. Return the frame and true if we found one
  if (bestFrame) {
    selectionVector.push_back(bestFrame);
#ifndef NO_DEBUG
    string desc_frame = bestFrame->describe();
    uLog::annotate(ULAT_NORMAL,"[DWtestREB]::selected "+desc_frame);
#endif    
    return true;
  }
  
  // 6. Return False if no frame for re-entry
#ifndef NO_DEBUG
    uLog::annotate(ULAT_NORMAL,"[DWtestREB]::selected none");
#endif    
  return false;
}  
  
bool DWtestREB::selectCandidateFramesForReEntry(mclFrameEntryVector& fev,
                                                frameVec& allFrames,
                                                frameVec& selectionVector,
                                                bool single) {
  // 1. Start with no best canidate
  double best_score = -1;
  mclFrame* bestFrame  = NULL;
  
  // 2. Determine if we are look for success or failure frames
  bool success = fev.isSuccessfulEntry();
  
  // 3. Loop for all of the frames
  for (frameVec::iterator afi = allFrames.begin();
       afi != allFrames.end();
       afi++) {
       
    // 4. Get the score for this frame   
    double current = -1;
    bool   possible;
    if (success) {
      possible = selectForSuccess(fev,(*afi),&current);
    } else {
      possible = selectForFailure(fev,(*afi),&current);
    }  
    if (possible) {
      if (single) {
        if (current > best_score) {
          best_score=current;
          bestFrame=(*afi);
        }
      } else {
       selectionVector.push_back(*afi);
      }  // end else if (single)
    } // end if (possible)
  }
  if (single && bestFrame) {
    selectionVector.push_back(bestFrame);
  }
  return !selectionVector.empty();
}


bool DWtestREB::selectForSuccess(mclFrameEntryVector& fev,
                                 mclFrame* frame,
				 double* score) {
				 
  // 1. An referent match is always acceptable				 
  if (frame->matchesReferent(fev.vRef)) {
    if (score) *score=1.0;
    return true;
  } 
  
  // 2. Defermine acceptability based on behavior    
  switch (method_number) {

    // F0: No MCL - flag for FrameRover.py
    // F9: Current algorithm (Passive)
    case 0: 
    case 9:
      if ((fev.vEG == frame->get_vegKey()) &&
          (frame->isMostRecentFrame()) &&
          (frame->in_advice_state())) {
            if (score) *score=1.0;
            return true;
          }
      break;

    // F1: Always the same frame
    // F2: Always use a new frame
    // F3: Random
    // F3: Reuse frame if EVS equal
    // F4: IIS equal
    // F5: both IIS and EVS equal
    // F6: EVS equal but IIS different
    // F7: IIS equal but EVS different 
    default:
      break;      
  }
  
  // 3. Didn't like this frame
  return false;
}

bool DWtestREB::selectForFailure(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score) {
  // 1. An referent match is always acceptable				 
  if (frame->matchesReferent(fev.vRef)) {
    if (score) *score=1.0;
    return true;
  } 
  
  // 2. Defermine acceptability based on behavior    
  switch (method_number) {

    // F0: No MCL - flag for FrameRover.py
    // F9: Current algorithm (Passive)
    case 0:
    case 9:
      if ((fev.vEG == frame->get_vegKey()) &&
          (frame->evSignatureExists(fev.vEVS)))  {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
      }
      break;

    // F1: Always the same frame
    case 1: 
      if (score) *score=1.0;
      fev.vECode = REENTRY_RECURRENCE;
      return true;
      
    // F2: Always use a new frame
    case 2: 
      return false;

    // F3: Random
    case 3:
      if (score) *score=1.0;
      fev.vECode = REENTRY_RECURRENCE;
      return true; 
      
    // F4: reuse frame if EVS equal
    case 4:
      if (frame->evSignatureExists(fev.vEVS)) {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
    }
    break;
    
    // F5: IIS equal
    case 5: 
      if (frame->isSignatureExists(fev.string_iis())) {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
    }
    break;
      
    // F6: both IIS and EVS equal
    case 6: 
      if ((frame->isSignatureExists(fev.string_iis())) && 
          (frame->evSignatureExists(fev.vEVS)) ) {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
    }
    break;
      
    // F7: EVS equal but IIS different
    case 7: 
      if ((!frame->isSignatureExists(fev.string_iis())) && 
          (frame->evSignatureExists(fev.vEVS)) ) {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
    }
    break;
      
    // F8: IIS equal but EVS different 
    case 8: 
      if ((frame->isSignatureExists(fev.string_iis())) && 
          (!frame->evSignatureExists(fev.vEVS)) ) {
        if (score) *score=1.0;
        fev.vECode = REENTRY_RECURRENCE;
        return true;
    }
    break;
      
    default: //Invalid method_number
      break;  
      
   } // end switch(method_number) 

  // 3. Didn't like this frame
  return false;
}


// -------------------------------------------------------------------
// Algorithms to select the best frame from a set of canidates or not.
// -------------------------------------------------------------------
int DWtestREB::random_frame(mclFrameEntryVector& fev,
			    frameVec& candidates,
			    bool addNone) {
 					
  // 1. No canidates, return new frame
  if (candidates.empty()) {
    return -1;
  }  
  
  // 2. Get method_number of choices (size maybe plus one)
  long size = candidates.size();
  long choices = size;
  if (addNone) ++choices;
  
  // 3. Get random choice
  ldiv_t m = ldiv(random(), choices);
  
  // 4. Return new frame if not selecting a frame
  if ((unsigned int)m.rem >= candidates.size())
    return -1;
    
  // 5. Return method_number of canidate frame to use  
  return m.rem;
} 					
  
int DWtestREB::first_frame(mclFrameEntryVector& fev,
			   frameVec& candidates) {
  // 1. No canidates, return new frame
  if (candidates.empty()) {
    return -1;
  }  

  // 2. Else return the first frame
  return 0;
}					


string DWtestREB::describe() {
  string what = "???";
  switch (method_number) {
    case 0: what = "0: Passive"; break;
    case 1: what = "1: First"; break;
    case 2: what = "2: New"; break;
    case 3: what = "3: Random"; break;
    case 4: what = "4: EVS"; break;
    case 5: what = "5: IIS"; break;
    case 6: what = "6: EVS and IIS"; break;
    case 7: what = "7: EVS but not IIS"; break;
    case 8: what = "8: IIS but not EVS"; break;
    case 9: what = "9: Passive"; break;
    default: what ="?: Invalid method_number"; break;
  }  
  return "DW_REB=" + what;
}

string DWtestREB::name() {
  switch (method_number) {
    case 0: return "zero";
    case 1: return "one";
    case 2: return "two";  
    case 3: return "three";
    case 4: return "four";
    case 5: return "five";
    case 6: return "six";
    case 7: return "seven";
    case 8: return "eight";
    case 9: return "nine";
  }  
  return "???";
}
