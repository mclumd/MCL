#include "mcl_api.h"
#include "mcl_ma_internal_api.h"
#include "mcl.h"
#include "Exceptions.h"

#include "ontology_config.h"
#include "ontology_reader.h"
#include "noiseProfiles.h"

#include "../include/umbc/settings.h"
#include "../include/umbc/exceptions.h"
#include "mclLogging.h"
#include "mclConstants.h"
#include "reentrancy.h"

#include <iostream>
#include <map>

using namespace std;
using namespace metacog;

map<string,mcl*> mcl_ma_map;

char _ma_perror[512];

using namespace umbc;

settings::systemPreInit mpi("mcl");

//////////////////////////////////////////////////////////////////
// INITIALIZE / SETUP

mcl* mclMAint::get_or_create(string k) {
  if (mcl_ma_map.find(k) != mcl_ma_map.end()) {
    return mcl_ma_map[k];
  }
  else {
    mcl* nuMCL = new mcl();
    nuMCL->setMAkey(k);
    mcl_ma_map[k]=nuMCL;
    return mcl_ma_map[k];
  }
}

void mclMAint::dumpbase() {
  for (map<string,mcl*>::iterator mmi = mcl_ma_map.begin();
       mmi != mcl_ma_map.end();
       mmi++) {
    printf(" M(%s) = 0x%08lx\n",mmi->first.c_str(),
	   (unsigned long)(mmi->second));
  }
}


mcl* mclMAint::getMCLFor(string k) {
  if (mcl_ma_map.find(k) != mcl_ma_map.end()) {
    return mcl_ma_map[k];
  }
  else return NULL;
}

inline mcl* get_mcl(string k) { 
  return mcl_ma_map[k]; 
}

bool mclMA::initializeMCL(string key,int Hz) {
  umbc::uLog::annotate(MCLA_MSG,
		      "[mcl/maAPI]::Initializing MCL ~> supervising " + key);
  mcl* this_mcl = mclMAint::get_or_create(key);
  if (Hz == 0) 
    this_mcl->setSynchronous();
  else
    this_mcl->setAsynchronous(Hz);
  // this_mcl->clearSensorDefs();
  if (umbc::settings::debug) {
    umbc::uLog::annotate(MCLA_DBG,
			"[mcl/maAPI]::baseline PV = ");
    this_mcl->getDefaultPV()->dumpEntity(umbc::uLog::log);
  }
  this_mcl->start();
  return true;
}

bool mclMA::releaseMCL(string key) {
  mcl* this_mcl = mclMAint::getMCLFor(key);
  mcl_ma_map.erase(key);
  delete this_mcl;
  return true;
}

bool mclMA::stopMCL(string key) {
  mcl* this_mcl = mclMAint::getMCLFor(key);
  bool previous = this_mcl->is_active();
  this_mcl->stop();
  return previous;
}

bool mclMA::startMCL(string key) {
  mcl* this_mcl = mclMAint::getMCLFor(key);
  bool previous = this_mcl->is_active();
  this_mcl->start();
  return previous;
}

/*
bool mclMA::chooseOntology(string key, string ont) {
  mcl* this_mcl = mclMAint::get_or_create(key);
  return this_mcl->setOntologyBase(ont);
}
*/

bool mclMA::legalOntologySpec(string ospec) {
  return ontology_reader::legit_ontologyname(ospec);
}

bool mclMA::configureMCL(string key, string ontology, string domain) {
  mcl* this_mcl = mclMAint::get_or_create(key);
  this_mcl->setOntologyBase(ontology);
  this_mcl->setConfigKey(ontology_configurator::require_config(ontology,domain));
  return true; //DW 12/23/2009  
}

bool mclMA::configureMCL(string key, string ontology, string domain, 
			 string agent) {
  mcl* this_mcl = mclMAint::get_or_create(key);
  this_mcl->setOntologyBase(ontology);
  this_mcl->setConfigKey(ontology_configurator::require_config(ontology,domain,agent));
  return true; //DW 12/23/2009  
}

