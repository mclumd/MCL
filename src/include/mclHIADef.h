#ifndef HIA_DEF_H
#define HIA_DEF_H

/** \file
 * \brief classes for Host Initiated Anomalies.
 */

namespace metacog {

  class mcl;

class mclHIADef : public mclEntity {
 public:
  /** constructor
   * @param name the name of the HIA being defined
   * @param failureNodeName the name of the ontology node to be activated when the HIA is signaled
   * @param m the MCL to which the HIA belongs
   */
  mclHIADef(string name,string failureNodeName,mcl *m) : 
    mclEntity(name),targetNode(failureNodeName),myMCL(m) {};

  virtual string baseClassName() { return "HIADef"; };
  virtual mcl   *MCL() { return myMCL; };

  //! returns the name of the ontology node to be activated on HIA signal.
  string targetNodeName() { return targetNode; };

 protected:
  //! this is the name of the node that will be "activated" when the HIA
  //! is signaled.
  string targetNode;

  mcl   *myMCL;

};

};
#endif

