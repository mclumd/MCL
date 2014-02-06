#include "hgGlue.h"
#include "mclFrame.h"
#include "mclLogging.h"
#include "../include/umbc/settings.h"

using namespace metacog;

// GLUE IMPLEMENTATION HOME-GROWN ULTRA[incomplete]-NAIVE INFERENCE IN MCL

////////////////////////////////////////////////////////////////
/// HG NODE GLUE

hgNodeGlue::~hgNodeGlue() {
  delete cp_table;
}

hgNodeGlue::hgNodeGlue(string name, mclNode* node) :
  nodeGlue(name,node),cp_table(NULL),hardVal(0.0),hardSet(false) {
  myFrameGlue = static_cast<hgFrameGlue*>(myFrame()->getGlue(HG_GLUE_IDENTIFIER));
}

hgFrameGlue* hgNodeGlue::getFrameGlue() {
  return myFrameGlue;
}

bool   hgNodeGlue::mpv() {
  return (p_true() >= 0.50);
}

double hgNodeGlue::p_bool(bool v) {
  if (hardSet)
    return hardVal;
  else if (cp_table != NULL) {
    return cp_table->p();
  }
  else {
    ensure_cpt();
    return cp_table->p();
  }
}

void hgNodeGlue::mpd(double* v) {
  v[0]=p_true();
  v[1]=1-v[0];
}

void hgNodeGlue::set_evidence(bool v) {
  setProbHard((v ? 1.0 : 0.0));
}

void hgNodeGlue::setProbHard(double p) {
  hardSet=true;
  hardVal=p;
}

void hgNodeGlue::unsetProbHard() {
  hardSet=false;
}

bool hgNodeGlue::initPriors(vector<string>& d,vector<double>& v) {
  if (umbc::settings::debug) {
    *umbc::uLog::log << "[mcl/hgGlue]::initializing priors: " 
		     << this->entityName() << endl;
  }
  ensure_cpt();
  cp_table->initializeVector(v);
  return true;
}

void hgNodeGlue::writeCPTtoArray(double* a) {
  if (cp_table == NULL) {
    // cout << "trying to write to array " << myNode()->entityBaseName()
    // << " but cpt is null!!" << endl;
    ensure_cpt();
  }
  else {
    // cout << "okay, cpt " << myNode()->entityBaseName()
    // << " is not null." << endl;
  }
  for (int i = 0;i<cp_table->size();i++)
    a[i]=cp_table->cpt_val(i);
  // memcpy(a,table,t_size*sizeof(double));
}

void hgNodeGlue::ensure_cpt() {
  if (cp_table == NULL) {
    cp_table = new CPT(myNode()->inLinkL());
    // cout << "new table created for " << myNode()->entityBaseName()
    // << " size=" << cp_table->size() << endl;
  }
  else if (cp_table->mismatch()) {
    *umbc::uLog::log << "[mcl/hgGlue]:: warning: cp_table is inconsistent across ensure_cpt calls and is being rebuilt." << endl;
    delete cp_table;
    cp_table = new CPT(myNode()->inLinkL());
  }
}


////////////////////////////////////////////////////////////////
/// HG FRAME GLUE

hgFrameGlue::~hgFrameGlue() {
}

bool hgFrameGlue::build() {
  // all we really need is to hang an hgNodeGlue on each node

  for (int oi = 0; oi < 3; oi++) {
    mclOntology* to = myFrame()->getCoreOntology(oi);
    for (nodeList::iterator toni = to->firstNode();
	 toni != to->endNode();
	 toni++) {
      (*toni)->addGlue(HG_GLUE_IDENTIFIER,
		       new hgNodeGlue((*toni)->entityBaseName()+"-HGglue",
				      (*toni)));
    }
  }

  return true;
}
