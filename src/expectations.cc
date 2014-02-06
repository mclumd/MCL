#include "expectations.h"
#include "mcl.h"
#include "../include/umbc/exceptions.h"
#include "mclLogging.h"

using namespace metacog;
using namespace umbc;

//////
///// group constructors moved here due to mcl references
//////

mclExpGroup::mclExpGroup(mcl *m,egkType ekey,egkType pkey,resRefType ref) 
  : mclEntity(),myMCL(m),this_key(ekey),parent_key(pkey),this_ref(ref),
    deleteMe(false),lastViolationTick(0) {
  // make a copy of the best known PV
  if (pkey) {
    mclExpGroup* pgroup = m->getExpGroup(pkey);
    if (pgroup && pgroup->getPV())
      pgroup->getPV()->copyValues(myPV);
    else if (m->getDefaultPV())
      m->getDefaultPV()->copyValues(myPV);
  }
  else if (m->getDefaultPV())
    m->getDefaultPV()->copyValues(myPV);
}

//////
///// THIS IS THE BEGINNING OF A MILLION EXPECTATION CLASS DEFS
//////

//////
///// but first, there could be some mixin classes
//////



//////
///// MAINTENANCE EXPECTATIONS LISTED FIRST
//////
//////
///// tick-based timer
//////

mclTickExp::mclTickExp(mcl *m, int maxTicks) : 
  mclMaintenanceExp("TickExpectation",m),tickMax(maxTicks),
  ticks(0) {
  lastTick = m->tickNumber();
}

bool mclTickExp::violation() {
  if (itsMCL->tickNumber() != lastTick) {
    ticks++;
    if (ticks >= tickMax)
      return true;
  }
  return false;
}

//////
///// real-time timer
//////

bool mclRealTimeExp::violation() {
  return internalTimer.expired();
}

//////
///// ratioMaintenance (ratio relation throughout lifetime of exp)
//////

mclRatioMaintenanceExp::mclRatioMaintenanceExp
  (mcl* m,string numeratorSensor, string denominatorSensor,
   int relation, float ratio, int minTotal) :
    mclMaintenanceExp("mclRatioMaintenanceExp",m),
    numerator(numeratorSensor),denominator(denominatorSensor),
    threshold(ratio),ratioRelation(relation),minimum(minTotal) {
}

bool genericRelation(float v1, float v2, int relation) {
  switch (relation) {
  case RELATION_EQUAL: return (v1 == v2);
  case RELATION_GT: return (v1 > v2);
  case RELATION_GTE: return (v1 >= v2);
  case RELATION_LT: return (v1 < v2);
  case RELATION_LTE: return (v1 <= v2);
  otherwise: return false;
  }
}

bool mclRatioMaintenanceExp::violation() {
  float n = itsMCL->sensor_value_pp(numerator);
  float d = itsMCL->sensor_value_pp(denominator);
  return ((minimum > 0) && ((n+d) >= minimum) && (genericRelation((n/d),threshold,ratioRelation)));
}

//////
///// maintainValue (hold value throughout lifetime of exp)
//////

mclMaintainValueExp::mclMaintainValueExp(string sensor,mcl *m) : 
  mclPollAndTestExp(sensor,"MaintainValue",m),nochange(true)
{ target=initial=itsMCL->sensor_value_pp(sensor); }

mclMaintainValueExp::mclMaintainValueExp(string sensor,mcl *m,float targ) : 
  mclPollAndTestExp(sensor,"MaintainValue",m),target(targ),nochange(false) 
{ initial=itsMCL->sensor_value_pp(sensor); }

bool mclMaintainValueExp::violation() {
  noiseProfile* np =noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np) {
    cout << " MAINTAIN VALUE: " << sname << " (w/ np) " 
	 << itsMCL->sensor_value_pp(sname)
	 << " vs. " << target 
	 << " p=" << np->p_that_EQ(itsMCL->sensor_value_pp(sname),target)
	 << endl;
    cout << " ::> " << np->describe() << endl;
    return !tolerate(np->p_that_EQ(itsMCL->sensor_value_pp(sname),target));
  }
  else {
    cout << " MAINTAIN VALUE: " << sname << " (no np) " << itsMCL->sensor_value_pp(sname)
	 << " vs. " << target << endl;      
    return (itsMCL->sensor_value_pp(sname) != target);
  }
}

//////
///// UpperBound (must never exceed upper threshold)
//////

bool mclUpperBoundExp::violation() {
  noiseProfile* np =noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_LT(itsMCL->sensor_value_pp(sname),bound));
  else return (itsMCL->sensor_value_pp(sname) > bound);
}

//////
///// LowerBound (must never go below lower threshold)
//////

