#ifndef ABSTRACT_GLUE_MCL
#define ABSTRACT_GLUE_MCL

#include "mclEntity.h"

#include <map>

namespace metacog {

class frameGlue : public frameComponent {
 public:
  frameGlue(string name, mclFrame* frame) :
    frameComponent(name,frame) {};
  ~frameGlue() {};
  virtual bool build()=0;

};

class nodeGlue : public nodeComponent {
 public:
  nodeGlue(string name,mclNode* node) : nodeComponent(name,node) {};
  ~nodeGlue() {};

  virtual bool   mpv()=0;
  virtual double p_true()=0;
  virtual double p_false()=0;
  virtual double p_bool(bool val)=0;
  virtual void   mpd(double* v)=0;
  virtual void   assert_true()  { set_evidence(true); };
  virtual void   assert_false() { set_evidence(false); };
  virtual void   set_evidence(bool val)=0;
  virtual void   writeCPTtoArray(double* a)=0;
  virtual bool   initPriors(vector<string>& d,vector<double>& b)=0;
  
  virtual nodeGlue* getSiblingNodeGlue(string key);
};

class hasFrameGlue {
 public:
  hasFrameGlue() {};
  frameGlue* getGlue(string key) { return frameGlueMapper[key]; };
  void       addGlue(string key, frameGlue* fg) { frameGlueMapper[key]=fg; };
  
 protected:
  map<string,frameGlue*> frameGlueMapper;
  
};

class hasNodeGlue {
 public:
  hasNodeGlue() {};
  nodeGlue* getGlue(string key) { return nodeGlueMapper[key]; };
  void      addGlue(string key, nodeGlue* fg) { nodeGlueMapper[key]=fg; };

 protected:
  map<string,nodeGlue*> nodeGlueMapper;
  
};

};

#endif
