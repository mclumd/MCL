#include "fringe_linkage.h"
#include "mcl_observables.h"

using namespace metacog;

void legalspec::addLinkTags(linkTags_t& rv) {
  rv.push_back("illegalValue");
}

void itsallgood::addLinkTags(linkTags_t& rv) {
  legalspec::addLinkTags(rv);
  rv.push_back("unreachableMCLstate");
}

void legalset::addLinkTags(linkTags_t& rv) {
  legalspec::addLinkTags(rv);
  rv.push_back("notInSet");
}

void legalrange::addLinkTags(linkTags_t& rv) {
  legalspec::addLinkTags(rv);
  rv.push_back("outOfRange");
}


