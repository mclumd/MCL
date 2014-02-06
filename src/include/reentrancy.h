#ifndef MCL_RE_ENTRANCY_H
#define MCL_RE_ENTRANCY_H

#include "frameEntryVector.h"

namespace metacog {

  using namespace std;

  /**
     abstract class that defines a behavior for deciding re-entry into a frame.
  */
  class ReEntrantBehavior {
  public:
    virtual bool selectFramesForReEntry(mclFrameEntryVector& fev,
					frameVec& allFrames,
					frameVec& selectonVector)=0;
    virtual string describe()=0;
    virtual string name()=0;
    virtual ~ReEntrantBehavior() {};
    
  protected:
    ReEntrantBehavior() {};
  };

  class SelectBestREB : public ReEntrantBehavior {
  public:
    virtual bool selectForSuccess(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score)=0;
    virtual bool selectForFailure(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score)=0;
    virtual bool selectFramesForReEntry(mclFrameEntryVector& fev,
					frameVec& allFrames,
					frameVec& selectonVector);    
  };
  
  /**
     Passive Hosts are basically monitoring hosts. Any controls asserted
     by the Host is not considered a new expectation group, and thus the 
     results or consequences of action taking are not explicitly monitored
     by MCL. Rather, the process of monitoring is wrapped into a monolith
     of an expectation group. The group may change (to incorporate different
     "phases" or "regimes") but the group change is not correlated to changes
     in host behavior so much as it is environmental or regime changes in
     whatever is being monitored.
     
     the criteria for re-entering a frame on failure are thus not so much 
     tied to EG but refernet, expectation signature, and violation.
  */
  class PassiveREB : public SelectBestREB {
  public:
    PassiveREB() : SelectBestREB() {};
    virtual bool selectForSuccess(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score);
    virtual bool selectForFailure(mclFrameEntryVector& fev,mclFrame* frame,
				   double* score);
    virtual string describe();
    virtual string name();
    
  };


  namespace REB {
    ReEntrantBehavior *makeREB(string identifier);
    ReEntrantBehavior *makeREB();
  };

};

#endif
