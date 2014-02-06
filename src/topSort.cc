#include "topSort.h"
#include "mclNode.h"
#include "../include/umbc/exceptions.h"
#include "links.h"

using namespace metacog;

bool _mclts_movable(vector<mclNode*>&, int pivot, int index);

bool topsort::topSort(vector<mclNode*>& invec) {
  size_t pivot=0;
  mclNode* tmp;
  while (pivot < invec.size()) {
    bool prog=false;
    for (size_t i=pivot; i < invec.size(); i++) {
      if (_mclts_movable(invec,pivot,i)) {
	if (i != pivot) {
	  /*
	    cout << "moving " << "(" << i << "," << pivot << ")"
	    << invec[i]->entityBaseName()
	    << " to " << pivot << endl;
	  */
	  tmp = invec[pivot];
	  invec[pivot]=invec[i];
	  invec[i]=tmp;
	}
	else {
	  /*
	    cout << "leaving " << invec[i]->entityBaseName()
	    << " where it is (" << i << ")" << endl;
	  */
	}
	prog=true;
	pivot++;
      }
    }
    if (!prog) {
      umbc::exceptions::signal_exception("[topSort]: graph has a cycle.");      
    }
  }
  return true;
}

bool _mclts_movable(vector<mclNode*>& invec, int pivot, int index) {
  mclNode* candidate = invec[index];
  for (llIterator ini = candidate->inLink_begin();
       ini != candidate->inLink_end();
       ini++) {
    bool matched=false;
    for (int q = 0; ((q < pivot) && !matched); q++) {
      if (invec[q] == (*ini)->sourceNode())
	matched = true;
    }
    if (!matched)
      return false;
  }
  return true;
}
