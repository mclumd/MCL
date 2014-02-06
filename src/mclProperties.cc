#include "mclProperties.h"

using namespace metacog;

string mclPropertyVector::pnames[PC_VECTOR_LENGTH] = 
  {"intentional","effectorsCanFail","sensorsCanFail",
   "parameterized","declarative",
   "retrainable","hlcController",
   "htn_in_play","plan_in_play","action_in_play",

   "can_ignore", "can_noop","can_try_again",
   "can_solicit_help", "can_relinquish_control",
   "can_sensor_diag", "can_effector_diag",
   "can_sensor_reset", "can_effector_reset",
   "can_activate_learning", "can_adj_params", "can_rebuildmodels",
   "can_revisit_assumptions", "can_amend_controller",
   "can_revise_expectations", "can_swap_algorithms", "can_change_hlc"

  };

pvType mclPropertyVector::defaults[PC_VECTOR_LENGTH] = 
  { false,false,false,false,false,false,false,false,false,false,
    true,true,false,false,false,false,false,false,false,false,false,false,false,false,false
  };

void mclPropertyVector::copyValues(mclPropertyVector& dest) {
  for (int i=0;i<PC_VECTOR_LENGTH;i++) {
    dest.setProperty(i,getPropertyValue(i));
  }
}

void mclPropertyVector::unSetProperty(int index) { 
  values[index]=defaults[index]; 
}

void mclPropertyVector::reSetProperties() {
  for (int i=0;i<PC_VECTOR_LENGTH;i++)
    values[i]=defaults[i];
}

void mclPropertyVector::setDefaultProperty(int index,pvType nv) {
  defaults[index]=nv;
}

pvType mclPropertyVector::getDefaultProperty(int index) {
  return defaults[index];
}

void mclPropertyVector::dumpEntity(ostream *s) {
  *s << "PV(";
  for (int i=0;i<PC_VECTOR_LENGTH;i++) {
    *s << pnames[i] << "=" << values[i];
    if (i < PC_VECTOR_LENGTH - 1)
      *s << ",";
  }
  *s << ")";
}

///////////////////////////////////////////////////////////////////
/// property stacks
///////////////////////////////////////////////////////////////////

mclPropertyVectorStack::mclPropertyVectorStack() :
  base(new mclPropertyVector()),myMCL(NULL) {};

mclPropertyVectorStack::mclPropertyVectorStack(mcl *m) :
  base(new mclPropertyVector(m)),myMCL(m) {};

void mclPropertyVectorStack::newPropertyVector() {
  mclPropertyVector *nuV = new mclPropertyVector(*currentPV());
  stack.push_front(nuV);
}

void mclPropertyVectorStack::popPropertyVector() {
  if (!stack.empty()) stack.pop_front();
}