bool mclMA::configureMCL(string key, string ontology, string domain, 
			 string agent, string controller) {
  mcl* this_mcl = mclMAint::get_or_create(key);
  this_mcl->setOntologyBase(ontology);
  this_mcl->setConfigKey(ontology_configurator::require_config(ontology,domain,agent,controller));
  return true; //DW 12/23/2009  
}

//////////////////////////////////////////////////////////////////
// Sensor Vector Compatibility Mode

bool mclMA::initializeSV(string key,float *sv,int svs) {
  throw RemovedFromAPIException("initializeSV(string,float*,int)");
  /* 
     mcl* this_mcl = get_mcl(key);
  this_mcl->setSV(sv,svs);
  umbc::uLog::annotate(MCLA_MSG,"[mcl/hostAPI]:: "+key+" sensor vector initialized."); 
  */
  return false;
}

bool mclMA::updateSV(string key,float *sv,int svs) {
  throw RemovedFromAPIException("updateSV(string,float*,int)");
  /*
  mcl* this_mcl = get_mcl(key);
  this_mcl->setSV(sv,svs);  
  */
  return false;
}

bool mclMA::registerSensor(string key,string name) {
  observables::declare_observable_self(key,name);

  /* old compatibility...
     mcl* this_mcl = get_mcl(key);
     this_mcl->addSensorDef(new mclSensorDef(name,this_mcl));
     umbc::uLog::annotate(MCLA_MSG,"[mcl/hostAPI]:: "+key+" registered sensor '"+name+"'");
  */
  return true;
}

//////////////////////////////////////////////////////////////////
// Host Initiated Anomalies

bool mclMA::HIA::registerHIA(string key,string name,string failureNodeName) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->addHIADef(new mclHIADef(name,failureNodeName,this_mcl));
  umbc::uLog::annotate(MCLA_MSG,"[mcl/hostAPI]::registered host-initiated anomaly '"+name+"'");
  return true;
}

bool mclMA::HIA::signalHIA(string key,string name) {
  mcl* this_mcl = get_mcl(key);
  return this_mcl->signalHIA(name);
}

bool mclMA::HIA::signalHIA(string key,string name,resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  return this_mcl->signalHIA(name,referent);
}

//////////////////////////////////////////////////////////////////
// noise profiles

bool mclMA::setSensorNoiseProfile(string mclkey, string sname,spvType npKey) {
  observables::set_obs_noiseprofile_self(mclkey,sname,npKey);
  return true;
}

bool mclMA::setSensorNoiseProfile(string mclkey, string sname,spvType npKey,
				  double param) {
  observables::set_obs_noiseprofile_self(mclkey,sname,npKey,param);
  return true;
}

bool mclMA::setSensorProp(string mclkey,string obsname,
			  spkType key,spvType pv) {
  if (key == PROP_NOISEPROFILE)
    observables::set_obs_noiseprofile_self(mclkey,obsname,pv);
  else 
    observables::set_obs_prop_self(mclkey,obsname,key,pv);
  return true;
}

void mclMA::newDefaultPV(string key) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->newDefaultPV();
}

void mclMA::popDefaultPV(string key) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->popDefaultPV();
}

void mclMA::setEGProperty(string key,egkType eg_key,
			  pkType index,pvType value) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl->getExpGroup(eg_key))
    this_mcl->getExpGroup(eg_key)->getPV()->setProperty(index,value);
  else { 
    // sprintf(_ma_perror,"Attempt set property of non-existant EG stack '%s.%lx'",key.c_str(),eg_key);
    throw MissingEGException(eg_key);
  }
}

void mclMA::setPropertyDefault(string key,int index,pvType value) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->getDefaultPV()->setProperty(index,value);
}

void mclMA::reSetDefaultPV(string key) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->getDefaultPV()->reSetProperties();
}

//////////////////////////////////////////////////////////////////
// OPERABILITY