bool mclLowerBoundExp::violation() {
  noiseProfile* np =noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_GT(itsMCL->sensor_value_pp(sname),bound));
  else return (itsMCL->sensor_value_pp(sname) < bound);
}

//////
///// RemainLegal -- must remain within the specified legalrange/set
//////

bool mclRemainLegalExp::violation() {
  // not checking noise profiles here... not sure whether this should always
  // be the case
  try {
    bool oil = itsMCL->obs_is_legal_pp(sname);
    if (oil)
      cout << "saying it's legal..." << endl;
    else
      cout << "saying it's non legal..." << endl;
    return !oil;
  }
  catch (MissingSensorException mse) {
    uLog::annotate(MCLA_WARNING,"during EC_LEGAL: "+(string)mse.what());
  }
  catch (...) {
    uLog::annotate(MCLA_WARNING,"unknown error trapped in EC_LEGAL");
  }
  return false;
}

//////
///// EFFECT EXPECTATIONS LISTED SECOND
//////

mclPollOnExitExp::mclPollOnExitExp(string sensor,string name,mcl *m) :
  mclEffectExp(name,m),sname(sensor) {
  initial=itsMCL->sensor_value_pp(sensor);
}

string mclPollOnExitExp::describe() {
  char x[255];
  sprintf(x,"(%s s=%s i=%.2f)",className().c_str(),sname.c_str(),initial);
  return (string)x;
}

//////
///// GoUp (must have gone up when exp group is dissolved)
//////

mclGoUpExp::mclGoUpExp(string sensor, mcl *m) :
  mclPollOnExitExp(sensor,"GoUp",m) { }

bool mclGoUpExp::violation() {
  noiseProfile* np=noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_GT(itsMCL->sensor_value_pp(sname),initial));
  else return (itsMCL->sensor_value_pp(sname) <= initial);
}

//////
///// GoDown (must have gone down when exp group is dissolved)
//////

mclGoDownExp::mclGoDownExp(string sensor, mcl *m) :
  mclPollOnExitExp(sensor,"GoDown",m) { }

bool mclGoDownExp::violation() {
  noiseProfile* np=noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_LT(itsMCL->sensor_value_pp(sname),initial));
  else return (itsMCL->sensor_value_pp(sname) >= initial);
}

//////
///// NoNetChange (must not have changed when exp group is dissolved)
//////

mclAnyNetChangeExp::mclAnyNetChangeExp(string sensor, mcl *m) :
  mclPollOnExitExp(sensor,"AnyChange",m) {
}

bool mclAnyNetChangeExp::violation() {
  noiseProfile* np=noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_EQ(itsMCL->sensor_value_pp(sname),initial));
  else return (itsMCL->sensor_value_pp(sname) == initial);
}

//////
///// NoNetChange (must not have changed when exp group is dissolved)
//////

mclNoNetChangeExp::mclNoNetChangeExp(string sensor, mcl *m) :
  mclPollOnExitExp(sensor,"NoChange",m) {
}

bool mclNoNetChangeExp::violation() {
  return (itsMCL->sensor_value_pp(sname) != initial);
}

//////
///// TakeValue (must have taken assigned value when exp group is dissolved)
//////

mclTakeValueExp::mclTakeValueExp(string sensor, mcl *m,float tar) :
  mclPollOnExitExp(sensor,"TakeValue",m),target(tar) { }

bool mclTakeValueExp::violation() {
  noiseProfile* np=noiseProfile::getNoiseProfileFor(itsMCL->getMAkey(),sname);
  if (np)
    return !tolerate(np->p_that_EQ(itsMCL->sensor_value_pp(sname),target));
  else return (itsMCL->sensor_value_pp(sname) != target);
}

string mclTakeValueExp::describe() {
  char x[255];
  sprintf(x,"(%s s=%s t=%.2f)",className().c_str(),sname.c_str(),target);
  return (string)x;
}

//////
///// DelayedMaintenance (sub-expectation becomes active after delay)
//////

mclDelayedMaintenanceExp::mclDelayedMaintenanceExp
(mcl *m,double delay, mclExp *subExp) :
  mclMaintenanceExp("DelayedExpectation",m),secs((int)delay),
  usecs((int)((delay-(int)delay)*1000000)),
  sub(subExp) {
  internalTimer.restart(secs,usecs);
}

bool mclDelayedMaintenanceExp::violation() {
  if (sub == NULL)
    umbc::exceptions::signal_exception("DelayedMaintenanceExp built with NULL subExpectation.");
  return (internalTimer.expired() && sub->violation());
}

//////
///// phaseChange (delay, changes to target value then maintains)
//////

mclPhaseChangeExp::mclPhaseChangeExp(mcl *m, 
				     double delay, 
				     string sensor, 
				     float target) :
  mclDelayedMaintenanceExp(m,delay,
			   new mclMaintainValueExp(sensor,m,target)) {};
