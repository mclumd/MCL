#ifndef MCL2_ENTITY_H
#define MCL2_ENTITY_H

#include "APITypes.h"

#include <stdio.h>
#include <map>
#include <string>
#include <iostream>

/** \file
 * \brief Base classes for MCL objects.
 * includes "mclEntity", "ontologyComponent", and "frameComponent"
 * which allow access to parent MCL, Ontology, and Frame objects,
 * among other things.
 *
 * to the MCL it is linked to. this virtual function 
 *
 * the following are all the Entity subclasses and where they get access from
 *
 * mcl (self)
 * expectations (direct to mcl)
 * frame (direct to mcl)
 *   ontology (frame)
 *     nodes (ontology)
 *       links (nodes (source or dest))
 * property vector (direct to mcl)
 * sensorDef (direct to mcl)
 * HIIDef (direct to mcl)
 */

using namespace std;

namespace metacog {

  class mcl;
  class mclFrame;
  class mclOntology;
  class mclNode;
  
/** The base class of all base classes.
 *  provides access to:
 *  1) entity name
 *  2) entity key (not used so much any more)
 *  3) parent MCL
 */
class mclEntity {

 public:
  mclEntity();
  mclEntity(string entity_name);
  //! preferred.
  mclEntity(string entity_name,string class_name);

  //! implemented by instantiable class
  virtual mcl *MCL()=0;
  
  virtual ~mclEntity();

  void setEntityName(string n) {
    mcle_instance_name = n;
    final=false;
  };

  //! for name-based lookup, tests that n is the same as the entity's name.
  bool matchesName(string n);

  //! returns the appropriate class name.
  //! can be used to generate default entity names.
  //! you must override this if you are going to default to it.
  virtual string baseClassName()=0;

  //! returns a finalized MCL_entity class name.
  virtual string className()  { if (!final) finalize(); return mcle_class_name; };
  //! returns a finalized MCL_entity class name hash key.
  virtual mclKey classKey()   { if (!final) finalize(); return mcle_class_key; };

  //! returns a pre-finalized entity base name (no address included).
  virtual string entityBaseName() { return mcle_instance_name; };
  //! returns a finalized entity name, which include the object address.
  virtual string entityName() {
    if (!final) finalize(); 
    char buff[255];
    sprintf(buff,"%s-0x%08lx",mcle_instance_name.c_str(),(unsigned long int)this);
    return buff;
  };

  virtual void dumpEntity(ostream *strm) {
    cout << entityName();
  };

  virtual string describe() {
    return "(" + entityName() + ")";
  };

 private:
  //! flag indicating whether the names/defaults have been finalized.
  bool   final;
  //! a finalized class name.
  string mcle_class_name;
  //! a finalized instance name.
  string mcle_instance_name;
  //! hash key for the instance
  mclKey mcle_class_key;
  
  //! global flag -- has a key already been issued.
  static bool   alreadyIssued(mclKey k);
  //! global hash key bookeeping (sets a key in key base).
  static mclKey issueKey(string name);
  //! global hash table with keys issued and the name to which they were issued.
  static map<mclKey,string> issuedKeys;
  //! global flag indicating a collision in the keybase (bad).
  static bool collision;

  //! function for finalizing class and entity names.
  void finalize();

};


/** an mclEntity that is a component of an mclFrame.
 *  provides access to the parent mclFrame.
 */
class frameComponent : public mclEntity {
 public:
  frameComponent(string name, mclFrame *mf) 
    : mclEntity(name),my_frame(mf)
    {};

  virtual mclFrame *myFrame() { return my_frame; };
  void setFrame(mclFrame* myframe) { my_frame=myframe; };

  virtual mcl* MCL();
    
 protected:
  mclFrame *my_frame;

};

/** a frameComponent that is also a component of an ontology.
 *  (mainly links and nodes): provides access to the parent mclOntology.
 */
class ontologyComponent : public frameComponent {
 public:
  ontologyComponent(string name, mclOntology *mo);

  mclOntology *myOntology() { return my_ontology; };

  virtual mcl* MCL();
  virtual mclFrame *myFrame();
    
 protected:
  mclOntology *my_ontology;

};

class nodeComponent : public ontologyComponent {
 public:
  nodeComponent(string nae, mclNode* mn);
  mclNode* myNode() { return my_node; };
  virtual mcl* MCL();
  virtual mclFrame *myFrame();
  
 protected:
  mclNode* my_node;

};

};
#endif
