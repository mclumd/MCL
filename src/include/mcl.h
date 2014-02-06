#ifndef MCL_BASE_H
#define MCL_BASE_H

/** \file
 * \brief class definition for the MCL structure.
 */

#include "mcl_internal_types.h"
#include "mclTimer.h"
#include "mclEntity.h"
#include "mclSensorDef.h"
#include "mcl_observables.h"
#include "mclHIADef.h"
#include "mclMonitorResponse.h"
#include "mclFrame.h"
#include "mclProperties.h"
#include "Exceptions.h"
#include "expectations.h"
#include "reentrancy.h"
#include "mcl_api.h"
#include <vector>
#include <map>

namespace metacog {

/** the Meta Cognitive Loop class.
 * manages expectations, monitoring of expectations, and meta-reasoning
 * about failures for a host system.
 */
class mcl : public mclEntity {
 public:
  mcl();

  string baseClassName() { return "mcl"; };
  
  mcl *MCL() { return this; };

  void setHostName(string hn) { hName=hn; };
  string getHostName() { return hName; };

  void setConfigKey(string kc);
  string getConfigKey() { return cKey; };

  void setMAkey(string mk) { maKey=mk; };
  string getMAkey() { return maKey; };

  // returns false if base does not exist...
  bool setOntologyBase(string ont);
  string getOntologyBase() { return oName; };

  //! sets operation of the MCL to be synchronous with #monitor() calls.
  void setSynchronous()       { synchronous=true; };
  //! sets operation of the MCL to be asynchronous with #monitor() calls.
  //! Asynchronous operation requires a Hz response rate. If #monitor() is
  //! called more frequently than the specified Hz response rate, #monitor()
  //! will return NULL in the off-cycle calls.
  void setAsynchronous(int h) { synchronous=false; Hz=h; };

  // I think these should be destroyed since they use an old and ugly protocol
  //
  // commented out tentatively on 5/2010 MS
  //
  // void clearSensorDefs();
  // void addSensorDef(mclSensorDef *sd) { sensorDefs.push_back(sd); };

  // see... this uses the observables protocol
  mclSensorDef *getSensorDef(string name);
  legalspec *getLegalSpec(string name);

  void addHIADef(mclHIADef *sd)       { HIAdefs.push_back(sd); };
  mclMonitorResponse *signalHIA(string name);
  mclMonitorResponse *signalHIA(string name,resRefType referent);
  mclMonitorResponse *activateNodeDirect(string name,resRefType referent);

  /** passes over active expectations to check for violations.
   * Actually, a ton of stuff goes on in monitor(). The chief thing is to 
   * run through expectations looking for violations, though. For each,
   * an #mclFrame is created and inference is run over the ontologies.
   * For each new or updated (existing mclFrames are also updated at 
   * this point) frame, an @mclMonitorResponse is created and returned in a
   * vector to the host.
   */
  responseVector monitor();

  void start()     { active=true;   };
  void stop()      { active=false;  };
  bool is_active() { return active; };

  mclExpGroup* getExpGroup(egkType eg_key, bool returnMarkedGroups=false);
  void addExpGroup(egkType eg_key,mclExpGroup* eg)
    { expGroups.push_back(eg); };
  void declareExpectationGroup(egkType key,egkType parent,resRefType ref);
  void expectationGroupAborted(egkType eg_key);
  void expectationGroupComplete(egkType eg_key);

  void dump_eg_table();

  //! returns the value of the named sensor.
  float sensorValue(string selfSensorName);
  float sensorValue(string objName, string objPropName);

  float sensor_value_pp(string sn);

  //! issues a new poperty vector and places it on the top of the PV stack.
  void newDefaultPV() { defaultPropertyStack.newPropertyVector(); };
  //! pops the topmost property vector off the PV stack.
  void popDefaultPV() { defaultPropertyStack.popPropertyVector(); };
  //! returns the topmost property vector from the default PV stack.
  mclPropertyVector *getDefaultPV() 
    { return defaultPropertyStack.currentPV(); };
  //! tests a property value from the topmost property vector on the PV stack.
  //! @param propKey the key for the property to be tested (see #APICodes.h)
  //! @param propVal the target value to be tested against (see #APICodes.h)
  bool testPropertyDefault(pkType propKey, pvType propVal) {
    return getDefaultPV()->testProperty(propKey,propVal);
    // return (getDefaultPV()->getPropertyValue(propKey) == propVal);
  };

