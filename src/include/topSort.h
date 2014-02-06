#ifndef TOPSORT_H
#define TOPSORT_H

#include<vector>
using namespace std;

namespace metacog {
  class mclNode;

  namespace topsort {
    bool topSort(vector<mclNode*>& invec);
  };
};

#endif
