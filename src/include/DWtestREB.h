#ifndef MCL_DW_TEST_REB_H
#define MCL_DW_TEST_REB_H

#include "reentrancy.h"

namespace metacog {

  using namespace std;

  /**
     Multiple REBs being tested by Dean Wright for PhD dissertaion
  */
  class DWtestREB : public SelectBestREB {
  public:
    DWtestREB() : SelectBestREB() {method_number = 0;};
    DWtestREB(int num) : SelectBestREB() {method_number = num;};
    virtual bool selectFramesForReEntry(mclFrameEntryVector& fev,
					frameVec& allFrames,
					frameVec& selectonVector);    
    virtual bool selectCandidateFramesForReEntry(mclFrameEntryVector& fev,
					frameVec& allFrames,
					frameVec& selectonVector,
					bool single);    
    virtual bool selectForSuccess(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score);
    virtual bool selectForFailure(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score);			   
    virtual string describe();
    virtual string name();
  private:
    int method_number;  
    int random_frame(mclFrameEntryVector& fev,
	             frameVec& candidates,
		     bool addNone=false);
    int first_frame(mclFrameEntryVector& fev,
	            frameVec& candidates);
  };

};

#endif

