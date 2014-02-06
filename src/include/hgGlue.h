#ifndef HG_GLUE
#define HG_GLUE

#include "common.h"
#include "mclEntity.h"
#include "abstractGlue.h"
#include "links.h"

#include <math.h>
#include <vector>

#define HG_GLUE_IDENTIFIER "hg_glue"

namespace metacog {

  class CPT;

class hgFrameGlue : public frameGlue {
 public:
  hgFrameGlue(string name, mclFrame* frame) : frameGlue(name,frame) {};
  virtual ~hgFrameGlue();

  virtual bool build();
  virtual string baseClassName() { return "hgFrameGlue"; };
  
 protected:

};

class hgNodeGlue : public nodeGlue {
 public:
  hgNodeGlue(string name,mclNode* node);
  virtual ~hgNodeGlue();

  //! most probable value
  virtual bool   mpv();
  virtual double p_true()  { return p_bool(true);  };
  virtual double p_false() { return p_bool(false); };
  virtual double p_bool(bool val);
  virtual void   mpd(double* v);
  virtual void   set_evidence(bool val);

  virtual void   writeCPTtoArray(double* a);
  virtual bool   initPriors(vector<string>& d,vector<double>& b);

  virtual string baseClassName() { return "hgNodeGlue"; };

 protected:
  hgFrameGlue*  myFrameGlue;
  hgFrameGlue*  getFrameGlue();
  void ensure_cpt();
  void setProbHard(double p);
  void unsetProbHard();
  // void initCPT(vector<double>& v);

  CPT *cp_table;
  double hardVal;
  bool   hardSet;

};

class CPT {
 public:
  
  /** constructor.
   * @param a linkList object containing pointers to incoming links.
   */
  CPT(linkList *in_links);

  virtual ~CPT();

  //! the number of cells in the CPT.
  int size() { return t_size; };

  //! tests for a mismatch in the number of incoming links and corresponding cells.
  bool mismatch() { return (pow(2,ins->size()) == t_size); };

  //! the probability, given the values of the incoming links' source nodes.
  double p();

  //! returns the discrete probability (1 or 0).
  double discrete_p();

  virtual void dumpEntity(ostream *o);

  //! initializes all table cells to 0.5.
  void initializeUniform();
  //! initializes the CPT using an array of doubles.
  void initializeVector(double v[]);
  //! initializes the CPT using a vector of doubles.
  void initializeVector(vector<double>& v);

  //! accesses the raw table value specified by 'index'.
  double cpt_val(int index) { return table[index]; };

 protected:
  //! the table values, stored in a dynamically-allocated double array
  double   *table;
  //! the computed size of the table, equal to 2^|inLinks|
  int       t_size;
  //! the list of incoming links.
  linkList *ins;

  //! not sure wtf this does.
  double p_priors();

};

};
#endif
