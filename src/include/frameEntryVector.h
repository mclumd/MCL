#ifndef MCL_FEV_H
#define MCL_FEV_H

#include "APITypes.h"
#include "mclConstants.h"
#include "expectations.h"
#include <string>
#include <list>

namespace metacog {

  using namespace std;

  /** an object that fully describes the state of the host/MCL interaction
      at the time of a violation, in preparation for re-entry check.
  */
  class mclFrameEntryVector {
  public:
    
  mclFrameEntryVector(entry_code ecode) : 
    vEG(EGK_NO_EG),vRef(RESREF_NO_REFERENCE),vEVS("no violation"),vECode(ecode) 
    { };

    //! this constructor is for violations
  mclFrameEntryVector(mclExpGroup* eGrp, mclExp* violated,
		      entry_code ec=ENTRY_NEW) :
    vEG(eGrp->get_egKey()),vRef(eGrp->get_referent()),vECode(ec) {
      if (violated) {
	vEVS=violated->signature();
	violated->addLinkTags(vIIS);
      }
      // okay, where does the PTrace come from?
    };
    
    mclFrameEntryVector(const mclFrameEntryVector& source);
    mclFrameEntryVector* clone();

    string  describe();
    egkType parentKey();
    string  string_iis();

    // these are tests that are shorthand for various multiline if/getters
    bool isSuccessfulEntry();
    bool isViolationEntry();
    
    /** violation Expectation Group Key **/
    egkType       vEG;
    /** violation Frame Referent **/
    resRefType    vRef;
    /** violation Initial Indication Signature **/
    list<string>  vIIS;
    /** violation Parent EGK Trace **/
    list<egkType> vPTrace;
    /** Expectation Violation Signature **/
    string        vEVS;
    /** entry code (where it came from) **/
    entry_code    vECode;
  };
  
};

#endif
