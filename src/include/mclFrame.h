#ifndef MCL_FRAME_H
#define MCL_FRAME_H

#include "glueFactory.h"

/** \file
 * \brief An mclFrame organizes data corresponding to an expectation violation.
 */

#include "mclConstants.h"
#include "buildCore.h"
#include "mcl_api.h"

#include "ontology_config.h"
#include "expectations.h"
#include "oNodes.h"
#include "frameEntryVector.h"

#include "abstractGlue.h"

namespace metacog {

  class mcl;
  class ontologyRecord;

  /** the class of violation Frames in MCL.
   */
  class mclFrame : public mclEntity, public hasFrameGlue {
  public:
    
    /** constructor.
     * creates a new mclFrame for an expectation group.
     * @param m the MCL managing the frame.
     * @param veg the expectation group that produced the failure.
     * @param veg_parent the expectation group of the parent (NULL=no parent).
     * @note this is the preferred usage of the mclFrame constructor.
     * OBSOLETE !!
    mclFrame(mcl *m,egkType veg,egkType veg_parent);
     */ 

    /** constructor.
     * creates a new mclFrame for an expectation group.
     * @param m the MCL managing the frame.
     * @param fev the Frame Entry Vector object that describes the entry state
     * @note this is the preferred usage of the mclFrame constructor.
     */
    mclFrame(mcl* m,mclFrameEntryVector& fev);

    virtual string baseClassName() { return "mclFrame"; };

    //! adds a new Frame Entry Vector to the FEV List --
    //! there is a switch that determines whether or not they
    //! are managed as a set or a trace (list)
    void    add_new_fev(mclFrameEntryVector& fev);

    /** returns the requested ontology.
     *  use INIDICATION_INDEX, FAILURE_INDEX, and RESPONSE_INDEX
     */
    mclOntology* getCoreOntology(int index) { return (*core)[index]; };
    //! returns the number of ontologies associated with the frame (3).
    int          numOntologies() { return core->size(); };
    mclOntology* getIndicationCore() 
      { return getCoreOntology(INDICATION_INDEX);};
    mclOntology* getFailureCore() 
      { return getCoreOntology(FAILURE_INDEX);};
    mclOntology* getResponseCore() 
      { return getCoreOntology(RESPONSE_INDEX);};
    void setCore(ontologyVector* ovp);
    
    int nodeCount() {
      int k=0;
      for (int q=0;q<numOntologies();q++) 
	k+=getCoreOntology(q)->size();
      return k;
    }
    
    mclNode **allNodes(); // i am a little worried that this function is insane.
    vector<mclNode*> allNodesV();
    
    //! returns the in-house ontology node named by 'nodename'.
    mclNode* findNamedNode(string nodename);
    
    virtual mcl *MCL() { return myMCL; };
    
    mclPropertyVector* getPV() { return &myPV; };
    
    //! sets the state of the mclFrame's processing (see #framestates).
    void setState(int nustate) { state=nustate; };
    //! gets the state of the mclFrame's processing (see #framestates).
    int  getState() { return state; };
    //! returns true if there is outstanding advice reflected in the state
    bool in_advice_state() {
      return ((state == FRAME_RESPONSE_ISSUED) ||
	      (state == FRAME_RESPONSE_TAKEN) ||
	      (state == FRAME_RESPONSE_ACTIVE));
    }
    
    //! updates the pointer to the node corresponsing to MCL's recommended response.
    void set_active_response(mclConcreteResponse* rnode) 
      { active_response=rnode; if (rnode != NULL) rnode->inc_totalAttempts(); }
    //! returns a pointer to the node corresponsing to MCL's recommended response.
    mclNode* get_active_response() { return active_response; };
    
    //! processes the response ontology to produce a response.
    virtual mclMonitorResponse *generateResponse();
    
    //! process feedback to an interactive response
    void processFeedback(bool k);
    
    //! Violation Expectation Group Key
    egkType get_vegKey() { return veg_key; };
    //! Violation Expectation Group's Parent Key
    egkType get_veg_parentKey() { return veg_parent_key; };
    
    void note_success() { successes++; };
    void note_refail() { refails++; };
    
    //! signals that the response has terminated on the host successfully.
    void frameComplete() { setState(FRAME_RESPONSE_SUCCEEDED); };
    
    void dumpEntity(ostream *out);
    void dumpLog(ostream& out);
    
    //! 
    void assess();
    //!
    mclMonitorResponse* guide();

