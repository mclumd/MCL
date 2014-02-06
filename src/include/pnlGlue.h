#ifndef PNL_GLUE
#define PNL_GLUE

#include "pnl_dll.hpp"
#include "mclEntity.h"
#include "abstractGlue.h"

#define PNL_GLUE_IDENTIFIER "pnl_glue"

// these are the indexes into the JPD for true and false
#define PNL_TRUE_VALUE  0
#define PNL_FALSE_VALUE 1

class mclFrame;

/* The classes contained in this file are considered "glue" components
   because they are abstract classes that contain the glue necessary
   to provide a PNL implementation of the probabilistic reasoning 
   functionality promised by MCL.

   It contains a frameComponent called pnlGraphicalModel which contains
   superstructure for the PNL implementation of the MCL ontologies as a
   graphical model.

*/

PNL_USING

class pnlFrameGlue : public frameGlue {
 public:
  pnlFrameGlue(string name,mclFrame* frame) :
    frameGlue(name,frame),
    model(NULL),bayesNet(NULL),inferenceEngine(NULL) {};
  virtual ~pnlFrameGlue();
  
  virtual bool build();
  virtual string baseClassName() { return "pnlFrameGlue"; };

  // non-standard but public glue functions
  void mpd(double* pd,int nodeID);
  void set_evidence(bool val, int nodeID);
  
 protected:
  CGraph* model;
  CBNet * bayesNet;
  CNaiveInfEngine* inferenceEngine;

};

class pnlNodeGlue : public nodeGlue {
 public:
  pnlNodeGlue(string name,mclNode* node);
  pnlNodeGlue(string name,mclNode* node,int id);
  virtual ~pnlNodeGlue();
  
  virtual bool   mpv();
  virtual double p_true()  { return p_bool(true);  };
  virtual double p_false() { return p_bool(false); };
  virtual double p_bool(bool val);
  virtual void   mpd(double* v);
  virtual void   set_evidence(bool val);

  virtual void   writeCPTtoArray(double* a);
  virtual bool   initPriors(vector<string>& d,vector<double>& b);

  virtual string baseClassName() { return "pnlNodeGlue"; };

  // only in pnl glue
  int     getPNLid() { return PNLid; };

 protected:
  static  int    autocounter;
  pnlFrameGlue*  myFrameGlue;
  int            PNLid;
  
  pnlFrameGlue* getFrameGlue();
  
};

#endif
