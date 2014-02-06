#include "umbc/instrumentation.h"
#include "umbc/logger.h"
#include "umbc/declarations.h"
#include <math.h>

using namespace std;
using namespace umbc;

int main(int argc, char** argv) {
  uLog::setAnnotateMode(UMBCLOG_XTERM);
  if (argc < 2) 
    uLog::annotate(UMBCLOG_ERROR,"usage: " + (string)argv[0] + " <outfile>");
  else {
    int counter=0;
    double c,s;
    instrumentation::dataset *d = 
      instrumentation::make_dataset((string)argv[1]);
    instrumentation::int_ptr_instrument i1("counter",&counter);
    instrumentation::dbl_ptr_instrument i2("cosine",&c);
    instrumentation::dbl_ptr_instrument i3("sine",&s);
    instrumentation::decl_based_instrument i4("sine>","over");
    instrumentation::settable_bool_instrument i5("over",false);
    cout << "range test: " << i4.range() << endl;
    // note that here the instruments will be CLONED and added to the dataset
    // it's all okay here since they use pointers or declarations for their
    // value derivation. if they had any non-pointer internal state, that would
    // be CLONED and you'd have no real good way to change them.
    d->add_instrument(i1);
    d->add_instrument(i2);
    d->add_instrument(i3);
    d->add_instrument(i4);
    // note that it is NOT OKAY if i5 goes out of scope and you use the
    // pointer version of add_instrument. here, we're not going to go out
    // of scope, but we want to be able to set an instance variable within
    // i5, so that's why we do this one this way. it will NOT be cloned.
    d->add_instrument(&i5);
    for (int i=0;i<100;i++) {
      counter++;
      c=cos((double)counter/25);
      s=sin((double)counter/25);
      if ((c > 0.9) || (s > 0.9)) {
	i5.setval(true);
	declarations::declare("over");
      }
      else i5.setval(false);
      d->writeline();
    }
  }
  return 0;
}
