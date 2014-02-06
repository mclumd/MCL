#include "reentrancy.h"
#include "mclLogging.h"
#include "Exceptions.h"
#include "../include/umbc/settings.h"
#include "DWtestREB.h"

using namespace metacog;
using namespace umbc;

#define DEFAULT_REB_IDENTIFIER "passive"

ReEntrantBehavior* REB::makeREB(string identifier) {
  if ((identifier == "default") || (identifier.size() == 0))
    identifier = DEFAULT_REB_IDENTIFIER;
  if (identifier == "passive") {
    return new PassiveREB();
  } else if (identifier == "zero") {
    return new DWtestREB(0);
  } else if (identifier == "one") {
    return new DWtestREB(1);
  } else if (identifier == "two") {
    return new DWtestREB(2);
  } else if (identifier == "three") {
    return new DWtestREB(3);
  } else if (identifier == "four") {
    return new DWtestREB(4);
  } else if (identifier == "five") {
    return new DWtestREB(5);
  } else if (identifier == "six") {
    return new DWtestREB(6);
  } else if (identifier == "seven") {
    return new DWtestREB(7);
  } else if (identifier == "eight") {
    return new DWtestREB(8);
  } else if (identifier == "nine") {
    return new DWtestREB(9);
  } else {
    uLog::annotate(MCLA_ERROR,
                   "REB identifier '"+identifier+"' "+
                   "is not known, defaulting to "+
                   "'"+DEFAULT_REB_IDENTIFIER+"'");
    throw BadKey("Unknown REB identifier '"+identifier+"'");
    // this is the old behavior -- changed MS (7/14/2011)
    // return makeREB(DEFAULT_REB_IDENTIFIER);
  }
}

ReEntrantBehavior* REB::makeREB() {
  return makeREB(umbc::settings::getSysPropertyString("mcl.rebPolicy",
                                                      DEFAULT_REB_IDENTIFIER));
}

