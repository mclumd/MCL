#include <list>
#include <vector>
#include <string>

using namespace std;

namespace metacog {

  class mclExpGroup;
  class mclFrame;
  class mclHIADef;
  class mclSensorDef;
  class mcl;

  typedef list<string> linkTags_t;
  typedef vector<mclExpGroup *>    egMap;
  typedef vector<mclFrame *>       frameVec;
  typedef vector<mclSensorDef *>   sdVec;
  typedef vector<mclHIADef *>      HIAVec;

};
