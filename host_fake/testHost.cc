// this is a test file for the fake host

#include <iostream>
#include <unistd.h>
#include "simHost.h"

#define EVERY_CHECK 100

using namespace std;

// extern float sv[3];
extern mclMA::observables::update mcl_update_obj;

int main(int argc, char **argv) {
  fakeHost::initMCL();
  
  fakeHost::registerSensors();
  fakeHost::registerHIAs();
  fakeHost::declareExps();

  int  maxI=20;
  bool bcoe=true,bc=false;
  
  if (argc > 1) 
    maxI=atoi(argv[1]);

  fakeHost::setPV();

  for (int i=0; ((i<maxI) && (!bcoe || !bc)); i++) {
    fakeHost::updateSensors();
    if (((float)rand() / (float)RAND_MAX) < 0.05)
      mclMA::HIA::signalHIA(FH_MCL_KEY,"inconvenientTruthException");
    if (((i+1) % EVERY_CHECK) == 0)
      cout << endl << (1+i) << ": ";
    responseVector m = mclMA::monitor(FH_MCL_KEY,mcl_update_obj);
    cout << "FH: " << m.size() << " responses in vector." << endl;
    if (m.size() > 0) {
      bc=true;
      int q=1;
      for (responseVector::iterator rvi = m.begin();
	   rvi!=m.end();
	   rvi++) {
	cout << "response[ref" << hex << (*rvi)->referenceCode() 
	     << "] #" << q++ << ": " << (*rvi)->responseText() << endl;
      }
    }
    usleep(100000);
  }

  cout << endl;
  return 0;
}
