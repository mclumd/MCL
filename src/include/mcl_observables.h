#ifndef MCL_OBSERVABLES_H
#define MCL_OBSERVABLES_H

#include "mclEntity.h"
#include "mclSensorDef.h"
#include "fringe_linkage.h"
#include <list>
#include <map>
#include <algorithm>

#define MCL_OBS_ERROR_CHECKING 

/** \file
 * \brief class definitions for MCL's observable database
 */

namespace metacog {

  class observable;
  class observable_vector;
  class object_def;
  class legalspec;
  class noiseProfile;

  class observable : public mclEntity {
  public:  
    static const char* SELF_OBJNAME;
    
    observable(string nm) : mclEntity(nm) {};
    virtual double v_double()=0;
    virtual int    v_int()=0;
    virtual int    last_set()=0;
    virtual void   set_obs_prop(spkType key, spvType pv)=0;
    
    virtual void set_v_double(double v, int time_v)=0;
    virtual void set_v_int(int v,int time_v)=0;

    virtual void copy_legalspec(legalspec& source)=0;
    virtual void add_legalval(double v)=0;
    virtual void set_legalrange(double l, double h)=0;
    virtual bool is_legal()=0;

    virtual string to_string();
    
    static string make_name(string obj_n, string obs_n)
      { return obj_n+"."+obs_n; };
    static string make_self_named(string obs_n) 
      { return make_name(SELF_OBJNAME,obs_n); };
    static bool object_name_match(string obj_name,string obs_name);
    
    static bool is_self_sensor(string sname);
    static bool is_object_sensor(string sname);
   
    void dumpEntity(ostream *strm);
    
  };
  
  class basic_observable : public observable {
  public:
    basic_observable(mcl* m,string nm, double def_v=0.0) : 
      observable(nm),cached_value(def_v),last_set_v(0),l_spec(NULL) {
      sensor_definition = new mclSensorDef(nm,MCL());
    };

    basic_observable(mcl* m,string nm,mclSensorDef* md,double def_v=0.0) : 
      observable(nm),cached_value(def_v),last_set_v(0),l_spec(NULL) {
      sensor_definition = md->clone();
    };

    basic_observable(mcl* m,string nm,mclSensorDef* md,legalspec* lspec,
		     double def_v=0.0);
    
    virtual double v_double() { return cached_value; };
    virtual int   v_int()     { return (int)cached_value; };
    virtual int   last_set()  { return last_set_v; };
    
    virtual void set_v_double(double v, int time_v) 
      {  cached_value = v; last_set_v = time_v; };
    virtual void set_v_int(int v,int time_v)
      { cached_value = (double)v; last_set_v = time_v; };

    virtual void copy_legalspec(legalspec& source);
    virtual void add_legalval(double v);
    virtual void set_legalrange(double l, double h);
    virtual bool is_legal();
    
    virtual void   set_obs_prop(spkType key, spvType pv)
      { sensor_definition->setSensorProp(key,pv); };
    
    virtual mcl* MCL() { return my_mcl; };
    virtual string baseClassName() { return "basic_observable"; };
    
    virtual string to_string();
    
    mclSensorDef* get_sensorDef()
      { return sensor_definition; };
    legalspec*    get_legalSpec()
      { return l_spec; };
    
  protected:
    double        cached_value;
    int           last_set_v;
    mclSensorDef* sensor_definition;
    mcl*          my_mcl;
    legalspec*    l_spec;
    
  };

/** OBJECT DEFINITIONS
 */

  typedef map<string,mclSensorDef*> fieldmap;
  typedef map<string,legalspec*>    fieldmap_legal;

  class object_def : public mclEntity {
  public:
   object_def(mcl* m,string nm) : mclEntity(nm),my_mcl(m) {};
   
   void add_field_def(string fieldname, mclSensorDef& fsd);
   void add_object_fields_to_ov(observable_vector* ov, string object_name);
   
   fieldmap::iterator fmi_begin() { return object_fields.begin(); };
   fieldmap::iterator fmi_end()   { return object_fields.end(); };
   
   virtual mcl* MCL() { return my_mcl; };
   virtual string baseClassName() { return "obj_def"; };
   
   void set_obs_prop(string obsname,spkType key, spvType pv);
   void add_obs_legalval(string obsname,double val);
   void set_obs_legalrange(string obsname,double l,double h);

   string to_string();
   
 protected:
   fieldmap       object_fields;
   fieldmap_legal object_legalvals;
   
   mcl* my_mcl;
   
 };


 /** OBJECT VECTOR
  */
 
 typedef map<string,object_def*> od_map;
 typedef map<string,observable*> ob_map;
 typedef map<string,string>      string_lookup;
 
 class observable_vector : public mclEntity {
 public:
   
