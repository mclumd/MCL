#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "text_utils.h"

using namespace std;
using namespace umbc;

void tnv(string tnv) {
  int o = textFunctions::numval(tnv);
  if (errno != 0) perror(" numval");
  else cout << "numval("+tnv+")=" << o << endl;

  long l = textFunctions::longval(tnv);
  if (errno != 0) perror(" longval");
  else cout << "longval("+tnv+")=" << l << endl;

  unsigned int ui = textFunctions::unumval(tnv);
  if (errno != 0) perror(" unumval");
  else cout << "unumval("+tnv+")=" << ui << endl;

  double d = textFunctions::dubval(tnv);
  if (errno != 0) perror(" dubval");
  else cout << "dubval("+tnv+")=" << d << endl;

}

int main(int argc, char** argv) {
  char buff[128];
  while (1) {
    cout << "testNumVal> ";
    cin.getline(buff,127);
    if (strcmp(buff,"exit") == 0)
       break;
    else tnv(buff);
  }
  return 1;
}
