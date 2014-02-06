#include "umbc/text_utils.h"
#include "umbc/token_machine.h"
#include <string>
#include <iostream>

using namespace std;
using namespace umbc;

int main(int argc, char ** argv) {
  cout << "*** token machine section ***" << endl;
  
  string tstr = "this is dull stuff (is that right?)";
  tokenMachine tm(tstr);
  tm.printWSource();
  while (tm.moreTokens()) {
    cout << "next='" << tm.nextToken() << "'" << endl;
  }  

  string s1 = "do something at time 5";
  tokenMachine tms1(s1);
  string s2 = "don't do anything cat time 30";
  tokenMachine tms2(s2);
  string tr = "at";
  if (tms1.containsToken(tr))
    cout << "'" << s1 << "' contains '" << tr << "'" << endl;
  else 
    cout << "'" << s1 << "' don't contain '" << tr << "'" << endl;

  if (tms2.containsToken(tr))
    cout << "'" << s2 << "' contains '" << tr << "'" << endl;
  else 
    cout << "'" << s2 << "' don't contain '" << tr << "'" << endl;

  string kwvt = ":x 2 :y (poop chute) :z terrible_towel";
  tokenMachine tmkw(kwvt);
  cout << "keyword test string = '" << kwvt << "'" << endl;
  cout << "x = " << tmkw.keywordValue(":x") << endl;
  cout << "z = " << tmkw.keywordValue(":z") << endl;
  cout << "y = " << tmkw.keywordValue(":y") << endl;

  string kwrt = "here is the test string for the test case";
  string rems = "the";
  tokenMachine tmkwr(kwrt);
  string kwr  = tmkwr.removeKWP(rems);
  cout << "kwr test = '" << kwrt << "'" << endl;
  cout << "removing   '" << rems << "'" << endl;
  cout << "result   = '" << kwr <<  "'" << endl;

  cout << "*** functor/param section ***" << endl;

  cout << "STRING : foo(bar,gah)" << endl;
  cout << "FUNCTOR: " << textFunctions::getFunctor("foo(bar,gah)") << endl;
  cout << "PARAMS : " << textFunctions::getParameters("foo(bar,gah)") << endl;

  string pstr = "(1,6,3,2,6,2,2)";
  int i=0;
  paramStringProcessor pm(pstr);
  pm.printWSource();
  while (pm.hasMoreParams()) {
    cout << "next='" << pm.getNextParam() << "' (" << i << ")='"
	 << pm.getParam(i) << "'" << endl;
    i++;    
  }
}