responseVector mclMA::monitor(string key,float *sv,int svs) {
  throw RemovedFromAPIException("monitor(string,float*,int)");
  /*
  mcl* this_mcl = get_mcl(key);
  umbc::uLog::annotateStart(MCLA_MSG);
  if (umbc::settings::debug) {
    *umbc::uLog::log << "[mcl/hostAPI]::monitor[";
    for (int i=0;i<svs;i++) {
      *umbc::uLog::log << sv[i];
      if (i<svs-1) *umbc::uLog::log << ",";
    }
    *umbc::uLog::log << "]" << endl;
  }
  this_mcl->setSV(sv,svs);
  return this_mcl->monitor();
  umbc::uLog::annotateEnd(MCLA_MSG);
  */
}

responseVector mclMA::monitor(string key,observables::update& the_update) {
  mcl* this_mcl = get_mcl(key);

  // umbc::uLog::annotateStart(MCLA_MSG);
  // the_update.dumpEntity(umbc::uLog::log);
  // umbc::uLog::annotateEnd(MCLA_MSG);

  observables::apply_update(key,the_update);
  return this_mcl->monitor();

}

responseVector mclMA::emptyResponse() {
  responseVector frrvec;
  return frrvec;
}

bool mclMA::updateObservables(string key,observables::update& the_update) {
  observables::apply_update(key,the_update);
  return true;
}

//////////////////////////////////////////////////////////////////
// EXPECTATIONS

void mclMA::declareExpectationGroup(string key,egkType eg_key) {
  declareExpectationGroup(key,eg_key,(egkType)NULL,RESREF_NO_REFERENCE);
}

void mclMA::declareExpectationGroup(string key, egkType eg_key,
				    egkType parent_key, resRefType ref) {
  sprintf(umbc::uLog::annotateBuffer,"[mcl/hostAPI]:: expectation group being declared (m=%s k=0x%lx pk=0x%lx ref=0x%lx)",key.c_str(),eg_key,parent_key,ref);
  umbc::uLog::annotateFromBuffer(MCLA_MSG);
  mcl* this_mcl = get_mcl(key); 
  this_mcl->declareExpectationGroup(eg_key,parent_key,ref);
}

void mclMA::expectationGroupAborted(string key,egkType eg_key) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->expectationGroupAborted(eg_key);
}

void mclMA::expectationGroupComplete(string key,egkType eg_key,float *sv,int svs) {
  throw RemovedFromAPIException("expectationGroupComplete(string,egkType,float*,int)");
  /*
    mcl* this_mcl = get_mcl(key);
    this_mcl->setSV(sv,svs);
    this_mcl->expectationGroupComplete(eg_key);
  */
}

void mclMA::expectationGroupComplete(string key,egkType eg_key,
				     observables::update& the_update) {
  mcl* this_mcl = get_mcl(key);
  observables::apply_update(key,the_update);
  this_mcl->expectationGroupComplete(eg_key);
}

void mclMA::expectationGroupComplete(string key,egkType eg_key) {
  umbc::uLog::annotate(MCLA_WARNING,"expectationGroupComplete(k,ke) is deprecated. please use the update version or suffer dire consequences.");
  mcl* this_mcl = get_mcl(key);
  this_mcl->expectationGroupComplete(eg_key);
}

///
/// these are internal functions (not in the API)
///

void ensure_eg(string key,egkType eg_key) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl->getExpGroup(eg_key) == NULL) {
    sprintf(umbc::uLog::annotateBuffer,
	    "Attempt to add exp to non-existant EG @0x%lx",eg_key);
    umbc::uLog::annotateFromBuffer(MCLA_ERROR);
    mclMA::declareExpectationGroup(key,eg_key);
  }
}

void ensure_sensor(string key,string sensor) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl->getSensorDef(sensor) == NULL) {
    throw MissingSensorException(sensor);
    // sprintf(_ma_perror,"Attempt to add exp for non-existant sensor '%s'",sensor.c_str());
    // umbc::exceptions::signal_exception(_ma_perror);
  }
}

