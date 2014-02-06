#include "mclEntity.h"
#include "mclOntology.h"
#include "mclFrame.h"
#include "keyGen.h"

using namespace metacog;

map<mclKey,string> mclEntity::issuedKeys;
bool mclEntity::collision=false;

void mclEntity::finalize() {
  if (mcle_class_name.length() == 0) {
    mcle_class_name = baseClassName();
    mcle_class_key  = issueKey(mcle_class_name);
  }
  if (mcle_instance_name.length() == 0) {
    mcle_instance_name = "g_";
    mcle_instance_name.append(mcle_class_name);
  }
  final=true;
}

mclEntity::mclEntity() : final(false) {
}

mclEntity::mclEntity(string entity_name) : 
  final(false),mcle_instance_name(entity_name) {
}

mclEntity::mclEntity(string entity_name,string class_name) : 
  final(true), mcle_class_name(class_name), mcle_instance_name(entity_name),
  mcle_class_key(issueKey(mcle_class_name)) { }

mclEntity::~mclEntity() {};

/////////////////////////////////////////////////////////////
/// keygen stuff

bool mclEntity::alreadyIssued(mclKey key) {
  map<mclKey,string>::iterator iter = issuedKeys.begin();
  while (iter != issuedKeys.end()) {
    if (iter->first == key)
      return true;
    iter++;
  }
  return false;
}

mclKey mclEntity::issueKey(string q) {
  int hk=computeKey(q);
  while (alreadyIssued(hk) && (q.compare(issuedKeys[hk])!=0)) {
    cerr << "WARNING: " << q << " collides with " << issuedKeys[hk] << " at "
	 << hex << hk << endl;
    hk++;
    hk &= 0x0FFFFFFF;
    collision=true;
  }
  issuedKeys[hk]=q;
  return hk;
}

bool mclEntity::matchesName(string name) {
  return ((entityBaseName() == name) ||
	  (entityName() == name));
}

/////////////////////////////////////////////////////////////
/// 

mcl* frameComponent::MCL() { return my_frame->MCL(); }

/// ontology component

ontologyComponent::ontologyComponent(string name, mclOntology *mo) :
  frameComponent(name,mo->myFrame()),my_ontology(mo) {}

mcl* ontologyComponent::MCL() { return myOntology()->MCL(); }

mclFrame* ontologyComponent::myFrame() {
  if (my_frame) return my_frame;
  else return myOntology()->myFrame();
}

/// node component

nodeComponent::nodeComponent(string name, mclNode* mn) :
  ontologyComponent(name,mn->myOntology()),my_node(mn) {}

mcl* nodeComponent::MCL() { return myNode()->MCL(); }

mclFrame* nodeComponent::myFrame() {
  if (my_frame) return my_frame;
  else return myNode()->myFrame();
}
