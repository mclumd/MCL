#include "mclNode.h"
#include "mcl.h"
#include "mclOntology.h"
#include "mclLogging.h"
#include "../include/umbc/settings.h"

using namespace metacog;
using namespace umbc;

mcl *mclNode::MCL() { return myOntology()->MCL(); }

bool mclNode::propertiesPassTest() {
  for (map<pkType,pvType>::iterator pmi = propertyTestVector.begin();
       pmi != propertyTestVector.end();
       pmi++) {
    // if fail, return false
    if (!myFrame()->getPV()->testProperty(pmi->first,pmi->second)) {
      return false;
    }
    else { }
  }
  return true;
}

void mclNode::addIncomingLink(mclLink *l)
{ inLinks.push_back(l); };

void mclNode::addOutgoingLink(mclLink *l)
{ outLinks.push_back(l); };

bool mclNode::mpv(string glueKey) {
  // get the p_true() value from the default node glue
  return getGlue(glueKey)->mpv();
}

bool mclNode::mpv() {
  // get the p_true() value from the default node glue
  return getGlue(myFrame()->defaultGlueKey())->mpv();
}

double mclNode::p_true(string glueKey) {
  // get the p_true() value from the default node glue
  if (getGlue(glueKey))
    return getGlue(glueKey)->p_true();
  else {
    umbc::uLog::annotate(MCLA_ERROR,"node "+entityName()+" is missing glue.");
    return 0;
  }
}

double mclNode::p_true() {
  // get the p_true() value from the default node glue
  if (myFrame())
    return p_true(myFrame()->defaultGlueKey());
  else {
    if (!settings::getSysPropertyBool("mcl.ignoreMissingFrameref",false))
      umbc::uLog::annotate(MCLA_ERROR,"node "+entityName()+" is missing frameref.");
    return 0;
  }
}

double mclNode::p_false(string glueKey) {
  // get the p_false() value from the default node glue
  return getGlue(glueKey)->p_false();
}

double mclNode::p_false() {
  // get the p_false() value from the default node glue
  return getGlue(myFrame()->defaultGlueKey())->p_false();
}

double mclNode::p_bool(string glueKey,bool b) {
  // get the p_bool() value from the default node glue
  return getGlue(glueKey)->p_bool(b);
}

double mclNode::p_bool(bool b) {
  // get the p_bool() value from the default node glue
  return getGlue(myFrame()->defaultGlueKey())->p_bool(b);
}

void   mclNode::mpd(string glueKey, double* d) {
  // get the mpd() value from the default node glue
  getGlue(glueKey)->mpd(d);
}

void   mclNode::mpd(double* d) {
  // get the mpd() value from the default node glue
  cout << "[MPD]: myFrame " << myFrame() << endl;
  cout << "[MPD]: defGlueKey " << myFrame()->defaultGlueKey() << endl;
  
  getGlue(myFrame()->defaultGlueKey())->mpd(d);
}

void   mclNode::set_evidence(string glueKey,bool b) {
  // get the mpd() value from the default node glue
  getGlue(glueKey)->set_evidence(b);
}

void   mclNode::set_evidence(bool b) {
  // set evidence for ALL GLUE INSTANCES!!
  for (map<string, nodeGlue*>::iterator np = nodeGlueMapper.begin();
       np != nodeGlueMapper.end();
       np++) {
    (np->second)->set_evidence(b);
  }
}

bool   mclNode::initPriors(vector<string>& d,vector<double>& b) {
  // init priors for ALL GLUE INSTANCES!!
  for (map<string, nodeGlue*>::iterator np = nodeGlueMapper.begin();
       np != nodeGlueMapper.end();
       np++) {
    // cout << "init priors: " << entityBaseName() << "/"
    // << (np->second)->entityBaseName() << endl;
    (np->second)->initPriors(d,b);
  }  
  return true;
}

void mclNode::writeCPTtoArray(string key,double* a) {
  getGlue(key)->writeCPTtoArray(a);  
}

void mclNode::writeCPTtoArray(double* a) {
  getGlue(myFrame()->defaultGlueKey())->writeCPTtoArray(a);  
}

string mclNode::dotDescription() {
  string bd = entityBaseName()+" [shape=ellipse];";
  std::replace( bd.begin(), bd.end(), '-', '_' );
  return bd;
}

string mclNode::dotLabel() {
  char pv[16];
  sprintf(pv,"(%1.2f)",p_true());
  string bd = "\""+entityBaseName()+pv+"\"";
  std::replace( bd.begin(), bd.end(), '-', '_' );
  return bd;
  // return entityBaseName();
}
