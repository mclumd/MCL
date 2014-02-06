#include <stdlib.h>
#include <math.h>
#include "simHost.h"

#define BASE_EXP_GRP 0x11110000

#define FH_NUM_SENSORS 4

string nv[FH_NUM_SENSORS] = 
  { "noisyCos","noisySin","noisyConst","randomWalk" };
float  sv[FH_NUM_SENSORS];

mclMA::observables::update mcl_update_obj;

void fakeHost::initMCL() {
  mclMA::initializeMCL(FH_MCL_KEY,8);
  mclMA::configureMCL(FH_MCL_KEY,"fake_host");
}

void fakeHost::updateSensors() {
  static int c=0;
  // noisy cos, breakout high
  sv[0]=cosf((float)c*3.14/(float)100) + (0.1 * (((float)rand()/(float)RAND_MAX)-0.5));
  // noisy sin, breakout low
  sv[1]=sinf((float)c*3.14/(float)100) + (0.1 * (((float)rand()/(float)RAND_MAX)-0.5));
  // noisy const, maintain val
  sv[2]=(((double)rand()/(double)RAND_MAX)*0.26-0.13);
  // random walk
  sv[3]+=((float)rand()/(float)RAND_MAX)-0.5;
  c++;
  for ( int i=0 ; i<FH_NUM_SENSORS ; i++ )
    mcl_update_obj.set_update(nv[i],sv[i]);
}

void fakeHost::registerSensors() {
  mclMA::observables::declare_observable_self(FH_MCL_KEY,nv[0]);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[0],
					PROP_SCLASS,SC_RESOURCE);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[0],
					PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_noiseprofile_self(FH_MCL_KEY,nv[0],
						    MCL_NP_UNIFORM,0.1);

  mclMA::observables::declare_observable_self(FH_MCL_KEY,nv[1]);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[1],
					PROP_SCLASS,SC_RESOURCE);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[1],
					PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_noiseprofile_self(FH_MCL_KEY,nv[1],
						    MCL_NP_UNIFORM,0.1);

  mclMA::observables::declare_observable_self(FH_MCL_KEY,nv[2]);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[2],
					PROP_SCLASS,SC_STATE);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[2],
					PROP_DT,DT_RATIONAL);
  mclMA::observables::set_obs_noiseprofile_self(FH_MCL_KEY,nv[2],
						MCL_NP_UNIFORM,0.1);

  mclMA::observables::declare_observable_self(FH_MCL_KEY,nv[3]);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[3],
					PROP_SCLASS,SC_SPATIAL);
  mclMA::observables::set_obs_prop_self(FH_MCL_KEY,nv[3],
					PROP_DT,DT_RATIONAL);

  for (int i=0; i<FH_NUM_SENSORS; i++) 
    mclMA::observables::dump_obs_self(FH_MCL_KEY,nv[i]);

  mclMA::dumpNPT();

}

void fakeHost::declareExps() {
  mclMA::declareExpectationGroup(FH_MCL_KEY,(egkType)BASE_EXP_GRP);

  mclMA::declareExpectation(FH_MCL_KEY,(egkType)BASE_EXP_GRP,
			    nv[0],EC_STAYUNDER,1.0);

  mclMA::declareExpectation(FH_MCL_KEY,(egkType)BASE_EXP_GRP,
			    nv[1],EC_STAYOVER,-1.0);

  mclMA::declareExpectation(FH_MCL_KEY,(egkType)BASE_EXP_GRP,
			    nv[2],EC_MAINTAINVALUE,0);

}

void fakeHost::registerHIAs() {
  mclMA::HIA::registerHIA(FH_MCL_KEY,"inconvenientTruthException",
			  "effectorError");
}

void fakeHost::setPV() {
  
  mclMA::setPropertyDefault(FH_MCL_KEY,PCI_INTENTIONAL,PC_YES);
  mclMA::setPropertyDefault(FH_MCL_KEY,PCI_PARAMETERIZED,PC_YES);
  mclMA::setPropertyDefault(FH_MCL_KEY,PCI_SENSORS_CAN_FAIL,PC_YES);
  mclMA::setPropertyDefault(FH_MCL_KEY,PCI_EFFECTORS_CAN_FAIL,PC_YES);

}
