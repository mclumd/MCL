#include "linkFactory.h"
#include "mclFrame.h"
#include "../include/umbc/exceptions.h"

using namespace metacog;

bool linkFactory::makeIFCLink(mclOntology *o, string sname,string dname) {
  mclNode *sn = o->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = o->findNamedNode(dname);
    if (dn != NULL) {
      IFCLink *tl = new IFCLink("IFCLink",o,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
      // o->myFrame()->addBayesArc(sn,dn);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeIFCLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeIFCLink.");
  }
  return true;
}

bool linkFactory::makeSpecificationLink(mclOntology *o, string sname,string dname) {
  mclNode *sn = o->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = o->findNamedNode(dname);
    if (dn != NULL) {
      specificationLink *tl = new specificationLink("specifiesLink",o,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
      // o->myFrame()->addBayesArc(sn,dn);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeSpecificationLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeSpecificationLink.");
  }
  return true;
}

bool linkFactory::makeAbstractionLink(mclOntology *o, string sname,string dname) {
  mclNode *sn = o->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = o->findNamedNode(dname);
    if (dn != NULL) {
      abstractionLink *tl = new abstractionLink("abstractionLink",o,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
      // o->myFrame()->addBayesArc(sn,dn);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeAbstractionLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeAbstractionLink.");
  }
  return true;
}

bool linkFactory::makeDiagnosticLink(mclOntology *srco,string sname,
				     mclOntology *dnco,string dname) {
  mclNode *sn = srco->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = dnco->findNamedNode(dname);
    if (dn != NULL) {
      interontologicalLink *tl = new diagnosticLink("diagnosis",srco,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
      // srco->myFrame()->addBayesArc(sn,dn);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeDiagnosticLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeDiagnosticLink.");
  }
  return true;  
}

bool linkFactory::makeInhibitoryLink(mclOntology *srco,string sname,
				     mclOntology *dnco,string dname) {
  mclNode *sn = srco->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = dnco->findNamedNode(dname);
    if (dn != NULL) {
      interontologicalLink *tl = new inhibitoryLink("inhibit",srco,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeInhibitoryLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeInhibitoryLink.");
  }
  return true;  
}


bool linkFactory::makeSupportLink(mclOntology *srco,string sname,
				     mclOntology *dnco,string dname) {
  mclNode *sn = srco->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = dnco->findNamedNode(dname);
    if (dn != NULL) {
      interontologicalLink *tl = new supportingLink("support",srco,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makeSupportLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makeSupportLink.");
  }
  return true;  
}

bool linkFactory::makePrescriptiveLink(mclOntology *srco,string sname,
				       mclOntology *dnco,string dname) {
  mclNode *sn = srco->findNamedNode(sname);
  if (sn != NULL) {
    mclNode *dn = dnco->findNamedNode(dname);
    if (dn != NULL) {
      interontologicalLink *tl = new prescriptiveLink("prescription",srco,sn,dn);
      sn->addOutgoingLink(tl);
      dn->addIncomingLink(tl);
      // srco->myFrame()->addBayesArc(sn,dn);
    }
    else {
      umbc::exceptions::signal_exception("Could not find "+dname+" in makePrescriptiveLink.");
    }
  }
  else {
    umbc::exceptions::signal_exception("Could not find "+sname+" in makePrescriptiveLink.");
  }
  return true;  
}
