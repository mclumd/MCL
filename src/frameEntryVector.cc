#include "frameEntryVector.h"

#include <list>
#include <stdio.h>

using namespace metacog;
using namespace std;

string IIS2String(list<string>& iis) {
  string rv="{ ";
  for (list<string>::iterator lsi = iis.begin();
       lsi != iis.end();
       lsi++) {
    rv+=*lsi+" ";
  }
  rv+="}";
  return rv;
}

string PT2String(list<egkType>& ptrace) {
  char rca[128];
  string rv="{ ";
  for (list<egkType>::iterator lsi = ptrace.begin();
       lsi != ptrace.end();
       lsi++) {
    sprintf(rca,"%lx ",*lsi);
    rv+=rca;
  }
  rv+="}";
  return rv;
}

string mclFrameEntryVector::describe() {
  char rca[1024];
  string iisas = IIS2String(vIIS);
  string ptvas = PT2String(vPTrace);
  sprintf(rca,"(FEV: vEG=%lx vRef=%lx vIIS=%s vPT=%s vEVS=%s)",
	  vEG,vRef,iisas.c_str(),ptvas.c_str(),vEVS.c_str() );
  return rca;
}

string mclFrameEntryVector::string_iis() {
   return IIS2String(vIIS);
}

mclFrameEntryVector::mclFrameEntryVector(const mclFrameEntryVector& source) {
  vECode=source.vECode;
  vEG=source.vEG;
  vRef=source.vRef;
  vEVS=source.vEVS;
  for (list<string>::const_iterator lsi=source.vIIS.begin();
       lsi != source.vIIS.end(); lsi++) { vIIS.push_back(*lsi); };
  for (list<egkType>::const_iterator pli=source.vPTrace.begin();
       pli != source.vPTrace.end(); pli++) { vPTrace.push_back(*pli); };
}

mclFrameEntryVector* mclFrameEntryVector::clone() {
  return new mclFrameEntryVector(*this);
}

egkType mclFrameEntryVector::parentKey() {
  if (vPTrace.empty())
    return EGK_NO_EG;
  else return (vPTrace.front());
}

bool mclFrameEntryVector::isSuccessfulEntry() {
  return ((vECode == ENTRY_CLEAN) || (vEVS == ""));
}

bool mclFrameEntryVector::isViolationEntry() {
  return (vEVS != "");
}