    /*
    //! re-enters an existing frame for further processing.
    //! assumes 'exp' has been violated unless it is NULL.
    //! this function is supposed to properly manage the frameState and
    //! bayes evidence based on the semantics of the current frameState
    //! and re-entry code (or other FEV parameters)
    //! 
    //! @param exp the newly violated expectation
    //! @param fev the Frame Entry Vector into the frame
    //! @param hostInitiated true if the host is responsible for initiating 
    //!   re-entry into the frame (by ignore or direct failure specification)
    // void reEnterFrame(mclExp* exp,mclFrameEntryVector& fev,bool hostInitiated);
    */

    void                reentry_ignoring();
    void                reentry_implementing();
    void                reentry_success();
    mclMonitorResponse* reentry_declining();
    mclMonitorResponse* reentry_aborting();
    mclMonitorResponse* reentry_failing();
    void reentry_recurrence(mclExp* exp,mclFrameEntryVector& fev);

    //  probabilistic reasoning helper monkeys
    //! P that the node is true
    double p_true(string glue,string  nn) 
      { return findNamedNode(nn)->p_true(glue); };
    double p_true(string glue,mclNode *n) { return n->p_true(glue); };
    double p_true(string  nn) { return findNamedNode(nn)->p_true(); };
    double p_true(mclNode *n) { return n->p_true(); };
    //! P that the node is false
    double p_false(string glue,string  nn) 
      { return findNamedNode(nn)->p_false(glue); };
    double p_false(string glue,mclNode *n) { return n->p_false(glue); };
    double p_false(string  nn) { return findNamedNode(nn)->p_false(); };
    double p_false(mclNode *n) { return n->p_false(); };
    //! P of supplied value
    double p_bool(string glue,string  nn,bool b) 
      { return findNamedNode(nn)->p_bool(glue,b); };
    double p_bool(string glue,mclNode *n,bool b) 
      { return n->p_bool(glue,b); };
    double p_bool(string  nn,bool b) { return findNamedNode(nn)->p_bool(b); };
    double p_bool(mclNode *n,bool b) { return n->p_bool(b); };
    //! MPD for a node
    void   mpd(string glue,string nn,double* d)  
      { return findNamedNode(nn)->mpd(glue,d); };
    void   mpd(string glue,mclNode *n,double* d) 
      { return n->mpd(glue,d); };
    void   mpd(string nn,double* d)  { return findNamedNode(nn)->mpd(d); };
    void   mpd(mclNode *n,double* d) { return n->mpd(d); };
    //! setting evidence
    void   set_evidence(string nn,bool b) 
      { return findNamedNode(nn)->set_evidence(b); };
    void   set_evidence(mclNode* n,bool b) 
      { dirty=true; return n->set_evidence(b); };

    void   record_ostate_if_dirty();
    
#ifndef NO_DEBUG
    void dbg_forceFrameState(string node,int state);
#endif
    
    string describe();

    void   setDefaultGlueKey(string key) { default_glue_key = key; };
    string defaultGlueKey() { return default_glue_key; };
    // void  setGlobalDefaultGlueKey(string key) { GLOBAL_DEFAULT_GLUE = key; };
    // string defaultGlobalGlueKey() { return GLOBAL_DEFAULT_GLUE; };   
    // static string GLOBAL_DEFAULT_GLUE;

    // public member variable -- id -- should match referents from host
    resRefType frame_id;

    // these are frame predicates that can be used to test for reentrancy
    bool matchesReferent(resRefType ref) { return (frame_id == ref); };
    bool isMostRecentFrame();
    int  lastUpdate() { return last_update; };
    bool evSignatureExists(string inputEVS);
    bool isSignatureExists(string inputIIS);

    void log(string& le);
    void log(const char* le);

  private:
    
    //! a vector of ontologies (indications, failures, responses)
    ontologyVector*      core;
    //! pointer to the parent MCL object
    mcl*                 myMCL;
    //! state encoding to maintain information about the repair in progress
    int                  state;
    //! the response node recommended, if any
    mclConcreteResponse* active_response;
    //! ViolationExpectationGroup key and
    //! ViolationExpectationGroup parent key.
    egkType              veg_key,veg_parent_key;
    //! bookkeeping variables for counting refails and successful repairs
    int                  refails,successes;
    int                  last_update;
    bool                 dirty;
    //! default key for underlying inference implementation
    string               default_glue_key;
    mclPropertyVector    myPV;
    //! list of expectation signatures for violated expectations
    list<mclFrameEntryVector*> fevList;
    //! time series record of ontology states...
    vector<ontologyRecord*> ontology_records;
    //! time series record of frame interactions...
    vector<string> frameLog;

    void initFrame();
    void touch();

    static const resRefType FRAME_ID_NO_REFERENCE = 0;
    static resRefType FRAME_ID_COUNTER;
    
  };

};

#endif 
