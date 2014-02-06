#ifndef MCL2_O_NODES_H
#define MCL2_O_NODES_H

/** \file oNodes.h
 * \brief O nodes are instantiable ontology nodes. 
 * This file includes classes for all instantiable UMBC nodes. oNodes are 
 * currently built off of the bayesNode class, which is the in-house version
 * of a node that belongs to a Bayes net. Note that these nodes are all
 * wrappers for PNL nodes, which do the heavy inference lifting.
 */

#include "mclNode.h"
#include "mcl_api.h"

namespace metacog {

//! a node belonging to the MCL indication ontology.
class mclIndication : public mclNode {
 public:
  mclIndication(string cn,mclOntology *o) : mclNode(cn,o) {
  };
};

/** an indication activated directly by the host.
 * these nodes are intended to represent results from interactive/diagnostic
 * processes between the host and mcl.
 */
class mclHostInitiatedIndication : public mclIndication {
 public:
  mclHostInitiatedIndication(string cn,mclOntology *o) : 
    mclIndication(cn,o) {};
  virtual string baseClassName() { return "hostInitiatedIndication"; };
  virtual string dotDescription();
};

/** a concrete node belonging to the MCL indication ontology.
 * these nodes represent indications "activated" directly from host
 * properties and violations (no incoming bayes links)
 */
class mclConcreteIndication : public mclIndication {
 public:
  mclConcreteIndication(string cn,mclOntology *o) : mclIndication(cn,o) {};
  virtual string baseClassName() { return "concreteIndication"; };
  virtual string dotDescription();
};

/** a general node belonging to the MCL indication ontology.
 * these nodes represent abstract concepts built off of concrete nodes.
 * there is currently no differentiation between abstract indication
 * fringe nodes and indication core nodes
 */
class mclGeneralIndication : public mclIndication {
 public:
  mclGeneralIndication(string cn,mclOntology *o) : mclIndication(cn,o) {};
  virtual string baseClassName() { return "generalIndication"; };
  virtual string dotDescription();
};

class mclIndicationCoreNode : public mclGeneralIndication {
 public:
  mclIndicationCoreNode(string cn,mclOntology *o) 
    : mclGeneralIndication(cn,o) {};
  virtual string baseClassName() { return "indicationCoreNode"; };
  virtual string dotDescription();
};

//! a node belonging to the MCL failue ontology.
class mclFailure : public mclNode {
 public:
  mclFailure(string cn,mclOntology *o) : mclNode(cn,o) {
  };
  virtual string baseClassName() { return "failure"; };
  virtual string dotDescription();
};


//! a node belonging to the MCL response ontology.
class mclResponse : public mclNode {
 public:
  mclResponse(string cn,mclOntology *o) : mclNode(cn,o) {};
};

/** an abstract, non-implementable node in the MCL response ontology.
 * these abstractions connote classes of responses that are not concrete 
 * enough to be implemented by a host.
 */
class mclGeneralResponse : public mclResponse {
 public:
  mclGeneralResponse(string cn,mclOntology *o) : mclResponse(cn,o) {};
  virtual string baseClassName() { return "response"; };
  virtual string dotDescription();
};

class mclFrame;

/** a concrete, implementable node in the MCL response ontology.
 * these abstractions connote classes of responses that are not concrete 
 * enough to be implemented by a host.
 */
class mclConcreteResponse : public mclResponse {
 public:
  mclConcreteResponse(string cn,mclOntology *o,pkType crc_code) : 
  mclResponse(cn,o), response_code(crc_code), cost_static(1.0),
    interactive(false), totalAttempts(0), successfulAttempts(0),
    failedAttempts(0), abortedAttempts(0), ignoredAttempts(0), 
    successCountPad(0), failCountPad(0)
    { setPropertyTest(crc_code,PC_YES); };
  virtual string baseClassName() { return "concreteResponse"; };

