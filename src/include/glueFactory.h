#ifndef GLUE_FACTORY
#define GLUE_FACTORY

#include "abstractGlue.h"

#include "hgGlue.h"
#include "smileGlue.h"
// #include "pnlGlue.h"

namespace metacog {

  class mclFrame;

  namespace glueFactory {
    void    clearAutoGlue();
    void    addAutoGlue(string key);
    void    addAsDefaultGlue(string key);
    void    setDefaultGlueKey(string key);
    string  getDefaultGlueKey();
    void    autoCreateGlue(mclFrame* f);
    void    printGlueConfig(ostream* s);
    
  };
};

#endif
