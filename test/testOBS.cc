#include "mcl_api.h"

int main(int argc,char ** argv) {
  mclMA::initializeMCL("testOBS",0);

  mclMA::observables::declare_observable_self("testOBS","odometer");
  mclMA::observables::set_obs_prop_self("testOBS","odometer",
					PROP_DT,DT_INTEGER);
  mclMA::observables::set_obs_prop_self("testOBS","odometer",
					PROP_SCLASS,SC_COUNTER);
  mclMA::observables::set_obs_legalrange_self("testOBS","odometer",0,100000);
  cout << " >> test: dump ov with self property." << endl;
  mclMA::observables::dump_globo("testOBS");
  cout << " >> test: dump obs / self property." << endl;
  mclMA::observables::dump_obs_self("testOBS","odometer");

  mclMA::observables::declare_observable_object_type("testOBS","ufo");
  mclMA::observables::declare_object_type_field("testOBS","ufo","size");
  mclMA::observables::set_obs_prop("testOBS","ufo","size",
				   PROP_DT,DT_INTEGER);
  mclMA::observables::set_obs_prop("testOBS","ufo","size",
				   PROP_SCLASS,SC_SPATIAL);
  mclMA::observables::add_obs_legalval("testOBS","ufo","size",1);
  mclMA::observables::add_obs_legalval("testOBS","ufo","size",2);
  mclMA::observables::add_obs_legalval("testOBS","ufo","size",3);
  mclMA::observables::declare_object_type_field("testOBS","ufo","distance");
  mclMA::observables::set_obs_prop("testOBS","ufo","distance",
				   PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_prop("testOBS","ufo","distance",
				   PROP_SCLASS,SC_OBJECTPROP);
  mclMA::observables::set_obs_legalrange("testOBS","ufo","distance",10,1000);
  mclMA::observables::notice_object_observable("testOBS","ufo","ufoX");
  cout << " >> test: dump obs / object properties." << endl;
  mclMA::observables::dump_obs("testOBS","ufoX","distance");
  mclMA::observables::dump_obs("testOBS","ufoX","size");
  mclMA::observables::dump_globo("testOBS");

  cout << " >> test: setting obj/obs properties with live objects." << endl;
  cout << " >>>> before" << endl;
  mclMA::observables::dump_obs("testOBS","ufoX","size");
  mclMA::observables::set_obs_prop("testOBS","ufo","size",
				   PROP_SCLASS,SC_OBJECTPROP);
  mclMA::observables::add_obs_legalval("testOBS","ufo","size",0.0);
  cout << " >>>> after" << endl;
  mclMA::observables::dump_obs("testOBS","ufoX","size");

  cout << " >> test: updating obj/obs value." << endl;
  mclMA::observables::update_observable("testOBS","ufoX","distance",100.0);
  mclMA::observables::dump_globo("testOBS");

  cout << " >> test: de-noticing object." << endl;
  mclMA::observables::notice_object_unobservable("testOBS","ufoX");
  mclMA::observables::dump_globo("testOBS");

  cout << " >> test: re-noticing object from memory." << endl;
  mclMA::observables::notice_object_observable("testOBS","ufo","ufoX");
  mclMA::observables::dump_globo("testOBS");
  
  cout << " >> test completed." << endl;
}
