#ifndef SMILE_GLUE
#define SMILE_GLUE

#include "smile.h"
#include "mclEntity.h"
#include "abstractGlue.h"

using namespace std;

#define SMILE_GLUE_IDENTIFIER "smile_glue"

// these are the indexes into the JPD for true and false
#define SMILE_TRUE_VALUE  "true"
#define SMILE_FALSE_VALUE "false"

namespace metacog {

  class mclNode;

  namespace smileGlue {
    string smilizedName(string name);
    int    smileIndex2configIndex(int smileI,
				  vector<string>& depsAsCreated,
				  vector<string>& hgD);
  };
  
  class mclFrame;

  /* The classes contained in this file are considered "glue" components
     because they are abstract classes that contain the glue necessary
     to provide a SMILE implementation of the probabilistic reasoning 
     functionality promised by MCL.
     
  */
  
  class smileFrameGlue : public frameGlue {
  public:
    smileFrameGlue(string name,mclFrame* frame) :
      frameGlue(name,frame) {};
    virtual ~smileFrameGlue();
    
    virtual bool build();
    virtual string baseClassName() { return "smileFrameGlue"; };
    
    DSL_network* getNet() { 
      // cout << "[SMILEGh]: frameGlue @ " << this
      // << ", theNet is @ " << &(this->theNet) << endl;
      return &theNet; 
    };
    
    void dumpToFile(string fn);
    
  protected:
    DSL_network theNet;
    
  };
  
  class smileNodeGlue : public nodeGlue {
  public:
    smileNodeGlue(string name,mclNode* node);
    virtual ~smileNodeGlue();
    
    virtual bool   mpv();
    virtual double p_true()  { return p_bool(true);  };
    virtual double p_false() { return p_bool(false); };
    virtual double p_bool(bool val);
    virtual void   mpd(double* v);
    virtual void   set_evidence(bool val);
    
    virtual void   writeCPTtoArray(double* a);
    virtual bool   initPriors(vector<string>& d,vector<double>& b);
    
    virtual string baseClassName() { return "smileNodeGlue"; };
    
    int getSMILEid() { return smileID; };
    
  protected:
    smileFrameGlue*  myFrameGlue;
    smileFrameGlue* getFrameGlue();
    int smileID;
    vector<string>  depsAsCreated;
    
  };
  
};
#endif
