#include "direct_hit.h"
#include "mcl/mcl_symbols.h"
#include "umbc/settings.h"
#include "umbc/text_utils.h"
#include "umbc/token_machine.h"

using namespace umbc;

void observe_obj(mclMA::observables::update& uo,double osv,bool* objup) {
  mclMA::observables::notice_object_observable(DHK,OBJT,OBJN);
  uo.set_update(SSN,osv);
  mclMA::updateObservables(DHK,uo);
  mclMA::declareObjExpectation(DHK,EGK_BASE_LEVEL,OBJN,OBJS,EC_BE_LEGAL);
  *objup = true;
}

void print_help() {
  cout << "drv: dump most recent response vector" << endl;
  cout << "bov: toggle break-on-violation flag" << endl;
  cout << "fix: set sensor values to legal levels" << endl;
  cout << "fxx: fix and respond suggestionImplemented to MRR" << endl;
  cout << "egc: declare expectation group complete" << endl;
  cout << "ega: declare expectation group aborted" << endl;
  cout << "ign: declare suggestion ignored (MRR)" << endl;
  cout << "dec: declare suggestion declined (MRR)" << endl;
  cout << "imp: declare suggestion implemented (MRR)" << endl;
  cout << "fail:declare suggestion failed (MRR)" << endl;
  cout << "ils: cause illegal self-sensor violation" << endl;
  cout << "ilo: cause illegal object-sensor violation" << endl;
  cout << "bls: cause self-sensor breakout-low violation" << endl;
  cout << "blo: cause object-sensor breakout-low violation" << endl;
  cout << "bhs: cause self-sensor breakout-high violation" << endl;
  cout << "bho: cause object-sensor breakout-high violation" << endl;
  cout << "oo : observe object" << endl;
  cout << "uo : unobserve object" << endl;
  cout << "q  : quit" << endl;
}