mclExp *_ma_makeExp(string key,ecType code,float arg) {
  mcl* this_mcl = get_mcl(key);
  switch (code) {
  case EC_REALTIME:
     return expectationFactory::makeRealtimeExp(this_mcl,arg);
     break;
  case EC_TICKTIME:
     return expectationFactory::makeTickExp(this_mcl,arg);
     break;
  default:
    sprintf(_ma_perror,
	    "Could not handle EXP code 0x%08x in declaration (args=())",
	    code);
    throw BadCode(_ma_perror);
  }
}

mclExp *_ma_makeExp(string key,string sensor,ecType code) {
  mcl* this_mcl = get_mcl(key);
  switch (code) {
  case EC_ANY_CHANGE:
    return expectationFactory::makeNetChangeExp(this_mcl,sensor);
    break;
  case EC_NET_ZERO:
    return expectationFactory::makeNZExp(this_mcl,sensor);
    break;
  case EC_GO_UP:
    return expectationFactory::makeGoUpExp(this_mcl,sensor);
    break;
  case EC_GO_DOWN:
    return expectationFactory::makeGoDownExp(this_mcl,sensor);
    break;
  case EC_MAINTAINVALUE:
    return expectationFactory::makeMVExp(this_mcl,sensor);
    break;
  case EC_BE_LEGAL:
    return expectationFactory::makeBLExp(this_mcl,sensor);
    break; 
  case EC_DONT_CARE:
    return expectationFactory::makeDCExp(this_mcl,sensor);
  default:
    sprintf(_ma_perror,
	    "Could not handle EXP code 0x%08x in declaration (args=())",
	    code);
    throw BadCode(_ma_perror);
    return NULL;
  }
}

mclExp *_ma_makeExp(string key,string sensor,ecType code,float value) {
  mcl* this_mcl = get_mcl(key);
  switch (code) {
  case EC_STAYUNDER:
    return expectationFactory::makeUBExp(this_mcl,sensor,value);
    break;
  case EC_STAYOVER:
    return expectationFactory::makeLBExp(this_mcl,sensor,value);
    break;
  case EC_MAINTAINVALUE:
    return expectationFactory::makeMVExp(this_mcl,sensor,value);
    break;
  case EC_TAKE_VALUE:
    return expectationFactory::makeTakeValExp(this_mcl,sensor,value);
    break;
  default:
    sprintf(_ma_perror,"Could not handle EXP code 0x%08x in expectation declaration (dispatch with args (float))",
	    code);
    throw BadCode(_ma_perror);
    return NULL;
  }
}

mclExp *_ma_makeExp(string key,string sensor1,string sensor2,
		    ecType eCode,float value,int v2) {
  mcl* this_mcl = get_mcl(key);
  switch (eCode) {
  case EC_RATIO_MAINTAIN:
    return expectationFactory::makeRatioMaintExp(this_mcl,sensor1,sensor2,RELATION_EQUAL,value,v2);
  case EC_RATIO_STAYUNDER:
    return expectationFactory::makeRatioMaintExp(this_mcl,sensor1,sensor2,RELATION_LT,value,v2);
  case EC_RATIO_STAYOVER:
    return expectationFactory::makeRatioMaintExp(this_mcl,sensor1,sensor2,RELATION_GT,value,v2);
  default:
    sprintf(_ma_perror,"Could not handle EXP code 0x%08x in expectation declaration (dispatch with args (str,str,int,float,int))",
	    eCode);
    throw BadCode(_ma_perror);
    return NULL;
  }
}

///
/// these are the internal functions, which have pre-processed sensor names
///

void declareExpectation_pp(string key,egkType eg_key,ecType code,float arg) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl) {
    try {
      ensure_eg(key,eg_key);
      sprintf(umbc::uLog::annotateBuffer,
	      "[mcl/hostAPI]:: expectation declared (c=0x%x,v=%lf)",
	      code,arg);
      umbc::uLog::annotateFromBuffer(MCLA_VRB);
      mclExp* nuE = _ma_makeExp(key,code,arg);
      this_mcl->getExpGroup(eg_key)->addExp(nuE);
    }
    catch (MCLException me) {
      if (settings::getSysPropertyInt("mcl.debugLevel",1) <= 1)
	this_mcl->processInternalException(me);
      else
	throw me;
    }
  }
}

