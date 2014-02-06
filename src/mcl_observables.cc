#include "../../include/umbc/exceptions.h"
#include "mclLogging.h"
#include "mcl_observables.h"
#include "mcl.h"

#include <list>

#define MCL_OBS_ERROR_CHECKING

using namespace metacog;
using namespace umbc;

/***************************************************************
                   start of observable code
***************************************************************/

const char* observable::SELF_OBJNAME = "self";

bool observable::object_name_match(string obj_name,string obs_name) {
  string onps = obj_name+".";
  return (strncmp(onps.c_str(),obs_name.c_str(),onps.length()) == 0);
}

string observable::to_string() {
  char nyaya[512];
  sprintf(nyaya,"( obs %s (%.3lf) )",entityBaseName().c_str(),v_double());
  return nyaya;
}

void observable::dumpEntity(ostream *strm) {
  *strm << to_string() << endl;
}

bool observable::is_self_sensor(string sname) {
  static string sbase = make_self_named("");
  return (strncmp(sbase.c_str(),sname.c_str(),sbase.length())==0);
}

bool observable::is_object_sensor(string sname) {
  static string sbase = make_self_named("");
  return ((strncmp(sbase.c_str(),sname.c_str(),sbase.length())!=0) &&
	  (sname.find('.') != string::npos));
}

basic_observable::basic_observable(mcl* m,string nm,mclSensorDef* md,
				   legalspec* lspec,double def_v) : 
  observable(nm),cached_value(def_v),last_set_v(0),l_spec(NULL) {
  sensor_definition = md->clone();
  // is it okay to not have a legalspec?
  // the else clause was added 5/2010 to fix a NULL PTR bug
  if (lspec) 
    l_spec=lspec->clone();
  else 
    l_spec=NULL;
}

void basic_observable::copy_legalspec(legalspec& source) {
  if (l_spec) {
    delete l_spec;
  }
  l_spec = source.clone();
}

void basic_observable::add_legalval(double v) {
  legalset* lsetver = NULL;
  if (!l_spec) { // if it's NULL, set to a new lset
    lsetver = new legalset();
    l_spec = lsetver;
  }
  else { // otherwise make sure it's a legalset
    lsetver = dynamic_cast<legalset*>(l_spec);
    if (!lsetver) {
      throw BadDeclaration("attempted to add a legalval to "+entityBaseName()+" but it is using a non-set legal spec.");
    }
  }
  // if you get here this should be okay
  lsetver->add_legal(v);
}

void basic_observable::set_legalrange(double l,double h) {
  legalrange* lranver = NULL;
  if (!l_spec) { // if it's NULL, set to a new lset
    lranver = new legalrange(l,h);
    l_spec = lranver;
  }
  else { // otherwise make sure it's a legalset
    lranver = dynamic_cast<legalrange*>(l_spec);
    if (!lranver) {
      throw BadDeclaration("attempted to set legalrange for "+entityBaseName()+" but it is using a non-range legal spec.");
    }
  }
  // if you get here this should be okay
  lranver->set_low(l);
  lranver->set_high(h);
}

bool basic_observable::is_legal() {
  if (l_spec) {
    cout << "lspec=" << l_spec->to_string() << endl;
    return l_spec->is_legal(v_double());
  }
  else {
    cout << "is legal by default (no lspec)." << endl;
    return true;
  }
}

string basic_observable::to_string() {
  if (l_spec) {
    char nyaya[512];
    char ov = ((is_legal()) ? ' ' : '*');
    sprintf(nyaya,"( obs %s (%.3lf%c) %s )",entityBaseName().c_str(),
	    v_double(),ov,l_spec->to_string().c_str());
    return nyaya;
  }
  else 
    return observable::to_string();
}


/***************************************************************
                   start of object_def code
***************************************************************/

string object_def::to_string() {
  string r= "(odef "+entityBaseName();
  for (fieldmap::iterator i = object_fields.begin();
       i != object_fields.end();
       i++) {
    r+=i->first+" ";
  }
  r+=")";
  return r;
}

void object_def::add_object_fields_to_ov(observable_vector* ov, 
					 string object_name) {
  for (fieldmap::iterator i = object_fields.begin();
       i != object_fields.end();
       i++) {
    // push into the observable vector
    observable* noob = 
      new basic_observable(MCL(),
			   observable::make_name(object_name,i->first),
			   i->second,
			   object_legalvals[i->first]);
    cout << "adding observable " << noob->to_string() << endl;
    ov->add_observable(noob);
  }
}

