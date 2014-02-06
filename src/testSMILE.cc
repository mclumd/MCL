#include "mcl.h"
#include "mclFrame.h"
#include "glueFactory.h"

#include <iostream>
using namespace std;

void test_set(mclFrame& f,string name);
void test_p(mclFrame& f,string name);
void test_index_conversion();

int main(int argc, char **argv) {
  glueFactory::addAutoGlue(HG_GLUE_IDENTIFIER);
  glueFactory::addAsDefaultGlue(SMILE_GLUE_IDENTIFIER);
  cfg::loadConfig("marsSim");

  mcl         m;
  mclFrame    f(&m);

  smileFrameGlue* fg = (smileFrameGlue*)(f.getGlue(SMILE_GLUE_IDENTIFIER));
  fg->dumpToFile("base_mcl.xdsl");

  cout << "[TESTSMILE]: testing set_evidence..." << endl;

  test_p(f,"sensorMalfunction");
  test_set(f,"sensorsCanFail");
  test_p(f,"sensorMalfunction");
  test_set(f,"sensorStuck");
  test_p(f,"sensorMalfunction");

  test_p(f,"amendPredictiveModels");
  test_set(f,"amendKnowledgeBase");
  test_p(f,"amendPredictiveModels");
  test_set(f,"predictiveModelError");
  test_p(f,"amendPredictiveModels");

  // test_index_conversion();
  
}

void test_set(mclFrame& f,string name) {
  cout << "[TESTSMILE]: initially, " << name << " p=" << f.p_true(name)
       << endl;
  f.set_evidence(name,true);
  cout << "[TESTSMILE]: " << name << " evidence set to true, p=" << f.p_true(name)
       << endl;
}

void test_p(mclFrame& f,string name) {
  cout << "[TESTSMILE]: " << name << ", <smile>p=" << f.p_true(name) << endl
       << "           > " << name << ", <  hg >p=" << f.p_true(HG_GLUE_IDENTIFIER,name) 
       << endl;
}

void test_index_conversion() {
  vector<string> smv,hgv;
  smv.push_back("a");
  smv.push_back("b");
  smv.push_back("c");
  hgv.push_back("b");
  hgv.push_back("c");
  hgv.push_back("a");
  for (int i=0; i< (int)pow(2,smv.size()); i++) {
    cout << i << " -> " << smileGlue::smileIndex2configIndex(i,smv,hgv) << endl;
  }
}
