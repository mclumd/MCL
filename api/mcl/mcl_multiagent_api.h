/*! \file mcl_multiagent_api.h
  \brief functional API to MCL for multiagent systems 
  (with multiple instantiations of MCL)
*/

#ifndef MCL_MA_API_H
#define MCL_MA_API_H

#include <string>
#include <map>

#include "mclMonitorResponse.h"
#include "APITypes.h"

using namespace std;

/*! \namespace mclMA
  \brief public functions available to multiagent MCL host systems.
*/

namespace mclMA {

  // INIT STUFF

  /** initializes a Meta Cognitive Loop for a host agent/system.
   * required for a host to interoperate with MCL.
   * \param key a *unique* host indicator
   * \param Hz the number of times per second monitor should execute (Hz = 0 for synchronous/maximum monitor rate)
   */
  bool initializeMCL(string key,int Hz);

  /** destroys a Meta Cognitive Loop, releasing the key and associated 
   * resources. required for recurrent usages like the MCL server.
   * \param key a previously defined MCL key.
   */
  bool releaseMCL(string key);

  /** Stop and stop monitor() operation.
   * \param key a previously defined MCL key.
   */
  bool startMCL(string key);
  bool stopMCL(string key);

  /* bool chooseOntology(string key, string ont); */

  bool legalOntologySpec(string ospec);

  /** configures the Meta Cognitive Loop.
   * instructs MCL how to configure frames when they are issued by specifying
   * the domain, agent, and controller the MCL is being applied to.
   * \param key the *unique* host indicator
   * \param domain the name of the domain being operated on
   * \param agent the name of the agent operating in *domain*
   * \param controller the name of the controller being used
   */
  bool configureMCL(string key, string ontology, string domain);
  bool configureMCL(string key, string ontology, string domain, string agent);
  bool configureMCL(string key, string ontology, string domain, string agent,
		    string controller);

  // SENSOR VECTOR STUFF

  /** initializes the MCL sensor vector.
   * recommended before actions are activated/deactivated by host.
   * \param sv a pointer to an array of floating point sensor values
   * \param svs the length of the array
   */      
  bool initializeSV(string key,float *sv,int svs);

  /** updates the MCL sensor vector.
   * equivalent to #initializeSV().
   */      
  bool updateSV(string key,float *sv,int svs);

  // PROPERTY VECTOR STUFF

  /** creates a new Property Vector Stack Frame.
   * can be used when a new control block is entered if host properties
   * change in the new scope.
   * \sa mclProperties.h
   */
  void newDefaultPV(string key);

  /** pops a Property Vector off the top of the PV Stack Frame.
   * can be used when a control block is completed.
   * \sa mclProperties.h
   */
  void popDefaultPV(string key);

  /** set a property value on the top vector of the PV Stack.
   * Used to change the currently active properties when a new control block
   * is entered.
   * \param index reference to the PCI code (see #pci_enum)
   * \param v the property value
   * \sa mclProperties.h
   */
  void setPropertyDefault(string key,int index,pvType v);


  /** set a property value on the top vector of the PV Stack.
   * Used to change the currently active properties when a new control block
   * is entered.
   * \param key is the reference to the MCL 
   * \param eg_key is the exp grp key to find the correct PV
   * \param index reference to the PCI code (see #pci_enum)
   * \param v the property value
   * \sa mclProperties.h
   */
  void setEGProperty(string key,egkType eg_key,pkType index,pvType value);

  /** set a property value on the top vector of the PV Stack.
   * sets the current PV (top of the PV stack) values to their defaults.
   * \sa mclProperties.h
   */
  void reSetDefaultPV(string key);
  
  // HIA STUFF
  
  namespace HIA {
    /** registers a Host Initiated Indication with MCL.
     * \param name the name of the HIA
     * \param failureNodeName the name of the MCL ontology node to be activated when the HIA is signaled
     * \sa mclHIADef.h
     */
    bool registerHIA(string key,string name,string failureNodeName);
    
    /** lets the host signal an HIA to MCL.
     * signals a host-initiated indication to MCL.
     * \sa mclHIADef.h
     */
    bool signalHIA(string key,string name);
    bool signalHIA(string key,string name, resRefType referent);
    
  };

  // INITIALIZE SENSOR STUFF

  /** registers a host sensor with MCL
   * \note updated to utilize ::observables 1/29/09
   * \param name the name of the sensor
   * \sa mclSensorDef.h
   */
  bool registerSensor(string key,string name);
  /** sets a sensor property with MCL
   * \note updated to utilize ::observables 1/29/09
   * \param name the name of the sensor
   * \param key the sensor's property key that we are going to set
   * \param pv the sensor's new property value
   * \sa mclSensorDef.h, APICodes.h
   */
  bool setSensorProp(string mclkey,string name,spkType key,spvType pv);

