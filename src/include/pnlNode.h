#ifndef MCL2_PNL_NODES_H
#define MCL2_PNL_NODES_H

#include "bayesNode.h"

#define PNL_NODE_UNITITALIZED -1

class pnlNode : public bayesNode {
 public:
  pnlNode(string cn, mclOntology *o) :
    bayesNode(cn,o),pnlNodeNumber(PNL_NODE_UNINITIALIZED) {};
  
  void setPNLnodeNumber(int n) { pnlNodeNumber=n; };

 protected:
  int pnlNodeNumber;
  

};

#endif
