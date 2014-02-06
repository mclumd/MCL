#ifndef MCL_ONTOLOGY_H
#define MCL_ONTOLOGY_H

/** \file
 * \brief In-house MCL ontology class.
 */

#include "mclEntity.h"
#include "mclNode.h"
#include "links.h"
#include "mclProperties.h"

namespace metacog {

class mclFrame;
class mcl;

typedef list<mclNode *> nodeList;

/** The MCL in-house ontology class.
 * Basically a container of nodes with lookup functions for getting and
 * adding nodes.
 */
class mclOntology : public frameComponent {

 public:
  mclOntology(string name,mclFrame *mf) : 
    frameComponent(name,mf),oDefName("unknown") {};
  mclOntology(string name,string oDefBase,mclFrame *mf) : 
    frameComponent(name,mf),oDefName(oDefBase) {};

  string getOntologySourceName() { return oDefName; };

  void addNode(mclNode *m);

  // deprecated...
  // virtual void update();

  mclNode *findNamedNode(string n);

  virtual mcl *MCL();

  virtual void dumpEntity(ostream *strm);

  virtual string baseClassName() { return "ontology"; };

  //! returns the maximum probability node in the ontology.
  mclNode* maxPNode();

  //! automatically activates any nodes in the ontology whose value
  //! in the MCL's #currentPV() is set to #PC_YES.
  void autoActivateProperties(mclPropertyVector& mpv);

  nodeList::iterator firstNode() { return nodes.begin(); };
  nodeList::iterator endNode() { return nodes.end(); };

  //! the number of nodes in the ontology
  int size() { return nodes.size(); };

 protected:
  string oDefName;
  nodeList nodes;
  // mclFrame *myFrame;

};

};
#endif
