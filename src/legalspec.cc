#include "mcl_observables.h"

using namespace metacog;

// using namespace umbc;

//////////////// LEGALSPEC STUFF

bool legalrange::is_legal(double v) { 
  cout << "LRIL: " << low << " [" << v << "] " << high << endl;
  return ((v >= low) && (v <= high)); 
}

string legalset::to_string() {
  string q="[";
  for (list<double>::iterator li = legals.begin();
       li != legals.end();
       li++) {
    char k[32];
    sprintf(k," %.3lf",(*li));
    q+=k;
  }
  return q+"]";
}

string legalrange::to_string() {
  char k[64];
  sprintf(k,"[%.3lf..%.3lf]",low,high);
  return k;
}