int main(int argc, char **argv) {
  mclMA::observables::update uo;
  double ssv=0.0,osv=0.0;
  uo.set_update(SSN,ssv);

  mclMA::initializeMCL(DHK,0);
  mclMA::configureMCL(DHK,"direct_hit");

  mclMA::observables::declare_observable_self(DHK,SSN);
  mclMA::observables::set_obs_prop_self(DHK,SSN,PROP_SCLASS,SC_STATE);
  mclMA::observables::set_obs_prop_self(DHK,SSN,PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_noiseprofile_self(DHK,SSN,MCL_NP_UNIFORM,0.1);
  mclMA::observables::set_obs_legalrange_self(DHK,SSN,-10,10);
  
  mclMA::observables::declare_observable_object_type(DHK,OBJT);
  mclMA::observables::declare_object_type_field(DHK,OBJT,OBJS);
  mclMA::observables::set_obs_prop(DHK,OBJT,OBJS,PROP_SCLASS,SC_STATE);
  mclMA::observables::set_obs_prop(DHK,OBJT,OBJS,PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_legalrange_def(DHK,OBJT,OBJS,-10,10);
  mclMA::observables::set_obj_obs_noiseprofile(DHK,OBJT,OBJS,
					       MCL_NP_UNIFORM,0.1);

  // base level eg + legality expectations....
  mclMA::declareExpectationGroup(DHK,EGK_BASE_LEVEL);
  mclMA::declareSelfExpectation(DHK,EGK_BASE_LEVEL,SSN,EC_BE_LEGAL);

  umbc::settings::setSysProperty("mcl.breakOnDLinkFailure",false);
  umbc::settings::setSysProperty("mcl.breakOnViolation",false);

  char buff[128];
  buff[0]='\0';
  bool oio = false;
  cout << "\nThis is the DirectHit MCL/so testbed.\n" << endl;
  responseVector rv;
  while (strcmp(buff,"exit") != 0) {
    cout << "\033[0;37;44mDH :>\033[0;34;49m ";
    cin.getline(buff,127);
    cout << "\033[0;39;49m";
    tokenMachine tm((string)buff);
    string cmd = tm.nextToken();
    if (strlen(buff) == 0) {
      if (oio) uo.set_update(OBJN,OBJS,osv);
      uo.set_update(SSN,ssv);
      rv = mclMA::monitor(DHK,uo);
      if (rv.empty())
	cout << "no responses from MCL." << endl;
      else 
	for (responseVector::iterator rvi = rv.begin(); rvi != rv.end(); rvi++) {
	  cout << (*rvi)->to_string() << endl;
	}      
    }
    else if ((strcmp(buff,"h") == 0) || (strcmp(buff,"help") == 0)) {
      print_help();
    }
    else if (strcmp(buff,"drv") ==0) {
      if (rv.empty()) 
	cout << "empty." << endl;
      else
	for (responseVector::iterator rvi = rv.begin(); rvi != rv.end(); rvi++) {
	  cout << (*rvi)->to_string() << endl;
      }
    }
    else if (strcmp(buff,"bov") ==0) {
      umbc::settings::setSysProperty("mcl.breakOnViolation",
				     !umbc::settings::getSysPropertyBool("mcl.breakOnViolation",true));
      if (umbc::settings::getSysPropertyBool("mcl.breakOnViolation",true))
	cout << "break on violation is ON" << endl;
      else
	cout << "break on violation is OFF" << endl;
    }
    else if (strcmp(buff,"fix") ==0) {
      ssv=0; osv=0;
      cout << "sensors set to 0" << endl;
    }
    else if (strcmp(buff,"fxx") ==0) {
      ssv=0; osv=0;
      if (!rv.empty())
	mclMA::suggestionImplemented(DHK,rv[0]->referenceCode());
    }
    else if (strcmp(buff,"ign") ==0) {
      if (!rv.empty()) {
	mclMA::suggestionIgnored(DHK,rv[0]->referenceCode());
      }
    }
    else if (cmd == "dec") {
      long v=-1;
      if (tm.moreTokens()) {
	string arg = tm.nextToken();
	v = textFunctions::longval(arg);
      }
      else if (!rv.empty())
	v=rv[0]->referenceCode();
      
      if (v > -1) {
	mclMonitorResponse* nr =
	  mclMA::suggestionDeclined(DHK,rv[0]->referenceCode());
	// memory leak?
	rv.clear();
	rv.push_back(nr);
	cout << nr->to_string() << endl;
      }
      else 
	cout << "no reference and rv is empty." << endl;

    }
    else if (strcmp(buff,"fail") ==0) {
      if (!rv.empty())
	mclMA::suggestionFailed(DHK,rv[0]->referenceCode());
    }
    else if (strcmp(buff,"imp") ==0) {
      if (!rv.empty())
	mclMA::suggestionImplemented(DHK,rv[0]->referenceCode());
    }
    else if (strcmp(buff,"egc") ==0) {
      if (oio) uo.set_update(OBJN,OBJS,osv);
      uo.set_update(SSN,ssv);
      complete(open_egks.front(),uo);
    }
    else if (strcmp(buff,"ega") ==0) {
      abort(open_egks.front());
    }
    else if ((strcmp(buff,"q") == 0) || (strcmp(buff,"quit") == 0)) {
      exit(0);
    }
    else if ((strcmp(buff,"oo") == 0)) {
      if (!oio)
	observe_obj(uo,osv,&oio);
    }
    else if ((strcmp(buff,"uo") == 0)) {
      mclMA::observables::notice_object_unobservable(DHK,OBJN);
      oio = false;
    }
    else if ((strcmp(buff,"ils") == 0)) {
      biff_illegal_self(SSN,&ssv);
    }
    else if ((strcmp(buff,"ilo") == 0)) {
      if (!oio)
	observe_obj(uo,osv,&oio);
      biff_illegal_obj(OBJN,OBJS,&osv);
    }
    else if ((strcmp(buff,"bls") == 0)) {
      biff_bl_self(SSN,&ssv);
    }
    else if ((strcmp(buff,"blo") == 0)) {
      if (!oio)
	observe_obj(uo,osv,&oio);
      biff_bl_obj(OBJN,OBJS,&osv);
    }
    else if ((strcmp(buff,"bhs") == 0)) {
      biff_bh_self(SSN,&ssv);
    }
    else if ((strcmp(buff,"bho") == 0)) {
      if (!oio)
	observe_obj(uo,osv,&oio);
      biff_bh_obj(OBJN,OBJS,&osv);
    }
    else {
      cout << "unhandled command string." << endl;
    }
  }

}