  //! returns the number of #monitor() calls that have been processed.
  int tickNumber() { return tickCounter; };

  //! activates the relevant indication fringe nodes for a violation.
  //! the source for this functionality can be found in @dynamicILinks.cc
  //! and uses an expectation method called #generateLinkTags() to generate
  //! a listing of the appropriate fringe node names to activate.
  //! @param m an mclFrame describing an expectation violation
  //! @param e an expectation that has been violated
  void linkToIndicationFringe(mclFrame* m,mclExp* e);

   // HOST-MCL FEEDBACK PROTOCOL

  //! the host has implemented/is implementing the response suggested 
  //! in the specified frame.
  void processSuggImplemented(mclFrame* frame);

  //! the host has declined the response suggested in the specified frame.
  mclMonitorResponse* processSuggDeclined(mclFrame* frame);

  //! the host has ignored the response suggested in the specified frame.
  void processSuggIgnored(mclFrame* frame);

  //! the host is providing feedback to an interactive response
  void processHostFeedback(bool hostReply, mclFrame* frame);

  //! the host has signaled failure of a frame recommendation.
  //! "Off cycle" refers to any MCL/Host event that occurs off of a #monitor()
  //! cycle. Since #monitor() is the only time which MCL communicates to the
  //! Host, off cycle events must be added to a queue for processing at the
  //! beginning of the next monitor() call.
  mclMonitorResponse* processSuggFailed(mclFrame* frame);

  //! attempts to recover a frame using provided context.
  //! This function will return the Frame most relevant to the provided 
  //! context: using the referent if possible, then the EG Key of the 
  //! violation, then the parent's key.
  //! @param r a Response Referent (a pointer to an MCL Frame)
  //! @param e an expectation group key
  //! @param p the expectation group of the parent
  mclFrame* recoverExistingFrame(resRefType r,egkType e,egkType p);

  mclFrame* referent2frame(resRefType r);
  mclFrame* mostRecentFrame();

  //! processes an internally generated MCL error.
  //! the purpose of this is to determine when a host is not adhering to
  //! protocols or is acting pathologically so that MCL can suggest
  //! repairs not associated directly with the host's primary operation.
  //! @param E the exception generated by MCL's internal error trapping
  void processInternalException(MCLException& E);

  void deleteMarkedGroups();
  void dumpMostRecentFrame();
  void dumpFrameDot(int which);
  void dumpFrameVec(frameVec& fv);

  //! specifies that the Frame has pending bidness with MCL.
  void markForReassess(mclFrame* nf);

  // these are to support the enhanced observables code
  void add_observable(observable* noo)
    { my_ov->add_observable(noo); };
  void add_observable_object_def(object_def* od)
    { my_ov->add_observable_object_def(od); };
  void add_object_field_def(string tname,string fname,mclSensorDef& sd)
    { my_ov->add_object_field_def(tname,fname,sd); };
  void notice_object_is_observable(string otype,string oname) 
    { my_ov->observe_observable_object(otype,oname); };
  void notice_object_unobservable(string oname)
    { my_ov->stow_observable_object(oname); };
  void observable_update_pp(string obsname, double v)
    { my_ov->observable_update_pp(obsname,v); };
  void observable_update(string obsname, double v)
    { my_ov->observable_update(obsname,v); };
  void observable_update(string objname, string obsname, double v)
    { my_ov->observable_update(objname,obsname,v); };
  void dump_ov() { my_ov->dump(); };

  void set_obs_prop_self(string obsname,spkType key,spvType pv)
    { my_ov->set_obs_prop_self(obsname,key,pv); };
  void set_obs_prop(string objname, string obsname,spkType key,spvType pv)
    { my_ov->set_obs_prop(objname,obsname,key,pv); };

