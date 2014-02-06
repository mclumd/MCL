#ifndef MCL_MONITOR_RESPONSE_H
#define MCL_MONITOR_RESPONSE_H

/*! \file mclMonitorResponse.h
  \brief contains class definitions for objects returned from mcl::monitor()
*/

#include <string>
#ifndef MCL_API_TYPES_H
#include "APITypes.h"
#endif
#include "mcl_symbols.h"
using namespace std;

string respName(pkType crc);

//! Base class for MCL monitor response.
/*! The base class providing minimum functionality and virtual/abstract
 function prototypes for responses from MCL to the Host on a monitor() call */
class mclMonitorResponse {
 public:
  mclMonitorResponse() : myText(""),myReferenceCode((resRefType)NULL) {};
  mclMonitorResponse(resRefType frameRef,egkType egk) 
    : myText(""),myReferenceCode((resRefType)frameRef),myEGK(egk) {};
  virtual ~mclMonitorResponse() {};
  virtual bool requiresAction()=0; //!< action is recommended in response
  virtual bool recommendAbort()=0; //!< current Host activity should be aborted
  /*! responseText is a natural language description of the response */
  virtual string responseText() { return myText; };
  virtual void   setResponse(string s) { myText=s; };
  /*! reference code is a pointer to an mclFrame for referring to 
    frames/responses when the host initiates an interactive response. */
  virtual resRefType referenceCode() { return myReferenceCode; };
  virtual void       setRefCode (resRefType r) { myReferenceCode=r; };
  virtual egkType    egRef() { return myEGK; };
  virtual void       setEgRef (egkType egk) { myEGK=egk; };
  virtual mclMonitorResponse *clone()=0;

  virtual string to_string();
  virtual string rclass()=0;

  static void clean_out(vector<mclMonitorResponse *> &v) {
    while (!v.empty()) {
      mclMonitorResponse *mmr = v.back();
      v.pop_back();
      delete mmr;
    }
  }
  
 protected:
  string myText;
  resRefType myReferenceCode;
  egkType    myEGK;

};

//! Class representing an error within MCL.
/*! In the unlikely event of an internal MCL error encountered during 
  processing of a monitor call, this class should be used. */
class mclInternalErrorResponse : public mclMonitorResponse {
 public:
  mclInternalErrorResponse() :mclMonitorResponse() 
    { setResponse("Internal MCL error."); };
  virtual ~mclInternalErrorResponse() {};
  virtual bool requiresAction() { return false; };  
  virtual bool recommendAbort() { return false; };  
  virtual mclMonitorResponse *clone()
    { return new mclInternalErrorResponse(*this); };
  virtual string rclass() { return "internalError"; };
};

//! Class representing no error.
/*! Generally no objects at all are issued when there is not an expectation
  violation. If for any reason we need to communicate back to the host
  that no violation occurred, this is the class we'd use. */
class mclMonitorOKResponse : public mclMonitorResponse {
 public:
  mclMonitorOKResponse(resRefType frameRef,egkType egk) 
    : mclMonitorResponse(frameRef,egk)
    { setResponse("violation considered safe to ignore."); };
  virtual ~mclMonitorOKResponse() {};
  virtual bool requiresAction() { return false; };  
  virtual bool recommendAbort() { return false; };  
  virtual mclMonitorResponse *clone()
    { return new mclMonitorOKResponse(*this); };
  virtual string rclass() { return "noAnomalies"; };
};

//! Class indicating no action was taken by MCL.
/*! Generally not necessary. Indicates that MCL has not taken any action
  vis-a-vis the referent. Down the road it is forseeable that MCL as a
  deliberative process may want to express that something is being considered
  but at the current time NO OP has been taken. Use this class in that case. */
class mclMonitorNOOPResponse : public mclMonitorResponse {
 public:

 mclMonitorNOOPResponse(resRefType frameRef,egkType egk) 
   : mclMonitorResponse(frameRef,egk)
    { setResponse("MCL recommends no response at this time."); };
  virtual ~mclMonitorNOOPResponse() {};
  virtual bool requiresAction() { return false; };  
  virtual bool recommendAbort() { return false; };  
  virtual mclMonitorResponse *clone()
    { return new mclMonitorNOOPResponse(*this); };
  virtual string rclass() { return "noOperation"; };

};

//! Class indicating that MCL has run out of viable responses.
/*!  */
class mclMonitorNoMoreOptions : public mclMonitorResponse {
 public:

 mclMonitorNoMoreOptions(resRefType frameRef,egkType egk) 
   : mclMonitorResponse(frameRef,egk)
    { setResponse("MCL has exhausted all probable responses."); };
  virtual ~mclMonitorNoMoreOptions() {};
  virtual bool requiresAction() { return false; };  
  virtual bool recommendAbort() { return false; };  
  virtual mclMonitorResponse *clone()
    { return new mclMonitorNoMoreOptions(*this); };
  virtual string rclass() { return "optionsExhausted"; };

};

//! Class indicating MCL recommends action be taken.
/*! Typical response from monitor (if any) belongs to this class. Expresses
 that action needs to be taken by the Host to address an expectation 
 violation. */
class mclMonitorCorrectiveResponse : public mclMonitorResponse {
 public:
  mclMonitorCorrectiveResponse(resRefType frameRef,egkType egk,pkType respCode) 
    : mclMonitorResponse(frameRef,egk),crc(respCode),hostShouldAbort(true) { 
    setResponse("recommendation: "+respName(respCode)); 
  };
  virtual ~mclMonitorCorrectiveResponse() {};
  virtual bool requiresAction() { return true; };  
  /*! returns a code corresponding to a concrete MCL response to be
    implemented by the host.
    \sa #crc_enum
  */
  virtual pkType responseCode() { return crc; };
  virtual bool recommendAbort() { return hostShouldAbort; };  
  void setAbortRec(bool rec) { hostShouldAbort=rec; };
  virtual mclMonitorResponse *clone()
    { return new mclMonitorCorrectiveResponse(*this); };

  virtual string to_string();
  virtual string rclass() { return "suggestion"; };

 protected:
  pkType crc;
  bool   hostShouldAbort;
};

// vector of responses from MCL monitor function
typedef vector<mclMonitorResponse *> responseVector; 

#endif
