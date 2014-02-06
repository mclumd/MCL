#include "mcl.h"
#include "mclNode.h"
#include "mcl_internal_types.h"
#include "mclLogging.h"
#include "Exceptions.h"
#include "../include/umbc/settings.h"
#include "../include/umbc/exceptions.h"
#include "../include/umbc/text_utils.h"

using namespace metacog;

/** \file
 * contains the functionality for linking expectation violations into the MCL ontology fringe.
 */

////
///  this function links an expectation into from a frame's indications
////

void dumpLTS(linkTags_t &linkage) {
  *umbc::uLog::log << " fringe linkage: {";
  for (linkTags_t::iterator sti = linkage.begin();
       sti != linkage.end();
       sti++) 
    *umbc::uLog::log << " " << *sti;
  *umbc::uLog::log << " }" << endl;
}

void mcl::linkToIndicationFringe(mclFrame *f,mclExp *e) {

  // phase 1: ask for linkage from the expectation
  linkTags_t linkage = e->generateLinkTags();

  umbc::uLog::annotateStart(MCLA_PRE);
  *umbc::uLog::log << "[mcl/ilinks]::";
  dumpLTS(linkage);
  umbc::uLog::annotateEnd(MCLA_PRE);

  // phase 2: activate the named linkages
  mclOntology *io = f->getIndicationCore();
  for (linkTags_t::iterator sti = linkage.begin();
       sti != linkage.end();
       sti++) {
    // cout << "fringe linkage to " << *sti << endl;
    mclNode *nta = io->findNamedNode(*sti);
    if (nta == NULL) {
      if (umbc::settings::getSysPropertyBool("mcl.breakOnDLinkFailure",false))
	throw MissingFringeNode(*sti);
      else {
	umbc::uLog::annotate(MCLA_ERROR,"failure to dynamically link to indication node \""+*sti+"\"");
      }
    }
    else {
      nta->set_evidence(true);
      // nta->assertTrue();
    }
  }

}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// utility