  void add_obs_legalval_self(string obsname,double v)
    { my_ov->add_obs_legalval_self(obsname,v); };
  void add_obs_legalval_def(string objnm,string obsnm,double v)
    { my_ov->add_obs_legalval_def(objnm,obsnm,v); };
  void add_obs_legalval_obj(string objnm,string obsnm,double v)
    { my_ov->add_obs_legalval_obj(objnm,obsnm,v); };

  void set_obs_legalrange_self(string obsname,double low, double high)
    { my_ov->set_obs_legalrange_self(obsname,low,high); };
  void set_obs_legalrange_def(string objnm,string obsnm,double low, double high)
    { my_ov->set_obs_legalrange_def(objnm,obsnm,low,high); };
  void set_obs_legalrange_obj(string objnm,string obsnm,double low, double high)
    { my_ov->set_obs_legalrange_obj(objnm,obsnm,low,high); };

  bool obs_is_legal(string obsname)
    { return my_ov->obs_is_legal(obsname); };
  bool obs_is_legal(string objname,string obsname)
    { return my_ov->obs_is_legal(objname,obsname); };
  bool obs_is_legal_pp(string sname)
    { return my_ov->is_legal_pp(sname); };

  void set_noise_profile(string selfOname,spvType npKey);
  void set_noise_profile(string selfOname,spvType npKey,double param);
  void set_noise_profile(string objnm,string obsnm,spvType npKey);
  void set_noise_profile(string objnm,string obsnm,spvType npKey,double param);

  void dump_obs(string obsname)
    { my_ov->dump_obs(obsname); };
  void dump_obs(string objname, string obsname)
    { my_ov->dump_obs(objname,obsname); };
  string ov2string() { return my_ov->active2string(); };

  // getter and setter for Reentrant Behavior policy
  void setREB(string reb);
  string getREB(void); 

 private:

  // general info / parameters
  string hName; // host name
  string cKey;  // configuration key to be used by ontology_configurator
  string maKey; // multi-agent key
  string oName; // ontology def name (defaults to "basic")

  // stuff for doing updates
  bool     active;
  bool     synchronous;
  int      Hz;
  mclTimer ticker; 
  int      tickCounter;

  // vectors of things mcl needs to keep track of
  sdVec    sensorDefs;
  HIAVec   HIAdefs;
  egMap    expGroups;
  frameVec frames;

  observable_vector* my_ov;
  
  //! pending frames is where frames that have been generated "off cycle"
  //! (not in the note function) are stored for processing at the next note
  //! phase
  frameVec pendingFrames;

  mclPropertyVectorStack defaultPropertyStack;

  ReEntrantBehavior* reb_policy;

  // DEPRECATED !! 6-10-2010 MS
  float   *sv;
  int      svl;

  // internal functions

  void safeFramePush(mclFrame* nf,frameVec* fv);
  mclHIADef *getHIADef(string name);
  //! destroys an expectation group without checking effects expectations.
  void deleteExpGroup(mclExpGroup* eg);
  //! queues an expectation group for deletion on cycle.
  void markExpGroupForDelete(egkType eg_key);
  //! queues an expectation group for deletion on cycle.
  void markExpGroupForDelete(mclExpGroup* egp);

  // NAG
  void                nag   (responseVector& frrvec);
  bool                note  (frameVec& fv);
  bool                assess(mclFrame *m);
  mclMonitorResponse *guide (mclFrame *m);  

  //! executes the note step for a single expectation group.
  //! @note internal use only.
  bool noteForExpGroup2(frameVec& resVec,mclExpGroup* eGrp,bool complete);

  //! Retires frames if needed and possible
  int retireFrames(void) {return retireFrames(MAXIMUM_MCL_FRAMES);};
  int retireFrames(int maximum);
  //! considers retiring a frame if it is a success.
  bool maybeRetireFrame(mclFrame* the_frame);
  //! prevents retiring a frame if it is a success.
  bool mustNotRetireFrame(mclFrame* the_frame);

};
};

#endif