void object_def::add_field_def(string fieldname, mclSensorDef& sd) {
  // IT SHOULD CLONE THE SENSORDEF!!
  if (object_fields[fieldname] != NULL) {
    mclSensorDef* osd = object_fields[fieldname];
    cout << "field def " << fieldname << " being overwritten (destroyed)."
	 << endl;
    delete osd;
  }
  mclSensorDef* nu = sd.clone();
  cout << "adding field def '" << fieldname << "', cloning "
       << hex << &sd << " to " << hex << nu << endl;
  nu->dumpEntity(&cout); cout << endl;
  object_fields[fieldname] = nu;
}

void object_def::set_obs_prop(string fieldname,spkType key, spvType pv) {
  if (object_fields[fieldname]) {
    object_fields[fieldname]->setSensorProp(key,pv);
  }
}

void object_def::add_obs_legalval(string obsname,double val) {
  legalspec* current = object_legalvals[obsname];
  legalset*  setver  = NULL;
  if (!current) { // if it's NULL, set to a new lset
    // cout << "   ... Creating legalset for " << obsname << endl;
    setver = new legalset();
    object_legalvals[obsname] = setver;
  }
  else { // otherwise make sure it's a legalset
    setver = dynamic_cast<legalset*>(current);
    if (!setver) {
      throw BadDeclaration("attempted to add a legalval to "+entityBaseName()+" but it is using a non-set legal spec.");
    }
  }
  // if you get here this should be okay
  setver->add_legal(val);
}

void object_def::set_obs_legalrange(string obsname,double l, double h) {
  legalspec*  current = object_legalvals[obsname];
  legalrange* ranver  = NULL;
  if (!current) { // if it's NULL, set to a new lset
    ranver = new legalrange(l,h);
    object_legalvals[obsname] = ranver;
    return;
  }
  else { // otherwise make sure it's a legalset
    ranver = dynamic_cast<legalrange*>(current);
    if (!ranver) {
      throw BadDeclaration("attempted to set legalrange for "+entityBaseName()+" but it is using a non-range legal spec.");
    }
  }
  // if you get here this should be okay
  ranver->set_low(l);
  ranver->set_high(h);
}

/***************************************************************
                start of observable_vector code
***************************************************************/

void observable_vector::add_observable(observable* obs) {
  ob_map::iterator fp = active.find(obs->entityBaseName());
  if (fp != active.end()) {
    // kill the old one?
    if (fp->second != NULL) { // why would it?
      observable* ootk = fp->second;
      delete ootk;
    }
  }
  active[obs->entityBaseName()]=obs;
}

void observable_vector::add_observable_object_def(object_def* od) {
  od_map::iterator q = object_defs.find(od->entityBaseName());
  if (q != object_defs.end()) {
    delete object_defs[od->entityBaseName()];
    object_defs[od->entityBaseName()]=NULL;
  }
  object_defs[od->entityBaseName()]=od;
}

void observable_vector::observe_observable_object(string defname,string name) {
  if (!recall_observable_object(name)) {
    cout << defname << ":" << name << " not in memory, creating..." << endl;
    observe_new_object(defname,name);
  }
}

void observable_vector::observe_new_object(string defname, string obname) {
  // requires creation of object (not in memory)
  // step 0: retrieve the object def
  object_def* tod = object_defs[defname];
  if (tod != NULL) {
    // step 1: create a mapping from the object name to its type
    object_defmap[obname]=defname;

    // step 2: add all object fields to the active_list
    tod->add_object_fields_to_ov(this,obname);
    cout << "object created..." << endl;
    current_objects.push_back(obname);
  }

#ifdef MCL_OBS_ERROR_CHECKING
  else {
    throw MissingObjectDefException("attempting to observe an undefined object: "+obname+"/"+defname);
  }
#endif

}

bool observable_vector::recall_observable_object(string name) {
  // test for active/current memory
  if (find(current_objects.begin(),current_objects.end(),name) != 
      current_objects.end()) {
    cout << name << " is already in active memory."<< endl;
    return true;
  }
  // no? find the object typename
  string obj_type = object_defmap[name];
  if (obj_type == "") {
    cout << "failed to find " << name << " in obj_defmap." << endl;
    return false;
  }
  else {
    // okay, now look up the typedef
    object_def* objdef = object_defs[obj_type];
    if (objdef == NULL) {
      cout << "failed to find " << obj_type << " in obj_defs." << endl;
      return false;
    }
    else {
      // now move from memory to active/current
      recover_from_memory_to_active(name,objdef);
      return true;
    }
  }
}