string scNodeLookup(spvType scCode) {
  bool empty=false,multi=false;
  string node = symbols::reverse_lookup("sc",scCode,&empty,&multi);
  cout << "lookup " << (int)scCode << " = " << node << endl;
  if (empty) {
    return "sc:unspec";
  }
  else {
    umbc::textFunctions::rchars(node,'_',':');
    return node;
  }
    
  /*
string sc_to_fringe[] = {
  "state","control","spatial","temporal","resource","reward","ambient",
  "objectprop","message","counter","unspecified_sc" };

  if (scCode >= SC_NUMCODES_LEGAL) {
    umbc::exceptions::signal_exception("Illegal sensorCode->indicationFringe lookup (out of range).");
    return "";
  }
  else 
    return sc_to_fringe[scCode];
  */
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// here are the internal linkage generators for the individual 
// expectations

void mclTickExp::addLinkTags(linkTags_t& rv) {
  rv.push_back("temporal");
  rv.push_back("late");
}

void mclRealTimeExp::addLinkTags(linkTags_t& rv) {
  rv.push_back("temporal");
  rv.push_back("late");
}

void do_sensor_provenance(linkTags_t& mrv, string sname) {
  if (observable::is_self_sensor(sname))
    mrv.push_back("provenance:self");
  else if (observable::is_object_sensor(sname))
    mrv.push_back("provenance:object");
  else mrv.push_back("provenance:unknown");
};

void mclMaintenanceExp::addLinkTags(linkTags_t& mrv) {
  // maintenance has been removed... temporarily?
  // mrv.push_back("maintenance");
}

void mclPollAndTestExp::addLinkTags(linkTags_t& mrv) {
  mclMaintenanceExp::addLinkTags(mrv);
  // get the sensor properties and push them onto the list...
  mcl *thisMCL = MCL();
  mclSensorDef *esd = thisMCL->getSensorDef(sname);

  do_sensor_provenance(mrv,sname);

  // sensor class
  string scs = scNodeLookup(esd->getSensorProp(PROP_SCLASS));
  if (scs.length() > 0)
    mrv.push_back(scs);


  // value class
  //   if (esd->testSensorProp(PROP_DT,DT_RATIONAL) ||
  //       esd->testSensorProp(PROP_DT,DT_INTEGER))
  //     mrv.push_back("ordinal");
  //   else
  //     mrv.push_back("discrete");

}

void mclPollOnExitExp::addLinkTags(linkTags_t& mrv) {
  // get the sensor properties and push them onto the list...
  mcl *thisMCL = MCL();
  mclSensorDef *esd = thisMCL->getSensorDef(sname);

  do_sensor_provenance(mrv,sname);

  // sensor class
  string scs = scNodeLookup(esd->getSensorProp(PROP_SCLASS));
  if (scs.length() > 0)
    mrv.push_back(scs);

  // value class
  //   if (esd->testSensorProp(PROP_DT,DT_RATIONAL) ||
  //       esd->testSensorProp(PROP_DT,DT_INTEGER))
  //     mrv.push_back("ordinal");
  //   else
  //     mrv.push_back("discrete");

  // mrv.push_back("effect");
}

void mclTakeValueExp::addLinkTags(linkTags_t& crv) {
  mclPollOnExitExp::addLinkTags(crv);
  float csv  = itsMCL->sensor_value_pp(sname);
  if (csv > target)
    crv.push_back("long-of-target");
  else 
    crv.push_back("short-of-target");
  // now check to see if the sensor never changed...
  if (initial == csv)
    crv.push_back("missed-unchanged");
}

void mclAnyNetChangeExp::addLinkTags(linkTags_t& crv) {
  mclPollOnExitExp::addLinkTags(crv);
  crv.push_back("missed-unchanged");
}

void mclGoUpExp::addLinkTags(linkTags_t& crv) {
  mclPollOnExitExp::addLinkTags(crv);
  float csv  = itsMCL->sensor_value_pp(sname);
  bool unch = (csv == initial);
  if (unch)
    crv.push_back("missed-unchanged");
  else 
    crv.push_back("missed-wrongway");
}

void mclGoDownExp::addLinkTags(linkTags_t& crv) {
  mclPollOnExitExp::addLinkTags(crv);
  float csv  = itsMCL->sensor_value_pp(sname);
  bool unch = (csv == initial);
  if (unch)
    crv.push_back("missed-unchanged");
  else 
    crv.push_back("missed-wrongway");
}

void mclNoNetChangeExp::addLinkTags(linkTags_t& crv) {
  mclPollOnExitExp::addLinkTags(crv);
  float csv  = itsMCL->sensor_value_pp(sname);
  bool tlong = (csv > initial);
  if (tlong)
    crv.push_back("cwa-increase");
  else
    crv.push_back("cwa-decrease");
}

void mclLowerBoundExp::addLinkTags(linkTags_t& crv) {
  // cout << "lowerbound generating linkage tags." << endl;
  mclPollAndTestExp::addLinkTags(crv);
  crv.push_back("breakout-low");
}

void mclUpperBoundExp::addLinkTags(linkTags_t& crv) {
  // cout << "upperbound generating linkage tags." << endl;
  mclPollAndTestExp::addLinkTags(crv);
  crv.push_back("breakout-high");
}

void mclRemainLegalExp::addLinkTags(linkTags_t& crv) {
  // cout << "remainlegal generating linkage tags." << endl;
  mclPollAndTestExp::addLinkTags(crv);
  legalspec *sls = itsMCL->getLegalSpec(sname);
  if (sls)
    sls->addLinkTags(crv);
  else
    crv.push_back("unreachableMCLstate");
}

string relationToDivergenceType(int relation) {
  switch (relation) {
  case RELATION_EQUAL: 
    return "aberration";
  case RELATION_GT:
  case RELATION_GTE: 
    return "breakout-low";
  case RELATION_LT:
  case RELATION_LTE: 
    return "breakout-high";
  otherwise: return "divergence";
  }
}

void mclRatioMaintenanceExp::addLinkTags(linkTags_t& crv) {
  mclMaintenanceExp::addLinkTags(crv);
  do_sensor_provenance(crv,numerator);
  do_sensor_provenance(crv,denominator);
  crv.push_back("ratio");
  crv.push_back(relationToDivergenceType(ratioRelation));
}

void mclMaintainValueExp::addLinkTags(linkTags_t& crv) {
  // cout << "upperbound generating linkage tags." << endl;
  mclPollAndTestExp::addLinkTags(crv);
  float csv  = itsMCL->sensor_value_pp(sname);
  bool tlong = (csv > target);
  if (nochange) {
    // it's an "aberration" (not supposed to change but it did)
    if (tlong)
      crv.push_back("cwa-increase");
    else
      crv.push_back("cwa-decrease");
  }
  else {
    if (initial == csv)
      crv.push_back("missed-unchanged");
    else if (tlong)
      crv.push_back("long-of-target");
    else
      crv.push_back("short-of-target");
  }
}

void mclDelayedMaintenanceExp::addLinkTags(linkTags_t& crv) {
  sub->addLinkTags(crv);
  crv.push_back("temporal");
}
