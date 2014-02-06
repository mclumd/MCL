#include "ontologyRecord.h"
#include "mclFrame.h"
#include "mclNode.h"
#include "mcl.h"

using namespace metacog;

ontologyRecord::ontologyRecord(mclFrame* the_frame) : 
  frameComponent(the_frame->entityName()+"rec",the_frame) {
  if (the_frame && the_frame->MCL())
    when=the_frame->MCL()->tickNumber();
}

string ontologyRecord::toString() {
  vector<string> nnv = nodeNames();
  string rv="";
  for (vector<string>::iterator ni = nnv.begin();
       ni != nnv.end();
       ni++) {
    char boof[255];
    sprintf(boof,"%s(%.3lf) ",ni->c_str(),p_for(*ni));
    rv+=boof;
  }  
  return rv;
}

////////////////////////////////////////////////////////
// now the instantiable classes for OR

ontologyRecord_map::ontologyRecord_map(mclFrame* mclf) :
  ontologyRecord(mclf) {
  vector<mclNode*> nodevec = my_frame->allNodesV();
  for (vector<mclNode*>::iterator ni = nodevec.begin();
       ni != nodevec.end();
       ni++) {
    p_map[(*ni)->entityBaseName()]=(*ni)->p_true();
  }
}

vector<string> ontologyRecord_map::nodeNames() {
  vector<string> rv;
  for (map<string,double>::iterator ni = p_map.begin();
       ni != p_map.end();
       ni++) {
    rv.push_back(ni->first);
  }
  return rv;
}

double ontologyRecord_map::p_for(string node) {
  if (p_map.find(node) != p_map.end())
    return p_map[node];
  else
#ifdef __APPLE_CC__
    return -1.0;
#else
    return ontologyRecord::P_NONE;
#endif
}
