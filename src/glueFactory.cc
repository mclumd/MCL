#include "glueFactory.h"
#include "../include/umbc/exceptions.h"
#include "mclFrame.h"
#include "mclLogging.h"

#include <list>

using namespace metacog;

///////////////// PRIVATE SECTION

string supportedGlues[] = { HG_GLUE_IDENTIFIER,
			    // PNL_GLUE_IDENTIFIER,
			    SMILE_GLUE_IDENTIFIER };

list<string> autoGlues;
string       defaultGlue="";

frameGlue* createFrameGlue(string key,mclFrame* f) {
  frameGlue* rv=NULL;
  if (key == HG_GLUE_IDENTIFIER)
    rv = new hgFrameGlue("hgFrameGlue",f);
  // else if (key == PNL_GLUE_IDENTIFIER)
  // rv = new pnlFrameGlue("pnlFrameGlue",f);
  else if (key == SMILE_GLUE_IDENTIFIER)
    rv = new smileFrameGlue("smileFrameGlue",f);
  else umbc::exceptions::signal_exception("no such glue supported by glueFactory: "+key);
  return rv;
}

/////////////////  PUBLIC SECTION

string glueFactory::getDefaultGlueKey() {
  return defaultGlue;
}

void glueFactory::setDefaultGlueKey(string key) {
  defaultGlue=key;
}

void glueFactory::addAsDefaultGlue(string key) {
  autoGlues.push_back(key);
  defaultGlue=key;
}

void glueFactory::addAutoGlue(string key) {
  autoGlues.push_back(key);
}

void glueFactory::clearAutoGlue() {
  autoGlues.clear();
}

void glueFactory::autoCreateGlue(mclFrame* f) {
  if (autoGlues.empty()) {
    autoGlues.push_back(HG_GLUE_IDENTIFIER);
    addAsDefaultGlue(SMILE_GLUE_IDENTIFIER);
    // this is significant, if the frame's default glue key is "", then
    // update that, too.
    if (f->defaultGlueKey() == "")
      f->setDefaultGlueKey(HG_GLUE_IDENTIFIER);
  }
  for (list<string>::iterator lsi = autoGlues.begin();
       lsi != autoGlues.end();
       lsi++) {
    frameGlue* rv = createFrameGlue(*lsi,f);
    f->addGlue(*lsi,rv);
    rv->build();
  }
  umbc::uLog::annotateStart(MCLA_MSG);
  *umbc::uLog::log << "[MCL:glueFactory]::glue config is ";
  printGlueConfig(umbc::uLog::log);
  umbc::uLog::annotateEnd(MCLA_MSG);
}

void glueFactory::printGlueConfig(ostream* s) {
  *s << "< ";
  for (list<string>::iterator lsi = autoGlues.begin();
       lsi != autoGlues.end();
       lsi++) {
    if (*lsi == defaultGlue)
      *s << "*";
    *s << *lsi << " ";
  }
  *s << ">" << endl;
}
