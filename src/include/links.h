#ifndef MCL2_LINKS_H
#define MCL2_LINKS_H

#include "mclEntity.h"
#include "mclNode.h"

/** \file
 *  \brief link classes for homegrown ontologies.
 */

namespace metacog {

  //!! general class of ontology links.

  class mclLink : public ontologyComponent {

  public:

  //! returns a pointer to the link destination.
  mclNode *destinationNode() { return dest; };
  //! returns a pointer to the link source.
  mclNode *sourceNode() { return src; };

  //! returns true if the link propogates influence.
  //! @deprecated no longer using a spreading activation model
  virtual bool spreadsActivation()=0;

  virtual mcl *MCL() { return dest->MCL(); };

  //! another influence-propogation function.
  //! @deprecated no longer relevant as it is a spreading activation parameter.
  virtual double weight() { return 1.0; };

  //! function to support generation of dot files
  virtual string dotDescription();

 protected:
  mclLink(string cn,mclOntology *o,mclNode *s,mclNode *d) : 
    ontologyComponent(cn,o),src(s),dest(d) { };
  mclNode *src, *dest;

};

/** the class of intra ongological links.
 */
class intraontologicalLink : public mclLink {
 public:

  virtual bool spreadsActivation() { return true; };

 protected:
  intraontologicalLink(string cn,mclOntology *o,mclNode *s,mclNode *d) : 
    mclLink(cn,o,s,d) {};
};

/** the class of inter ongological links.
 */
class interontologicalLink : public mclLink {
 public:
  // should probably be moved lower in the hierarchy...
  virtual bool spreadsActivation() { return true; };
  virtual string baseClassName() { return "interLink"; };
  virtual string dotDescription();

 protected:
  interontologicalLink(string cn,mclOntology *o,mclNode *s,mclNode *d) : 
    mclLink(cn,o,s,d) {};

};

/** the class of links that connote abstraction from source to dest.
 */
class abstractionLink : public intraontologicalLink {

 public:
  abstractionLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    intraontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "abstraction"; };

};

/** the class of links that connect Indication Fringe nodes to Indication Core nodes.
 */
class IFCLink : public intraontologicalLink {

 public:
  IFCLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    intraontologicalLink(cn,o,s,d) { 
    //printf("creating IFC 0x%08x -> 0x%08x @ 0x%08x\n",s,d,this); 
  };
  virtual string baseClassName() { return "indicationFringeCoreLink"; };

};

/** the class of links that connote specification from source to dest.
 */
class specificationLink : public intraontologicalLink {

 public:
  specificationLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    intraontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "specification"; };

};

/** the class of links that connote diagnosis of failure from indications.
 *  these links connect Indication nodes to Failure nodes.
 */
class diagnosticLink : public interontologicalLink {

 public:
  diagnosticLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    interontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "diagnosis"; };

};

/** class of links that connote inhibition of a response from an indication.
 *  these links connect Indication nodes to Response nodes.
 */
class inhibitoryLink : public interontologicalLink {

 public:
  inhibitoryLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    interontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "inhibitory"; };

};

/** the class of links that connote support for a response from an indication.
 *  these links connect Indication nodes to Response nodes.
 */
class supportingLink : public interontologicalLink {

 public:
  supportingLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    interontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "supports"; };

};

/** the class of links that connote prescription of a response from a failure.
 *  these links connect Failure nodes to Response nodes.
 */
class prescriptiveLink : public interontologicalLink {

 public:
  prescriptiveLink(string cn,mclOntology *o,mclNode *s,mclNode *d) :
    interontologicalLink(cn,o,s,d) {};
  virtual string baseClassName() { return "prescription"; };

};

};
#endif
