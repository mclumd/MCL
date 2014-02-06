#ifndef MCL2_MCL_NODE_H
#define MCL2_MCL_NODE_H

/** \file 
 * \brief The base class for the represnetation of an ontology node.
 * mclNodes define the structure/connectivity of the ontologies.
 * inference and probabilistic reasoning are provided by creating glue
 * objects that correspond to implementations of the underlying probabilistic
 * functionality being provided.
 */

#include <list>
#include <iostream>
#include "mclEntity.h"
#include "common.h"
#include "abstractGlue.h"

namespace metacog {

  class mclOntology;
  class mcl;
  class mclLink;

/** The base class for in-house ontology nodes.
 */
class mclNode : public ontologyComponent, public hasNodeGlue {
 public:
  /** constructor.
   * @param cn the Common Name of the node
   * @param o pointer to the ontology to which the node will belong.
   */
  mclNode(string cn,mclOntology *o) : 
    ontologyComponent(cn,o),hasNodeGlue() { };
  
  // linkage...
  virtual void addIncomingLink(mclLink *l);
  virtual void addOutgoingLink(mclLink *l);
  
  llIterator inLink_begin()  { return inLinks.begin();  };
  llIterator inLink_end()    { return inLinks.end();  };
  llIterator outLink_begin() { return outLinks.begin(); };
  llIterator outLink_end()   { return outLinks.end(); };
  linkList* inLinkL() { return &inLinks; };

  //! gets in incoming link by index into the vector (for quick iteration).
  mclLink    *getInLink(int i) { return inLinks[i]; };

  int inLinkCnt() { return inLinks.size(); };
  int outLinkCnt() { return outLinks.size(); };

  /** sets a property test (requirement) on the node.
   * Property tests express the host property values 
   * (see #mclPropertyVector) required for node activation.
   * @param key the key value for the required property
   * @param val the value required for activation
   */
  void setPropertyTest(pkType key,pvType val) { propertyTestVector[key]=val; };
  
  //  probabilistic reasoning helper monkeys
  //! most probable value
  bool   mpv(string glueKey);
  bool   mpv();
  //! P that the node is true
  double p_true(string glueKey);
  double p_true();
  //! P that the node is false
  double p_false(string glueKey);
  double p_false();
  //! P of supplied value
  double p_bool(string glueKey,bool b);
  double p_bool(bool b);
  //! MPD for a node
  void   mpd(string glueKey,double* d);
  void   mpd(double* d);
  //! setting evidence
  void   set_evidence(string glueKey,bool b);
  void   set_evidence(bool b);
  
  void   writeCPTtoArray(string key,double* a);
  void   writeCPTtoArray(double* a);
  bool   initPriors(vector<string>&,vector<double>&);

  //! sets the documenation string for a node.
  void   document(string ds) { documentation = ds; };
  //! returns the documentation string for a node.
  string getDocumentation() { return documentation; };

  // formalities...
  virtual mcl *MCL();
  virtual void dumpEntity(ostream *o) { 
     *o << "node(" << entityName() << ",[def]p=" << p_true() 
	<< ", [def]~p=" << p_false() << ")" << endl; 
  };

  //! ensures that the properties necessary for node activation are passed.
  virtual bool propertiesPassTest();  

  //! a method for outputting dot node descriptions
  virtual string dotDescription();
  //! a method for outputting dot node descriptions
  virtual string dotLabel();

 protected:

  linkList inLinks,outLinks;
  string documentation;
  map<pkType,pvType> propertyTestVector;

};

};

#endif
