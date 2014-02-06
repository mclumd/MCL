#include "ontology_config.h"
#include "mclLogging.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

using namespace std;
using namespace metacog;

#ifdef WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

ontology_configuration::ontology_configuration(string ontology,string domain) 
  : conf_id(ontology_configurator::cKey_for(domain)),
    onto_id(ontology),source_path(""),write_path(""),specificity(1) {
  load_initial_config();
}

ontology_configuration::ontology_configuration(string ontology,string domain,
					       string agent)
  : conf_id(ontology_configurator::cKey_for(domain,agent)),
    onto_id(ontology),source_path(""),write_path(""),specificity(2) {
  load_initial_config();
}

ontology_configuration::ontology_configuration(string ontology,string domain,
					       string agent,string controller) 
  : conf_id(ontology_configurator::cKey_for(domain,agent,controller)),
    onto_id(ontology),source_path(""),write_path(""),specificity(3) {
  load_initial_config();
}

string ontology_configuration::cptFile(string path,string ontology_base_name) {
  return path+"/"+ontology_base_name+"_cpt.mcl";
}

bool ontology_configuration::load_initial_config() {
  // first compute the path
  string pth = determine_paths();
  cout << "determined load  path is '" << source_path << "'" << endl;
  cout << "determined write path is '" << write_path << "'" << endl;
  bool rv = cpt_cfg::load_cpts(cpt_configuration,cptFile(pth,onto_id));
  // rv &=  rc_cfg::load_response_costs(pth);
  return rv;
}

string ontology_configuration::determine_write_path() {
  char *pathToMCLCore = getenv("MCL_CONFIG_PATH");
  char *pathToHome = getenv("HOME");
  string pth=".";
  string ptc=".";
  if (pathToMCLCore != NULL) ptc = pathToMCLCore;
  ptc+="/config/";
  if (pathToHome != NULL) pth = pathToHome;
  
  if (ensure_write_path(ptc)) {
    // config is writable... ensure dirs exist
    ptc+=ontology_configurator::cKey2path(conf_id);
    make_path(ptc);
    write_path = ptc;
    return write_path;
  }
  else if (ensure_write_path(pth)) {
    // if HOME doesn't work, you are screwed...
    pth+="/.mcl/"+ontology_configurator::cKey2path(conf_id);
    make_path(pth);
    write_path = pth;
    return write_path;    
  }
  else {
    umbc::uLog::annotate(MCLA_WARNING,"[mcl/oc]:: could not confirm user preference for write path using MCL_CONFIG_PATH or HOME. Using CWD as root for writing configurations.");
    write_path = "./";
    return write_path;
  }
}

string ontology_configuration::determine_read_path() {
  char *pathToHome = getenv("HOME");
  char *pathToMCLConfig = getenv("MCL_CONFIG_PATH");
  string pth=".";
  string ptc=".";
  if (pathToMCLConfig != NULL) ptc = pathToMCLConfig;
  ptc+="/config/";
  if (pathToHome != NULL) pth = pathToHome;
  pth+="/.mcl/";

  string relax_path = conf_id;
  bool confirmed = false;
  int relax_count = 0;

  do {
    // prefer config path
    if ((pathToMCLConfig != NULL) && 
	(ensure_file_readable(cptFile(ptc+relax_path,onto_id)))) {
      source_path=ptc+relax_path;
      return source_path;
    }
    // but take HOME if it is more specific
    if ((pathToHome != NULL) && 
	(ensure_file_readable(cptFile(pth+relax_path,onto_id)))) {
      source_path=pth+relax_path;
      return source_path;
    }
    // relax the paths and try again
    relax_count++;
    relax_path = relax_config_path(relax_path);
  } while (relax_count < specificity);

  // failed to relax -- attempt to find default config
  if ((pathToMCLConfig != NULL) && 
      ensure_file_readable(cptFile(ptc+"/default/",onto_id))) {
    source_path=ptc+"/default/";
    return source_path;
  }
  cout << "checking HOME binding for default config..." << endl;
  if ((pathToHome != NULL) && 
      ensure_file_readable(cptFile(pth+"/default/",onto_id))) {
    source_path=pth+"/default/";
    return source_path;
  }
  // give up, return empty config path
  umbc::uLog::annotate(MCLA_WARNING,"[mcl/oc]:: could not find config in MCL_CONFIG_PATH or HOME. Ontologies will not be configured by ontology configurator.\n");
  return "";
}

string ontology_configuration::determine_paths() {
  determine_read_path();
  determine_write_path();
  return source_path;
}

bool ontology_configuration::apply_rc_config(mclFrame* f) {
  return rc_cfg::apply_rc_config(f);
}

bool ontology_configuration::apply_cpt_config(mclFrame* f) {
  return cpt_cfg::apply_cpt_config(cpt_configuration,f);
}

string ontology_configuration::relax_config_path(string &relax_path) {
  size_t lastslash = relax_path.find_last_of("/\\");
  if (lastslash == string::npos)
    return relax_path;
  else
    return relax_path.substr(0,lastslash);
}

bool ontology_configuration::ensure_file_readable(string filename) {
  fstream fin;
  cout << "testing for file <" << filename << ">...";
  fin.open(filename.c_str(),ios::in);
  if( fin.is_open() ) {
    fin.close();
    cout << "yes" << endl;
    return true;
  }
    cout << "no" << endl;
  return false;
}

bool ontology_configuration::ensure_read_path(string &path) {
  struct stat stat_info;
  // 1. Check that the path exists -- false if not
  if (!stat(path.c_str(), &stat_info))
    return false; 
  // 2. Check that it is a directory -- false if not
  if (!stat_info.st_mode && S_IFDIR)
    return false;
  return true;
}

bool ontology_configuration::ensure_write_path(string &path) {
  cout << "ensuring " << path << " is writable...";
  struct stat stat_info;
  // 1. Check that the path exists -- false if not
  if (stat(path.c_str(), &stat_info) < 0) {
    cout << "no (stat)" << endl;
    return false;
  }
  // 2. Check that it is a directory -- false if not
  if (!stat_info.st_mode && S_IFDIR) {
    cout << "no (S_IFDIR)" << endl;
    return false;
  }
  // 3. Check that it is writable -- false if not
  string test_fn = path+"/"+"verify_write";
  ofstream write (test_fn.c_str()); //writing to a file
  if (write.is_open()) {
    cout << "yes" << endl;
    write.close();
    return true;
  }
  else {
    cout << "no (write)" << endl;
    return false;
  }
}

bool ontology_configuration::make_path(string &path) {
  return true;
}