// iterate over field defs and pull observables from memory to active
void observable_vector::recover_from_memory_to_active(string name,
						   object_def* objdef) {
  for (fieldmap::iterator i = objdef->fmi_begin();
       i != objdef->fmi_end();
       i++) {
    // this is a little messy since there this code is taken from
    // a object_def function (add_obj_fields_to_ov)
    string i_obsname = observable::make_name(name,i->first);
    observable* i_obs_from_mem = memory[i_obsname];
    if (i_obs_from_mem != NULL) {
      // push into the observable vector
      add_observable(i_obs_from_mem);
      memory.erase(i_obsname);
    }
    else {
      cout << "WARNING: recall should work but field '" 
	   << i->first << "' is missing in observable memory." << endl;
      observable* noob = 
	new basic_observable(MCL(),i_obsname,i->second);
      add_observable(noob);
    }
  }
  current_objects.push_back(name);
}

//< takes all the observables in _active_ that match ob_name
//< and puts them in _memory_
void observable_vector::stow_observable_object(string ob_name) {
  // i hate this but erase invalidates the iterator....
  list<string> cache;
  for (ob_map::iterator ai = active.begin();
       ai != active.end();
       ai++) {
    if (observable::object_name_match(ob_name,ai->first))
      cache.push_back(ai->first);
  }
  while (!cache.empty()) {
    observable* item_to_stow = active[cache.front()];
    if (item_to_stow != NULL) {
      memory[cache.front()] = active[cache.front()];
      active.erase(cache.front());
    }
    cache.pop_front();
  }
  current_objects.remove(ob_name);
}


void observable_vector::delete_from_ob_map(ob_map& map,string ob_name) {
  // i hate this but erase invalidates the iterator....
  list<string> cache;
  for (ob_map::iterator ai = map.begin();
       ai != map.end();
       ai++) {
    if (observable::object_name_match(ob_name,ai->first))
      cache.push_back(ai->first);
  }
  while (!cache.empty()) {
    observable* item_to_delete = map[cache.front()];
    if (item_to_delete != NULL) {
      delete item_to_delete;
      map.erase(cache.front());
    }
    cache.pop_front();
  }
}

void observable_vector::delete_observable_object(string name) {
  delete_from_ob_map(active,name);
  delete_from_ob_map(memory,name);
}

void observable_vector::add_object_field_def(string objname,
					     string fieldname,
					     mclSensorDef& sd) {
  object_def* tod = object_defs[objname];
  if (tod != NULL) {
    tod->add_field_def(fieldname,sd);
  }
  else {
#ifdef MCL_OBS_ERROR_CHECKING
    throw MissingObjectDefException("attempt to add field to an undefined object: "+objname);
#else
    add_observable_object_def(object_def* od);    
#endif

  }
}

double observable_vector::v_double(string obs) {
  return v_double(observable::SELF_OBJNAME,obs);
}

int    observable_vector::v_int(string obs) {
  return v_int(observable::SELF_OBJNAME,obs);
}

double observable_vector::v_double(string obj,string obs) {
  string obsn = observable::make_name(obj,obs);

#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(obsn) == active.end())
    throw MissingObjectException("attempt to get value for "+obsn+" failed. not an active observable.");
#endif

  return (active[obsn])->v_double();
}

int observable_vector::v_int(string obj,string obs) {
  string obsn = observable::make_name(obj,obs);

#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(obsn) == active.end())
    throw MissingObjectException("attempt to get value for "+obsn+" failed. not an active observable.");
#endif
      
  return (active[obsn])->v_int();
}

double observable_vector::v_double_pp(string sn) {
#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(sn) == active.end())
    throw MissingObjectException("attempt to get value for "+sn+" failed. not an active observable.");
#endif

  return (active[sn])->v_double();
}

int observable_vector::v_int_pp(string sn) {
#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(sn) == active.end())
    throw MissingObjectException("attempt to get value for "+sn+" failed. not an active observable.");
#endif

  return (active[sn])->v_int();
}

bool observable_vector::is_legal_pp(string sn) {
#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(sn) == active.end())
    throw MissingSensorException(sn);
#endif

  return (active[sn])->is_legal();
}