  /** sets up a noise profile for the named sensor
   * \param name the name of the sensor
   * \param npKey the key for the desired noise profile
   * \param param passed to createNoiseProfile (dependent on npKey)
   * \sa mclSensorDef.h, APICodes.h
   */
  bool setSensorNoiseProfile(string mclkey,string name,spvType npKey);
  bool setSensorNoiseProfile(string mclkey,string name,spvType npKey,
			     double param);

  // SENSOR NOISE MAINTENANCE
  //

  // EXPECTATION STUFF

  /** declares an expectation group with MCL.
   * an expectation group corresponds to a logical control block within 
   * the host system (a plan, action, etc.). expectations that correspond
   * to control blocks are grouped by MCL by their corresponding expectation
   * group.
   * \param eg_key a host-generated key for the expectation group (a void*)
   */
  void declareExpectationGroup(string key,egkType eg_key);
  /** declares an expectation group with MCL.
   * an expectation group corresponds to a logical control block within 
   * the host system (a plan, action, etc.). expectations that correspond
   * to control blocks are grouped by MCL by their corresponding expectation
   * group.
   * \param eg_key a host-generated key for the expectation group (a void*)
   * \param parent_key an expectation group key for a parent control block
   * \param ref is a referent to an existing MCL stack frame (for interactive repairs)
   */
  void declareExpectationGroup(string key,
			       egkType eg_key,
			       egkType parent_key,
			       resRefType ref);

  /** indicates to MCL that a control block/expectation group has been aborted.
   * used to complete/destroy expectation groups without checking for
   * action effects on the way out.
   * \param eg_key the key for the aborted expectation group
   */
  void expectationGroupAborted(string key,egkType eg_key);

  /** indicates to MCL that a control block/expectation group has completed.
   * used to tell MCL to clean up the expectation stack and check the EG's
   * action effects on the way out. this particular parameter list allows
   * the host to perform a sensor vector update prior to effect checking.
   * \param eg_key the key for the completed expectation group
   * \param sv the current sensor vector (float array)
   * \param svs the size of the sensor vector
   */
  void expectationGroupComplete(string key,egkType eg_key,float *sv,int svs);

  /** indicates to MCL that a control block/expectation group has completed.
   * used to tell MCL to clean up the expectation stack and check the EG's
   * action effects on the way out.
   * \param eg_key the key for the completed expectation group
   */
  void expectationGroupComplete(string key,egkType eg_key);

  /** declares an expectation
   * \param group_key the expectation group to be associated with
   * \param code the exepectation type code (see #APICodes.h)
   * \param arg an argument to be passed to the expectation constructor
   */
  void declareExpectation(string key,egkType group_key,ecType code,float arg);

  /** declares an expectation
   * \param group_key the expectation group to be associated with
   * \param sensor the named sensor that the expectation is based on
   * \param code the exepectation type code (see #api/APICodes.h)
   */
  void declareExpectation(string key,egkType group_key,string self_sensor,
			  ecType code);

  /** declares an expectation
   * \param group_key the expectation group to be associated with
   * \param sensor the named sensor that the expectation is based on
   * \param code the exepectation type code (see #APICodes.h)
   * \param value an argument to be passed to the expectation constructor
   */
  void declareExpectation(string key,egkType group_key,string self_sensor,
			  ecType code,float value);

  void declareSelfExpectation(string key,egkType group_key,string self_sensor,
			      ecType code);
  void declareSelfExpectation(string key,egkType group_key,string self_sensor,
			      ecType code,float value);

  void declareObjExpectation(string key,egkType group_key,
			     string obj,string obs,ecType code);
  void declareObjExpectation(string key,egkType group_key,
			     string obj,string obs,ecType code,float value);

  // 2-sensor exps
  void declareSelfExpectation(string key,egkType group_key,
			      string obs1,string obs2,ecType eCode,
			      float value,int v2);
  void declareObjExpectation(string key,egkType group_key,
			     string obj,string obs1,string obs2,ecType eCode,
			     float value,int v2);

  /** declares an expectation that is not checked until a delay has been processed
   * \param delay the delay in seconds to wait before checking the expectation
   * \param group_key the expectation group to be associated with
   * \param sensor the named sensor that the expectation is based on
   * \param code the exepectation type code (see #APICodes.h)
   */
  void declareDelayedExpectation(string key,double delay,egkType group_key,string sensor,ecType code);

  /** declares an expectation that is not checked until a delay has been processed
   * \param delay the delay in seconds to wait before checking the expectation
   * \param group_key the expectation group to be associated with
   * \param sensor the named sensor that the expectation is based on
   * \param code the exepectation type code (see #APICodes.h)
   * \param value an argument to be passed to the expectation constructor
   */
  void declareDelayedExpectation(string key,double delay,egkType group_key,string sensor,ecType code,float value);
   
  // SYNCHRONIZATION
   
  /** indicates to MCL that a suggestion made has been implemented
   * \param referent a frame referent provided in an mclMonitorResponse
   */
  void suggestionImplemented(string key,resRefType referent);

  /** indicates to MCL that a suggestion made has been declined and
   * a new one should be generated.
   * \param referent a frame referent provided in an mclMonitorResponse
   */
  mclMonitorResponse* suggestionDeclined(string key,resRefType referent);

