#include "keyGen.h"

mclKey computeKey(string q) {
  int hk=0;
  for (int k=2; k < (int)q.length(); k++)
    hk = (hk<<4)^(hk>>28)^q[k];
  hk &= 0x0FFFFFFF;
  return hk;
}