void declareExpectation_pp(string key,egkType eg_key,string pp_sensor,ecType code) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl) {
    try {
      string sensor = pp_sensor;
      ensure_eg(key,eg_key);
      ensure_sensor(key,sensor);
      sprintf(umbc::uLog::annotateBuffer,
	      "[mcl/hostAPI]:: expectation declared %s(c=0x%x)",
	      sensor.c_str(),code);
      umbc::uLog::annotateFromBuffer(MCLA_VRB);
      mclExp *nuE = _ma_makeExp(key,sensor,code);
      this_mcl->getExpGroup(eg_key)->addExp(nuE);
    }
    catch (MCLException me) {
      if (settings::getSysPropertyInt("mcl.debugLevel",1) <= 1) {
	this_mcl->processInternalException(me);
      }
      else
	throw me;
    }
  }
}

void declareExpectation_pp(string key,egkType eg_key,string pp_sensor,
			       ecType code,float value) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl) {
    try {
      string sensor = pp_sensor;
      ensure_eg(key,eg_key);
      ensure_sensor(key,sensor);
      sprintf(umbc::uLog::annotateBuffer,
	      "[mcl/hostAPI]:: expectation declared %s(c=0x%x,v=%lf)",
	      sensor.c_str(),code,value);
      umbc::uLog::annotateFromBuffer(MCLA_VRB);
      mclExp *nuE=_ma_makeExp(key,sensor,code,value);
      this_mcl->getExpGroup(eg_key)->addExp(nuE);
    }
    catch (MCLException me) {
      if (settings::getSysPropertyInt("mcl.debugLevel",1) <= 1)
	this_mcl->processInternalException(me);
      else
	throw me;
    }
  }
}

void declareExpectation_pp(string key,egkType eg_key,
			   string pp_sensor1,string pp_sensor2,
			   ecType eCode,float value,int value2) {
  mcl* this_mcl = get_mcl(key);
  if (this_mcl) {
    try {
      ensure_eg(key,eg_key);
      ensure_sensor(key,pp_sensor1);
      ensure_sensor(key,pp_sensor2);
      sprintf(umbc::uLog::annotateBuffer,
	      "[mcl/hostAPI]:: expectation declare %s:%s(c=0x%x,v=%lf)",
	      pp_sensor1.c_str(),pp_sensor2.c_str(),eCode,value);
      umbc::uLog::annotateFromBuffer(MCLA_VRB);
      mclExp *nuE=_ma_makeExp(key,pp_sensor1,pp_sensor2,eCode,value,value2);
      this_mcl->getExpGroup(eg_key)->addExp(nuE);
    }
    catch (MCLException me) {
      if (settings::getSysPropertyInt("mcl.debugLevel",1) <= 1)
	this_mcl->processInternalException(me);
      else
	throw me;
    }
  }

}


///
/// now here are the published functions
///

void mclMA::declareExpectation(string key,egkType eg_key,ecType code,float arg1) {
  declareExpectation_pp(key,eg_key,code,arg1);
}

void mclMA::declareExpectation(string key,egkType eg_key,
			       string self_sensor,ecType code) {
  declareExpectation_pp(key,eg_key,
			observable::make_self_named(self_sensor),
			code);
}

void mclMA::declareExpectation(string key,egkType eg_key,string self_sensor,
			       ecType code,float arg1) {
  declareExpectation_pp(key,eg_key,
			observable::make_self_named(self_sensor),
			code,arg1);
}

void mclMA::declareSelfExpectation(string key,egkType group_key,
				   string self_sensor, ecType code)
{ declareExpectation(key,group_key,self_sensor,code); }