string observable_vector::vector2string(ob_map& map) {
  string dtw = "";
  if (&map == &active) dtw = "active";
  else if (&map == &memory) dtw = "memory";
  else dtw = "unknown";
  char buff[512];
  string r = "( " + baseClassName() +"("+dtw+")" + " | ";
  for (ob_map::iterator i = map.begin();
       i != map.end();
       i++) {
    if (!(i->second)) {
      sprintf(buff,"%s=inactive ",i->first.c_str());
      r+=buff;
    }
    else {
      sprintf(buff,"%s=%.2f ",i->first.c_str(),i->second->v_double());
      r+=buff;
    }
  }
  return r+" )";
}

void observable_vector::observable_update_pp(string pp_name, double v) {
  if (active.find(pp_name) != active.end()) {
    observable* oo = active[pp_name];
    oo->set_v_double(v,MCL()->tickNumber());
  }
#ifdef MCL_OBS_ERROR_CHECKING
  else {
    throw MissingSensorException(pp_name);
  }
#endif
  
}

string observable_vector::active2string() {
  return vector2string(active);
}

void observable_vector::dump() {
  uLog::annotate(MCLA_MSG,vector2string(active));
  uLog::annotate(MCLA_MSG,vector2string(memory));
}

bool observable_vector::obs_is_legal(string obsname) {
  return obs_is_legal(observable::SELF_OBJNAME,obsname);
}

bool observable_vector::obs_is_legal(string objname,string obsname) {
  string obsn = observable::make_name(objname,obsname);

#ifdef MCL_OBS_ERROR_CHECKING
  if (active.find(obsn) == active.end())
    throw MissingObjectException("attempt to get value for "+obsn+" failed. not an active observable.");
#endif

  return (active[obsn])->is_legal();
}

void observable_vector::dump_obs(string obs) {
  string kee = observable::make_self_named(obs);
  if (active.find(kee) != active.end()) {
    active[kee]->dumpEntity(uLog::log);
  }
  else if (memory.find(kee) != memory.end()) {
    memory[kee]->dumpEntity(uLog::log);
  }
  else cout << "(unknown observable " << kee << ")" << endl;
}

void observable_vector::dump_obs(string obj,string obs) {
  string kee = observable::make_name(obj,obs);
  if (active.find(kee) != active.end()) {
    active[kee]->dumpEntity(uLog::log);
  }
  else if (memory.find(kee) != memory.end()) {
    memory[kee]->dumpEntity(uLog::log);
  }
  else *uLog::log << "(unknown observable " << kee << ")" << endl;
}

void observable_vector::set_obs_prop_self(string name,spkType key,spvType pv) {
  // THE FOLLOWING COMMENTED LINE WON'T WORK!!
  //  set_obs_prop(observable::SELF_OBJNAME,name,key,pv);

  string kee = observable::make_self_named(name);
  if (active.find(kee) != active.end()) {
    active[kee]->set_obs_prop(key,pv);
    cout << "self prop reset: " << kee << endl;
  }
}

void observable_vector::set_obs_prop(string obj_type_name,string obsname,
				     spkType key,spvType pv) {
  cout << "obj/prop set in progress: " << obj_type_name 
       << "." << obsname << " = [" << key << ":" 
       << dec << (int)pv << "]" << endl;
  // okay, this is actually a little hard because we need to find
  // all object defs *and* active/memorized observables that match

  // first, fix object def (if any)
  if (object_defs.find(obj_type_name) != object_defs.end()) {
    object_defs[obj_type_name]->set_obs_prop(obsname,key,pv);
    cout << "obj-def prop reset: " << obj_type_name 
	 << "." << obsname << " = [" << key << ":" << dec << (int)pv << "]" << endl;
  }

  // next, fix active objects and memory
  for (string_lookup::iterator sli = object_defmap.begin();
       sli != object_defmap.end();
       sli++) {
    if (sli->second == obj_type_name) {
      string kee = observable::make_name(sli->first,obsname);
      if (active.find(kee) != active.end()) {
	active[kee]->set_obs_prop(key,pv);
	cout << "active obj prop reset: " << kee << endl;
      }
      else if (memory.find(kee) != memory.end()) {
	memory[kee]->set_obs_prop(key,pv);    
	cout << "memory obj prop reset: " << kee << endl;
      }
    }
  }
}

