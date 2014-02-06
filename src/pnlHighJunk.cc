// FROM mclFrame.h

#include "pnlHigh.hpp"

using namespace pnl;

PNLW_USING

   //! an openPNL high BayesNet to mirror the 3 homegrown ontologies
   BayesNet             coreNet;

   //! PNL interoperability code.
   void addBayesNode(mclNode *m);
   //! PNL interoperability code.
   void addBayesArc(mclNode *s,mclNode *d);
   //! PNL interoperability code.
   BayesNet* getBayesNet() { return &coreNet; };
   //! PNL interoperability code.
   void   BayesEvidenceFor(mclNode *m); 
   //! PNL interoperability code.
   void   BayesEvidenceAgainst(mclNode *m); 
   //! PNL interoperability code.
   bool   BayesTF(mclNode *m);
   //! PNL interoperability code.
   double BayesP(mclNode *m) { return BayesP(m,true); };
   //! PNL interoperability code.
   double BayesP(mclNode *m,bool val);
   //! PNL interoperability code.
   string BayesMPE(mclNode *m);
   //! PNL interoperability code.
   string BayesJPD(mclNode *m);
   //! PNL interoperability code.
   string BayesCPT(mclNode *m);

///////////////PNL API
// FROM mclFRAME.cc

PNLW_USING

void mclFrame::addBayesNode(mclNode *m) {
  TokArr bnn("discrete^"+m->pnlName());
  coreNet.AddNode(bnn,"true false");
}

void mclFrame::addBayesArc(mclNode *s,mclNode *d) {
  TokArr snn(s->pnlName()),dnn(d->pnlName());
  coreNet.AddArc(snn,dnn);
}

void mclFrame::BayesEvidenceFor(mclNode *s) {
  TokArr evidence(s->pnlName()+"^true");
  coreNet.EditEvidence(evidence);
}

void mclFrame::BayesEvidenceAgainst(mclNode *s) {
  TokArr evidence(s->pnlName()+"^false");
  coreNet.EditEvidence(evidence);
}

string mclFrame::BayesMPE(mclNode *s) {
  TokArr v(s->pnlName());
  TokArr q = coreNet.GetMPE(v);
  const char* x = q[0].Name().c_str();
  string xs = x;
  return xs;
}

bool mclFrame::BayesTF(mclNode *s) {
  TokArr v(s->pnlName());
  TokArr q = coreNet.GetMPE(v);
  if (strcmp(q[0].Name().c_str(),"true")==0)
    return true;
  else
    return false;
}

double mclFrame::BayesP(mclNode *s,bool val) {
  TokArr v(s->pnlName());
  TokArr q = coreNet.GetJPD(v);
  if (val)
    return (double)(q[0].FltValue());
  else
    return (double)(q[1].FltValue());    
}

string mclFrame::BayesJPD(mclNode *s) {
  TokArr v(s->pnlName());
  TokArr q = coreNet.GetJPD(v);
  const char* qcp = String(q).c_str();
  string qs = qcp;
  return qs;
}

string mclFrame::BayesCPT(mclNode *s) {
  TokArr v(s->pnlName());
  TokArr q = coreNet.GetPTabular(v);
  const char* qcp = String(q).c_str();
  string qs = qcp;
  return qs;
}

// FROM mclnode.h

  //! asserts that the node was observed to be true.
  virtual void   assertTrue() { hg_setProbabilityHard(1.0); };
  //! asserts that the node was observed to be false.
  virtual void   assertFalse() { hg_setProbabilityHard(0.0); };
  virtual double hg_getProbability()=0;
  virtual void   hg_setProbabilityHard(double p)=0;
  //! the P value of the node.
  //! @return the probability that the node's value is TRUE
  virtual double P()=0;
  //! the P value of a truth value for the node.
  //! @return the probability that the node's value is 'val'
  virtual double P(bool val)=0;
  //! returns the True/False probability distribution.
  virtual bool   TF()=0;
  //! Maximum Probability Explanation.
  //! @return a string with the MPE node name
  virtual string mpe()=0;
  //! Joint Probability Distribution.
  //! @return a string expressing the joint probability distribution of a node (the format will be dependant on the internal representation of the node)
  virtual string jpd()=0;
  //! Conditional Probability Table
  //! @return a string expressing the conditional probability distribution of a node (the format will be dependant on the internal representation of the node)
  virtual string cpt()=0;
  virtual void   hg_unSetHard()=0;
  //! Performs an update of the node's probability value(s).
  //! @deprecated should not be necessary under forseeable implementations.
  virtual void   update()=0;