void mclMA::declareSelfExpectation(string key,egkType group_key,
				   string self_sensor,
				   ecType code,float value)
{ declareExpectation(key,group_key,self_sensor,code,value); }

void mclMA::declareObjExpectation(string key, egkType eg_key,
			       string object, string observable, ecType code) {
  declareExpectation_pp(key,eg_key,
			observable::make_name(object,observable),
			code);
}

void mclMA::declareObjExpectation(string key,egkType eg_key,
				  string object, string observable,
				  ecType code,float arg1) {
  declareExpectation_pp(key,eg_key,
			observable::make_name(object,observable),
			code,arg1);
}

// 2 SENSOR RELATIONAL EXPS

void mclMA::declareSelfExpectation(string key,egkType group_key,
				   string obs1,string obs2,
				   ecType eCode,float value,int v2) {
  declareExpectation_pp(key,group_key,
			observable::make_self_named(obs1),
			observable::make_self_named(obs2),
			eCode,value,v2);
}

void mclMA::declareObjExpectation(string key,egkType group_key,
				  string obj,string obs1,string obs2,
				  ecType eCode,float value,int v2) {
  declareExpectation_pp(key,group_key,
			observable::make_name(obj,obs1),
			observable::make_name(obj,obs2),
			eCode,value,v2);
}

void mclMA::declareDelayedExpectation(string key,double delay,egkType eg_key,
				       string self_sensor,ecType code) {
  mcl* this_mcl = get_mcl(key);
  string sensor = observable::make_self_named(self_sensor);
  ensure_eg(key,eg_key);
  ensure_sensor(key,sensor);
  sprintf(umbc::uLog::annotateBuffer,
	  "[mcl/hostAPI]:: expectation declared %s+%lfs (c=0x%x)",
	  sensor.c_str(),delay,code);
  umbc::uLog::annotateFromBuffer(MCLA_VRB);
  mclExp *se = _ma_makeExp(key,sensor,code);
  mclExp *de = 
    expectationFactory::makeDelayedExp(this_mcl,delay,se);
  this_mcl->getExpGroup(eg_key)->addExp(de);  
}

void mclMA::declareDelayedExpectation(string key,double delay,egkType eg_key,
				      string self_sensor,ecType code,float v) {
  mcl* this_mcl = get_mcl(key);
  string sensor = observable::make_self_named(self_sensor);
  ensure_eg(key,eg_key);
  ensure_sensor(key,sensor);
  sprintf(umbc::uLog::annotateBuffer,
	  "[mcl/hostAPI]:: expectation declared %s+%lfs(c=0x%x,v=%f)",
	  sensor.c_str(),delay,code,v);
  umbc::uLog::annotateFromBuffer(MCLA_VRB);
  mclExp *se = _ma_makeExp(key,sensor,code,v);
  mclExp *de = 
    expectationFactory::makeDelayedExp(this_mcl,delay,se);
  this_mcl->getExpGroup(eg_key)->addExp(de);  
}

//
/// interactivity functionality
//

void mclMA::suggestionImplemented(string key,resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  mclFrame *frame = this_mcl->referent2frame(referent);
  if (frame == NULL) {
    throw MissingReferentException("suggestionImplemented");
  }
  else {
    this_mcl->processSuggImplemented(frame);
  }
}

mclMonitorResponse* mclMA::suggestionDeclined(string key,resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  mclFrame *frame = this_mcl->referent2frame(referent);
  if (frame == NULL) {
    throw MissingReferentException("suggestionDeclined");
  }
  else {
    return this_mcl->processSuggDeclined(frame);
  }
}

mclMonitorResponse*  mclMA::suggestionFailed(string key,resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  mclFrame *frame = this_mcl->referent2frame(referent);
  if (frame == NULL) {
    throw MissingReferentException("suggestionFailed");
  }
  else {
    this_mcl->processSuggFailed(frame);
  }
}

void mclMA::suggestionIgnored(string key,resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  mclFrame *frame = this_mcl->referent2frame(referent);
  if (frame == NULL) {
    throw MissingReferentException("suggestionIgnored");
  }
  else {
    this_mcl->processSuggIgnored(frame);
  }
}