  /** indicates to MCL that a suggestion made has been implemented and failed
   * causes MCL to update its ontologies and possibly generate a new 
   * mclMonitorResponse suggesting further correction.
   * \param referent a frame referent provided in an mclMonitorResponse
   */
  mclMonitorResponse* suggestionFailed(string key,resRefType referent);

  /** indicates to MCL that a suggestion made has been ignoredx
   * MCL will not update/change the state of the frame referenced.
   * \param referent a frame referent provided in an mclMonitorResponse
   */
  void suggestionIgnored(string key,resRefType referent);

  /** indicates to MCL the feedback requested by a diagnostic response
   * MCL will issue a call to the active node's interpretFeedback() 
   * method.
   */
  void provideFeedback(string key,bool feedback, resRefType referent);

  /** instructs MCL to perform a monitoring pass over the host
   * Monitor is the primary interactive function and should be called 
   * regularly executed by the host. It causes MCL to check maintenance
   * expectations for faiures, creating frames as necessary, and revist
   * existing frames for updates. 
   * @param sv a sensor vector (a pointer to an array of float)
   * @param svs the length of the sensor vector
   * @return a vector of mclMonitorResponses (possibly empty)
   */
  responseVector  monitor(string key,float *sv,int svs);

  responseVector emptyResponse();

  // UTILITIES

  bool writeInitializedCPTs(string key);
  bool writeInitializedCosts(string key);

  // REENTRANT BEFAVOR
  bool setREB(string key, string reb);
  bool getREB(string key, string &reb);
  
  // DIAGNOSTICS

  void dumpMCL();

  void dumpOntologiesMostRecent(string key);

  void dumpMostRecentFrameDot(string key);
  void dumpFrameDot(string key,int dot);
  
  void dumpNPT();

  /** 
   *  sets output to quiet (minimal)
   */
  void beQuiet();

  /** 
   *  sets output to verbose (wordy)
   */
  void beVerbose();

  /** 
   *  turns on debugging output
   */
  void goDebug();

  /** 
   *  turns off debugging output
   */
  void noDebug();

  /** 
   *  sets output to a file
   *  @param fn the filename to write to
   */
  void setOutput(string fn);

  /** 
   *  sets output to std out
   */
  void setStdOut();


  namespace observables {

    void declare_observable_self(string mclkey,
				 string obs_name,double def_id=0);
    void declare_observable_object_type(string mclkey,
					string obj_type_name);
    void declare_object_type_field(string mclkey,
				   string obj_type_name, string obs_name);
    
    void notice_object_observable(string mclkey,
				  string obj_type,string obj_name);
    void notice_object_unobservable(string mclkey,string obj_name);
    
    void update_observable(string mclkey,string obs_name,double v);
    void update_observable(string mclkey,
			   string obj_name,string obs_name,double v);
    
    void set_obs_prop_self(string mclkey,string name,spkType key,spvType pv);
    void set_obs_prop(string mclkey,string objname,string obsname,
		      spkType key,spvType pv);

    void add_obs_legalval_self(string mclkey,string name,double lval);
    void add_obs_legalval_def(string mclkey,string objname,string obsname,
			  double lval);
    void add_obs_legalval_obj(string mclkey,string objname,string obsname,
			  double lval);
    void set_obs_legalrange_self(string mclkey,string obsname,
				 double low,double high);
    void set_obs_legalrange_def(string mclkey,string objname,string obsname,
				double low,double high);
    void set_obs_legalrange_obj(string mclkey,string objname,string obsname,
				double low,double high);

    void set_obs_noiseprofile_self(string mclkey,string obsname,spvType npKey);
    void set_obs_noiseprofile_self(string mclkey,string obsname,
				   spvType npKey,double param);
    void set_obj_obs_noiseprofile(string mclkey,string objname,string obsname,
				  spvType npKey);
    void set_obj_obs_noiseprofile(string mclkey,string objname,string obsname,
				  spvType npKey,double param);
      
    void dump_globo(string mclkey);
    string ov_as_string(string mclkey);
    void dump_obs_self(string k,string obsname);
    void dump_obs(string k,string objname,string obsname);

    typedef map<string,double>::iterator update_iterator;
    
    class update {
    public:
      update() {};
      void   set_update(string obs, double v);
      void   set_update(string obj, string obs, double v);
      double get_update(string obj,string obs);

      update_iterator begin() { return u_table.begin(); };
      update_iterator end  () { return u_table.end(); };
      string pp_key(update_iterator i)
	{ return i->first; };
      double value(update_iterator i)
	{ return i->second; };

    private:
      // string my_key;
      map<string,double> u_table;
    };

    void apply_update(string mcl_key,update& the_update);
    
  };
  
  
  bool updateObservables(string key,observables::update& the_update);
  responseVector monitor(string key,observables::update& the_update);
  void expectationGroupComplete(string key,egkType eg_key,
				observables::update& the_update);

};

#endif