  pkType  respCode() { return response_code; };
  string  respCodeExtended() { return response_extended; };
  void    setRespCodeExtended(string extcode) { response_extended=extcode; };

  virtual double cost();
  void    setCost(double x) { cost_static=x; };

  bool isInteractive() { return interactive; };
  void setInteractive() { interactive=true; };
  void setInteractive(bool inter) { interactive=inter; };  

  bool isDurative() { return durative; };
  void setDurative() { durative=true; };
  void setDurative(bool inter) { durative=inter; };  

  // this next batch of functions is for the frame to do bookkeeping at the
  // response level
  virtual void noteCleanPass(bool* stateChangeOk);
  virtual void noteJustIssued();

  void inc_totalAttempts() { totalAttempts++; };
  void inc_successfulAttempts() { successfulAttempts++; };
  void inc_failedAttempts() { failedAttempts++; };
  void inc_abortedAttempts() { abortedAttempts++; };
  void inc_ignoredAttempts() { ignoredAttempts++; };

  // meant to be overridden..
  virtual bool isNotWorking() {
    return ((abortedAttempts+failedAttempts) > (successfulAttempts+failCountPad)); 
  }
  string countsAsString();
  // mclMonitorResponse *node2response(mclFrame *f);

  virtual string dotDescription();

 private:
  pkType  response_code;
  string  response_extended;
  double  cost_static;
  bool    interactive,durative;

 protected:
  void resetCycleCounts();

  int totalAttempts,successfulAttempts,failedAttempts,
    abortedAttempts,ignoredAttempts;
  int successCountPad,failCountPad;

};

/** a specific type of concrete response that waits for a host reply
 *  this is an abstract class, nodes that require specific types of 
 *  feedback should be subclassed to express the kind of feedback expected.
 */
class mclInteractiveResponse : public mclConcreteResponse {
 public:
  mclInteractiveResponse(string cn,mclOntology *o,pkType crc_code) : 
  mclConcreteResponse(cn,o,crc_code), run_once(true)
    { setInteractive(); };

  void onlyRunOnce(bool really=true) { run_once = really; };

  virtual bool interpretFeedback(bool feedback) {
    if (run_once) set_evidence(false);
    return true;
  };
  
 protected:
  bool run_once;

};

/** an interactive response that activates and deactivates other nodes
 *  requires a true or false (yes or no) answer from the host
 *  maintains a list of nodes to assert as true or false based on the
 *  feedback.
 *  positive list asserts same value as feedback,
 *  negative list asserts the negation
 */
class mcl_AD_InteractiveResponse : public mclInteractiveResponse {
 public:
  mcl_AD_InteractiveResponse(string cn,mclOntology *o,pkType crc_code) : 
    mclInteractiveResponse(cn,o,crc_code)
    { setInteractive(); };

  virtual bool interpretFeedback(bool feedback);

  void addPositive(string node) { positiveList.push_back(node); };
  void addNegative(string node) { negativeList.push_back(node); };

 protected:
  vector<string> positiveList;
  vector<string> negativeList;

};

/** a special off-ontology node representing a host property.
 * host properties are intended to affect reasoning through the conditional
 * probability tables of failure and response nodes.
 * \sa mclHostCapability
 */
class mclHostProperty : public mclNode {
 public:
  mclHostProperty(string cn,mclOntology *o,pkType pkey) : 
    mclNode(cn,o),property_code(pkey) {
  };
  virtual string baseClassName() { return "hostProperty"; };
  pkType propCode() { return property_code; };
  
  virtual string dotDescription();

 private:
  pkType  property_code;

};

/** a special off-ontology node representing a host capability.
 * host capabilities are intended to affect reasoning through the conditional
 * probability tables of response nodes.
 * \sa mclHostProperty
 */
class mclHostCapability : public mclNode {
 public:
  mclHostCapability(string cn,mclOntology *o) : mclNode(cn,o) {
  };
};

};
#endif
