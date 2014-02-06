#include "configManager.h"
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

string system_dir;

string local_system_config_path;
string core_system_config_path;
string local_default_config_path;
string core_default_config_path;
string write_config_path;

void cfg::setSystem(string system) {
  system_dir=system;
  find_config_read_write();
}

string cfg::getSystemName() {
  return system_dir;
}

void cfg::loadConfig(string system) {
  cout << "Restoring configuration for '" << system << "'" << endl;
  setSystem(system);
  loadCPTs();
  loadResponseCosts();
}

void cfg::saveConfig() {
  // requires a frame...
  // saveCPTs(system);
}

void cfg::find_config_read_write() {
  
  char *pathToHome = getenv("HOME");
  char *pathToMCLCore = getenv("MCL_CONFIG_PATH");

  if (pathToMCLCore != NULL) {
    cout << "using MCL_CONFIG binding for config path..." << endl;
    core_system_config_path =pathToMCLCore;
    core_system_config_path+="/config/"+system_dir+"/";
    core_default_config_path =pathToMCLCore;
    core_default_config_path+="/config/default/";
    return;
  }
  else {
    cerr << "MCL_CONFIG_PATH not set. Only local configurations can be loaded."
	 << endl;
  }

  if (pathToHome != NULL) {
    local_default_config_path=pathToHome;
    local_default_config_path+="/.mcl-config/default/";
    local_system_config_path=pathToHome;
    local_system_config_path+="/.mcl-config/"+system_dir+"/";
  }
  else {
    cerr << "HOME not set. Local configurations cannot be loaded." << endl;
  }
 
}

inline bool cf_exists(string fp) {
  fstream fin;
  cout << "testing for file <" << fp << ">...";
  fin.open(fp.c_str(),ios::in);
  if( fin.is_open() ) {
    fin.close();
    cout << "yes" << endl;
    return true;
  }
    cout << "no" << endl;
  return false;
}

string cfg::pathToConfigRead(string file) {
  // order of looking:
  // #1: local system
  if (cf_exists(local_system_config_path+file))
    return (local_system_config_path+file);
  // #2: core  system
  if (cf_exists(core_system_config_path+file))
    return (core_system_config_path+file);
  // #3: local defaults
  if (cf_exists(local_default_config_path+file))
    return (local_default_config_path+file);
  // #4: core  defaults
  if (cf_exists(core_default_config_path+file))
    return (core_default_config_path+file);
  return file;
}

bool try_ensuredir(string path) {
#ifdef WIN32
  return (_mkdir(local_system_config_path.c_str())==0);
#else      
  DIR* d = opendir(path.c_str());
  if (d != NULL) {
    closedir(d);
    return true;
  }
  else {
    cout << "could not open " << path << ", trying to create." << endl;
    int rv = mkdir(path.c_str(),777);
    if (rv != 0) {
      cout << "could not create " << path << endl;
      perror("mkdir");
    }
    return (rv==0);
  }
#endif
}

bool ensure_writeable(string path) {
  string fp = path+"/test.mcl";
  fstream fin;
  // cout << "testing for file <" << fp << ">..." << endl;
  fin.open(fp.c_str(),ios::out);
  if( fin.is_open() ) {
    fin.close();
    return true;
  }
  else
    return false;
}

string cfg::pathToConfigWrite(string file) {
  if (try_ensuredir(core_system_config_path) &&
      ensure_writeable(core_system_config_path))
    return core_system_config_path+file;
  else if (try_ensuredir(local_system_config_path) &&
	   ensure_writeable(local_system_config_path))
    return local_system_config_path+file;
  else {
    cerr << "Warning: local system configuration path ("
	 << local_system_config_path
	 << ") not found, writing configuration to CWD" << endl;
    return file;
  }
}