   observable_vector(mcl* its_mcl, string vname) : 
     mclEntity(vname),my_mcl(its_mcl) {};
   
   double v_double(string obs);
   int    v_int(string obs);
   double v_double(string object,string obs);
   int    v_int(string object,string obs);
   
   void add_observable(observable* def);
   
   void add_observable_object_def(object_def* od);
   void add_object_field_def(string objname,string fieldname,mclSensorDef& sd);
   
   void observe_observable_object(string name, string defname);
   void stow_observable_object(string name);
   
   bool recall_observable_object(string name);
   void delete_observable_object(string name);

   void recover_from_memory_to_active(string name,object_def* objdef);
   
   void observable_update(string obs_name,double v) 
     { observable_update_pp(observable::make_self_named(obs_name),v); };
   void observable_update(string obj_name,string obs_name,double v)
     { observable_update_pp(observable::make_name(obj_name,obs_name),v); };
   
   void set_obs_prop_self(string name,spkType key,spvType pv);
   void set_obs_prop(string objname,string obsname,spkType key,spvType pv);

   // 6-8-2010 MS
   // the semantics of the old OBJ functions were found to be ambiguous
   // in that one might want to effect an existig observable *OR*
   // the object definition and all existing observables that match
   //
   void add_obs_legalval_self(string oname,double lval);
   void set_obs_legalrange_self(string oname,double l, double h);
   void add_obs_legalval_def(string o_type_name,string obsname,double lval);
   void add_obs_legalval_obj(string o_inst_name,string obsname,double lval);
   void set_obs_legalrange_def(string o_type_name,string obsname,
			       double l, double h);
   void set_obs_legalrange_obj(string o_inst_name,string obsname,
			       double l, double h);

   bool obs_is_legal(string obsname);
   bool obs_is_legal(string objname,string obsname);

   void establish_np(string self_name,noiseProfile* the_np);
   void establish_np(string obj_name,string obs_name,noiseProfile* the_np);
   
   void dump_obs(string obsname);
   void dump_obs(string objname, string obsname);
   
   // void activate_observable_object(string name);
   // void deactivate_observable_object(string name);
   
   string active2string();
   void   dump();
   
   virtual mcl* MCL() { return my_mcl; };
   virtual string baseClassName() { return "obs_vec"; };

   // the following are PP (pre-processed), meaning the key string has
   // been pre-processed... they are mainly for use by mcl for checking
   // up on expectation details
   void observable_update_pp(string pp_name,double v);
   mclSensorDef* get_sensorDef_pp(string obs);
   legalspec*    get_legalSpec_pp(string obs);
   double v_double_pp(string sn);
   int    v_int_pp(string sn);
   bool   is_legal_pp(string sn);
   
 protected:
   ob_map        active;        // 
   ob_map        memory;        // 
   od_map        object_defs;   // obj_type_name->definition
   string_lookup object_defmap; // obj_name->obj_type_name
   list<string>  current_objects; // objects currently being observed
   
   mcl* my_mcl;
   
   string vector2string(ob_map& map);
   void observe_new_object(string defname, string obname);
   void delete_from_ob_map(ob_map& map,string ob_name);
   
 };

 class legalspec : public produces_linktags {
 public:
   legalspec() {};
   virtual ~legalspec() {};
   virtual legalspec* clone()=0;
   
   virtual bool is_legal(double v)=0;
   virtual string to_string()=0;

   virtual void addLinkTags(linkTags_t& rv);

 };

 class itsallgood : public legalspec {
 public: 
   itsallgood() : legalspec() {};
   virtual ~itsallgood() {};
   virtual legalspec* clone() { return new itsallgood(*this); };

   virtual bool is_legal(double v) { return true; };
   virtual string to_string() { return "[..]"; };

   virtual void addLinkTags(linkTags_t& rv);

 };

 class legalset : public legalspec {
 public:
   legalset() : legalspec() {};
   legalset(double v) : legalspec() { legals.push_back(v); };
   virtual ~legalset() {};
   virtual legalspec* clone() { return new legalset(*this); };

   virtual bool is_legal(double v) 
   { return (find(legals.begin(),legals.end(),v) != legals.end()); };

   void add_legal(double v) { legals.push_back(v); };
   void rem_legal(double v) { legals.remove(v); };
   virtual string to_string();
   
   virtual void addLinkTags(linkTags_t& rv);

 private:
   list<double> legals;

 };

 class legalrange : public legalspec {
 public:
   legalrange(double l, double h) : legalspec(), low(l), high(h) {};

   virtual bool is_legal(double v);

   virtual legalspec* clone() { return new legalrange(*this); };

   void set_low(double l) { low = l; };
   void set_high(double h) { high = h; };
   virtual string to_string();

   virtual void addLinkTags(linkTags_t& rv);
    
 private:
   double low,high;

 };

};

#endif
