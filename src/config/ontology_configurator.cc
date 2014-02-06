#include "ontology_config.h"
#include "mclLogging.h"
#include "../../include/umbc/exceptions.h"
#include "mcl.h"
#include "mclFrame.h"

#include <map>

using namespace std;
using namespace metacog;

map<string,ontology_configuration*> _config_base;

// require_config will create an ontology_configuration object
// and store it in _config_base along with the cKey for the
// specified domain/agent/controller

string ontology_configurator::require_config(string ontology,string domain) {
  string id = cKey_for(domain);
  umbc::uLog::annotate(MCLA_MSG,"[mcl/oc]:: config required: "+id);
  if (_config_base.find(id) == _config_base.end()) {
    _config_base[id]=new ontology_configuration(ontology,domain);
  }
  return id;
}

string ontology_configurator::require_config(string ontology,string domain, string agent) {
  string id = cKey_for(domain,agent);
  umbc::uLog::annotate(MCLA_MSG,"[mcl/oc]:: config required: "+id);
  if (_config_base.find(id) == _config_base.end()) {
    _config_base[id]=new ontology_configuration(ontology,domain,agent);
  }
  return id;
}

string ontology_configurator::require_config(string ontology,string domain, string agent, string controller) {
  string id = cKey_for(domain,agent,controller);
  umbc::uLog::annotate(MCLA_MSG,"[mcl/oc]:: config required: "+id);
  if (_config_base.find(id) == _config_base.end()) {
    _config_base[id]=
      new ontology_configuration(ontology,domain,agent,controller);
  }
  return id;
}

string ontology_configurator::cKey2path(string cKey) {
  return cKey;
}
  
bool ontology_configurator::apply_config(mclFrame* f) {
  // first try to get the frame-parent's cKey
  if (f && f->MCL()) {
    umbc::uLog::annotate(MCLA_MSG,
			"[mcl/oc]:: "+ f->MCL()->entityName() + 
			" attempting to apply config '" +
			f->MCL()->getConfigKey() + "'....");
    if (f->MCL()->getConfigKey() == "") {
      umbc::uLog::annotate(MCLA_WARNING,
			  "[mcl/oc]:: mcl " + 
			  f->MCL()->entityName() +
			  " not configurated properly, using default config.");
      f->MCL()->setConfigKey(require_config(f->MCL()->getOntologyBase(),
					    "default"));
    }
    ontology_configuration* toc = _config_base[f->MCL()->getConfigKey()];
    if (toc == NULL) {
      umbc::exceptions::signal_exception("configuration key '"+f->MCL()->getConfigKey()+"' is empty in _config_base.");
    }
    else {
      toc->apply_rc_config(f);
      toc->apply_cpt_config(f);
      umbc::uLog::annotate(MCLA_MSG,"[mcl/oc]:: configuration applied.");
    }
  }
  else {
    umbc::exceptions::signal_exception("attempted to restore configuration for NULL frame/MCL.");
  }
  return true;
}

string ontology_configurator::cKey_for(string domain) {
  return domain;
}
string ontology_configurator::cKey_for(string domain, string agent) {
  return domain+"/"+agent;
}
string ontology_configurator::cKey_for(string domain, string agent, string controller) {
  return domain+"/"+agent+"/"+controller;
}

string ontology_configurator::configPath(mclEntity* m,string fn) {
  if (m && m->MCL()) {
    if (m->MCL()->getConfigKey() == "") {
      umbc::uLog::annotate(MCLA_WARNING,
			  "[mcl/oc]:: autoPath used for unconfigured MCL object " + m->entityName() + " -- using path 'unconfig'");
      m->MCL()->setConfigKey("unconfig");
    }
    return cKey2path(m->MCL()->getConfigKey())+"/"+fn;
  }
  else {
    umbc::uLog::annotate(MCLA_ERROR,
			"[mcl/oc]:: autoPath used for lost (NULL/unattached) MCL object " + m->entityName() + " -- using path 'lost'");
    return "lost/"+fn;
  }  
}