void observable_vector::add_obs_legalval_def(string obj_type_name,
					     string obsname,
					     double lval) {
  // okay, this is actually a little hard because we need to find
  // all object defs *and* active/memorized observables that match
  // first, fix object def (if any)
  if (object_defs.find(obj_type_name) != object_defs.end()) {
    object_defs[obj_type_name]->add_obs_legalval(obsname,lval);
  }
  else {
    throw MissingObjectDefException("while adding to legalset: "+obj_type_name+" is not defined.");
  }

  // next, fix active objects and memory
  for (string_lookup::iterator sli = object_defmap.begin();
       sli != object_defmap.end();
       sli++) {
    if (sli->second == obj_type_name) {
      add_obs_legalval_obj(sli->first, obsname,lval);
    }
  }
}

void observable_vector::add_obs_legalval_obj(string obj_inst_name,
					     string obsname,
					     double lval) {

  string kee = observable::make_name(obj_inst_name,obsname);
  if (active.find(kee) != active.end()) {
    active[kee]->add_legalval(lval);
  }
  else if (memory.find(kee) != memory.end()) {
    memory[kee]->add_legalval(lval);    
  }
  else {
    throw InternalMCLException("while adding to legalspec: "+obj_inst_name+" is in defmap but is not active or in memory.");
  }
}

void observable_vector::set_obs_legalrange_def(string obj_type_name,string obsname,
					       double l, double h) {
  cout << "otype legalrange being set: " << obj_type_name << "."
       << obsname << " "
       << l << "..." << h << endl;
  // okay, this is actually a little hard because we need to find
  // all object defs *and* active/memorized observables that match

  // first, fix object def (if any)
  if (object_defs.find(obj_type_name) != object_defs.end()) {
    object_defs[obj_type_name]->set_obs_legalrange(obsname,l,h);
  }
  else {
    throw MissingObjectException(obj_type_name);
  }

  // next, fix active objects and memory
  for (string_lookup::iterator sli = object_defmap.begin();
       sli != object_defmap.end();
       sli++) {
    if (sli->second == obj_type_name) {
      set_obs_legalrange_obj(sli->first,obsname,l,h);
    }
  }
}

void observable_vector::set_obs_legalrange_obj(string obj_inst_name,
					       string obsname,
					       double l, double h) {
  string kee = observable::make_name(obj_inst_name,obsname);
  if (active.find(kee) != active.end()) {
    active[kee]->set_legalrange(l,h);
  }
  else if (memory.find(kee) != memory.end()) {
    memory[kee]->set_legalrange(l,h);    
  }
  else {
    umbc::exceptions::signal_exception("while setting legalrange: "+obj_inst_name+" is in defmap but is not active or in memory.");
  }
}

void observable_vector::add_obs_legalval_self(string obsname,double lval) {
  string kee = observable::make_self_named(obsname);
  if (active.find(kee) != active.end()) {
    active[kee]->add_legalval(lval);
    cout << "active legalval added: " << kee << " " << lval << endl;
  }
}

void observable_vector::set_obs_legalrange_self(string obsname,
						double l, double h) {
  string kee = observable::make_self_named(obsname);
  if (active.find(kee) != active.end()) {
    active[kee]->set_legalrange(l,h);
    cout << "active legalrange set: " << kee << " " 
	 << l << "..." << h << endl;
  }
}

mclSensorDef* observable_vector::get_sensorDef_pp(string obs) {
  // cout << " attempting to look up " << obs << endl;
  if (active.find(obs) != active.end()) {
    // here we have to assume that it's a basic_observable so we can get
    // the sensorDef
    return ((basic_observable*)active[obs])->get_sensorDef();
  }
  else return NULL;
}

legalspec* observable_vector::get_legalSpec_pp(string obs) {
  if (active.find(obs) != active.end()) {
    return ((basic_observable*)active[obs])->get_legalSpec();
  }
  else return NULL;
}
  
//////////////// NOISE PROFILE STUFF

void observable_vector::establish_np(string self_name,noiseProfile* the_np) {
  noiseProfile::establishNoiseProfileFor(MCL()->getMAkey(),observable::make_self_named(self_name),the_np);
}

void observable_vector::establish_np(string obj_name,string obs_name,
				     noiseProfile* the_np) {
  noiseProfile::establishNoiseProfileFor(MCL()->getMAkey(),observable::make_name(obj_name,obs_name),the_np);
}

