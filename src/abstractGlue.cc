#include "mclNode.h"
#include "abstractGlue.h"

using namespace metacog;

nodeGlue* nodeGlue::getSiblingNodeGlue(string key) {
  return myNode()->getGlue(key);
}