void mclMA::provideFeedback(string key,bool feedback, resRefType referent) {
  mcl* this_mcl = get_mcl(key);
  mclFrame *frame = this_mcl->referent2frame(referent);
  umbc::uLog::annotate(MCLA_MSG, "[mcl/hostAPI]:: receiving feedback for " +
		      frame->entityName());
  if (frame == NULL) {
    // probably should issue a warning and not generate an exception
    throw MissingReferentException("provideFeedback");
  }
  else {
    this_mcl->processHostFeedback(feedback,frame);
  }  
}

// UTILITIES

bool mclMA::writeInitializedCPTs(string key) {
  mcl* m = mclMAint::getMCLFor(key);
  if (m == NULL) {
    sprintf(_ma_perror,"attempting to write CPT for '%s' but it does not have an associated, initialized MCL.",key.c_str());
    throw BadKey(_ma_perror);
  }
  else {
    mclFrameEntryVector fev(ENTRY_UNKNOWN);
    mclFrame    *f = new mclFrame(m,fev);
    bool rv = (umbc::settings::getSysPropertyBool("writeConfigGlobal",false)) ?
      cpt_cfg::save_cpts_to_global(f) :      
      cpt_cfg::save_cpts_to_home(f);
    if (rv) {
      return true;
    }
    else {
      cout << "a problem occurred while writing to " 
	   << "'" << f->MCL()->getConfigKey() << "'"
	   << endl;
      return false;
    }
  }
  return false;  // never reached but quiets -Wall
}

bool mclMA::writeInitializedCosts(string key) {
  return false;
}

// RENTRANT BEHEVIOR

bool mclMA::setREB(string key, string reb) {

  // 1. Get the MCL associated with this key
  mcl* m = mclMAint::getMCLFor(key);
  if (m == NULL) {
    sprintf(_ma_perror,"attempting to set REB for '%s' but it does not have an associated, initialized MCL.",key.c_str());
    throw BadKey(_ma_perror);
    return false;
  }

  // 2. Set the REB
  m->setREB(reb);

  // 3. Return success
  return true;
}

bool mclMA::getREB(string key, string &reb) {

  // 1. Get the MCL associated with this key
  mcl* m = mclMAint::getMCLFor(key);
  if (m == NULL) {
    sprintf(_ma_perror,"attempting to get REB for '%s' but it does not have an associated, initialized MCL.",key.c_str());
    throw BadKey(_ma_perror);
    return false;
}

  // 2. Get the REB
  reb = m->getREB();

  // 3. Return success
  return true;
}

// DEBUG / DIAGNOSTICS

void mclMA::setOutput(string fn) { umbc::uLog::setLogToFile(fn); }
void mclMA::setStdOut() { umbc::uLog::setLogToStdOut(); }
void mclMA::beQuiet()   { umbc::settings::quiet=true; }
void mclMA::beVerbose() { umbc::settings::quiet=false; }
void mclMA::goDebug()   { umbc::settings::debug=true; }
void mclMA::noDebug()   { umbc::settings::debug=false; }

void mclMA::dumpOntologiesMostRecent(string key) {
  mcl* this_mcl = get_mcl(key);
  this_mcl->dumpMostRecentFrame();
}

void mclMA::dumpMostRecentFrameDot(string key) {
  dumpFrameDot(key,0);
}
void mclMA::dumpFrameDot(string key,int dot) {
  mcl* this_mcl = get_mcl(key);  
  this_mcl->dumpFrameDot(dot);
}

void mclMA::dumpMCL() {
  cout << "MCL Configurations :" << endl;
  for (map<string,mcl*>::iterator mmI=mcl_ma_map.begin();
       mmI!=mcl_ma_map.end();
       mmI++) {
    cout << " ~> " << mmI->first << ": @0x" << hex << mmI->second << endl;
  }
}

void mclMA::dumpNPT() {
  noiseProfile::np_dump();
}
