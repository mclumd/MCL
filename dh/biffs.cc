#include "direct_hit.h"

list<biff_eg> open_egks;

void open(biff_eg begk) {
  open_egks.push_front(begk);
  mclMA::declareExpectationGroup(DHK,begk);
}

void complete(biff_eg begk,mclMA::observables::update uo) {
  open_egks.remove(begk);
  mclMA::expectationGroupComplete(DHK,begk,uo);
}

void abort(biff_eg begk) {
  open_egks.remove(begk);
  mclMA::expectationGroupAborted(DHK,begk);
}

void dump_egks() {
  cout << "open egks { ";
  for (list<biff_eg>::iterator bei = open_egks.begin(); bei != open_egks.end();
       bei++) {
    cout << hex << (*bei) << " ";    
  }
  cout << "}" << endl;
}

void biff_bl_self(string oname,double* sv) {
  open(EGK_BLS);
  mclMA::declareSelfExpectation(DHK,EGK_BLS,oname,EC_STAYOVER,-1);
  *sv=-1.5;
}

void biff_bl_obj(string obj,string obs,double* sv) {
  open(EGK_BLO);
  mclMA::declareObjExpectation(DHK,EGK_BLO,obj,obs,EC_STAYOVER,-1);
  *sv=-1.5;  
}

void biff_bh_self(string oname,double* sv) {
  open(EGK_BHS);
  mclMA::declareSelfExpectation(DHK,EGK_BHS,oname,EC_STAYUNDER,1);
  *sv=1.5;
}

void biff_bh_obj(string obj,string obs,double* sv) {
  open(EGK_BHO);
  mclMA::declareObjExpectation(DHK,EGK_BHO,obj,obs,EC_STAYUNDER,1);
  *sv=1.5;  
}

void biff_illegal_self(string oname,double* sv) {
  *sv=11.5;
}

void biff_illegal_obj(string obj,string obs,double* sv) {
  *sv=-11.5;
}
