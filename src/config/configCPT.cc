#include <iostream>
#include <fstream>
#include <algorithm>

#ifdef WIN32
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "ontology_config.h"
#include "mcl.h"
#include "Exceptions.h"
#include "mclLogging.h"
#include "../../include/umbc/settings.h"
#include "../../include/umbc/exceptions.h"
#include "../../include/umbc/text_utils.h"
#include "../../include/umbc/token_machine.h"
#include "../../include/umbc/file_utils.h"

using namespace std;
using namespace metacog;
using namespace umbc;

extern char mcl_perror[2048];
// map<string,string> CPTmap;

bool ignore_errors = false;

bool checkConsistent(mclNode *n,vector<string>& deps);
bool checkSorted(mclNode *n,vector<string>& deps);
bool resortAndInitCPT(mclNode *n,
		      vector<string>& deps,
		      vector<double>& cpv);
bool ensure_path(string path);


bool cpt_cfg::ignore_cfg_errors() {
  return (ignore_errors=true);
}

bool cpt_cfg::load_cpts(cpt_configuration_type& dest,string conf_fname) {
  // cout << "attempting to load cpt config from '" << pth << "'" << endl;
  dest.clear();
  
  uLog::annotate(MCLA_MSG,"loading conditional probability priors from " +
		      conf_fname + "...");
  string conf_s = textFunctions::file2string(conf_fname);
  bool running=true;
  tokenMachine level1(conf_s);
  while (running) {
    string table_s = level1.nextToken();
    // cout << "::> '" << table_s << "'" << endl;
    if (table_s.size() == 0) 
      running=false;
    else {
      tokenMachine level2(table_s);
      level2.trimParens();
      string node_s = textFunctions::dequote(level2.nextToken());
      dest[node_s]=table_s;
    }
  }
  sprintf(uLog::annotateBuffer,"%ld tables loaded.",(unsigned long)dest.size());
  uLog::annotateFromBuffer(MCLA_MSG);
  return false;
}

string cfg_globalize(string path) {
  char* foo = getenv("MCL_CONFIG_PATH");
  if (foo) {
    string pathToMCLCore = foo;
    return pathToMCLCore+"/config/"+path;
  }
  else {
    uLog::annotate(MCLA_WARNING,
			"could not get MCL_CONFIG_PATH to globalize '"+path+"', defaulting to CWD for writing config.");
    return path;
  }
}

string cfg_homize(string path) {
  string pathToHome = getenv("HOME");
  if (pathToHome != "") return pathToHome+"/.mcl_config/"+path;
  return path;
}

bool cpt_cfg::save_cpts_to_global(mclFrame *f) {
  // cout << "save cpts to global (autopath)." << endl;
  return save_cpts_to_global(f->MCL()->getConfigKey(),f);
}

bool cpt_cfg::save_cpts_to_cwd(mclFrame *f) {
  return save_cpts(f);
}

bool cpt_cfg::save_cpts_to_home(mclFrame *f) {
  // cout << "save cpts to home (autopath)." << endl;
  return save_cpts_to_home(f->MCL()->getConfigKey(),f);
}

bool cpt_cfg::save_cpts_to_global(string path, mclFrame *f) {
  cout << "save cpts to global (" << path << ")." << endl;
  cout << "path is (" << cfg_globalize(path) << ")." << endl;
  return save_cpts(cfg_globalize(path),f);
}

bool cpt_cfg::save_cpts_to_cwd(string path, mclFrame *f) {
  return save_cpts(path,f);
}

bool cpt_cfg::save_cpts_to_home(string path, mclFrame *f) {
  return save_cpts(cfg_homize(path),f);
}

bool cpt_cfg::save_cpts(mclFrame *f) {
  string pithy_path = f->MCL()->getConfigKey();
  // cout << "save path would be '" << pithy_path << "'" << endl;
  return save_cpts(pithy_path,f);
}

bool cpt_cfg::save_cpts(string path, mclFrame *f) {
  if (!ensure_path(path)) {
    cerr << "could not ensure a writeable path at '"
	 << path << "'... aborting write." << endl;
    return false;
  }
  // string oBase = f->getIndicationCore()->getOntologySourceName();
  string oBase = f->MCL()->getOntologyBase();
  string is = path+"/"+oBase+"_cpt.mcl";
  if (umbc::file_utils::backup_if_exists(is)) {
    uLog::annotate(MCLA_MSG,"existing file '"+is+"' has been backed up.");
  }

  uLog::annotate(MCLA_MSG,"writing CPT config file to " + is);
  ofstream no(is.c_str(),ios_base::out);
  if (!no.good()) {
    uLog::annotate(MCLA_ERROR, is + " is not good.");
    return false;
  }

  for (int indx=0; indx<f->numOntologies(); indx++) {
    // cout << "writing ontology " << indx << endl;
    mclOntology *this_o = f->getCoreOntology(indx);
    for (nodeList::iterator oni = this_o->firstNode();
	 oni != this_o->endNode();
	 oni++) {
      // cout << "writing node " << (*oni)->entityBaseName() << endl;
      no << "(\"" << (*oni)->entityBaseName() << "\" "; 
      // deps
      if ((*oni)->inLinkCnt() > 0) {
	no << ":dep (";
	for (llIterator ili = (*oni)->inLink_begin();
	     ili != (*oni)->inLink_end();
	     ili++) {
	  no << "\"" << (*ili)->sourceNode()->entityBaseName() << "\" ";
	}
	no << ") ";
      }
      // cpt
      {
	int cpt_size = (int)(pow(2,(*oni)->inLinkCnt()));
	double* CPTaa = new double[cpt_size];
	(*oni)->writeCPTtoArray(CPTaa);
	no << ":cpv (";
	for (int c = 0;
	     c < cpt_size;
	     c++) {
	  no << CPTaa[c] << " ";
	}
	no << ")";
	no << ")" << endl;
	delete[] CPTaa;
      }
    }
  }
  return true;
}


bool ensure_path(string path) {
#ifdef WIN32
  return (_mkdir(local_system_config_path.c_str())==0);
#else      
  // check the whole path....
  DIR* d = opendir(path.c_str());
  if (d != NULL) {
    closedir(d);
    return true;
  }
  else {
    // okay, now unwind using a tokenMachine
    string base = textFunctions::substChar(path,'/',' ');
    // cout << "(" << base << ")" << endl;
    tokenMachine tm(base);
    string ccd = "";
    if (path.at(0) == '/')
      ccd+="/";
    while (tm.moreTokens()) {
      ccd+=tm.nextToken()+"/";
      DIR* d = opendir(ccd.c_str());
      if (d != NULL) {
	closedir(d);
      }
      else {
	uLog::annotate(MCLA_MSG,"[mcl/cfgcpt]:: creating "+ccd);
	// mode_t omask = 	umask();
	int rv = mkdir(ccd.c_str(),0777);
	if (rv != 0) {
	  perror("error in mkdir");
	  return false;
	}
      }
    }
    return true;
  }
#endif  
}

bool cpt_cfg::apply_cpt_config(cpt_configuration_type& dest,mclFrame *f) {
  bool rv = true;
  rv &= cpt_cfg::apply_cpt_config(dest, f->getIndicationCore());
  rv &= cpt_cfg::apply_cpt_config(dest, f->getFailureCore());
  rv &= cpt_cfg::apply_cpt_config(dest, f->getResponseCore());
  return rv;
}

bool cpt_cfg::apply_cpt_config(cpt_configuration_type& dest,mclOntology *o) {
  bool rv = true;
  for (nodeList::iterator nli = o->firstNode();
       nli != o->endNode();
       nli++) {
    try {
      rv &= cpt_cfg::apply_cpt_config((*nli),dest[(*nli)->entityBaseName()]);
    }
    catch (MissingNodeConfiguration& mnc) {
      // okay, there could be a more sophisticated thing to do here.
      // but we don't want to rethrow if we are going to soldier on, so...
      uLog::annotate(MCLA_WARNING,mnc.what());
    }
    catch (exception& e) {
      uLog::annotate(MCLA_WARNING,"untrapped exception in apply_cpt_config");
    }
  }
  return rv;
}

bool cpt_cfg::apply_cpt_config(mclNode *n,string config) {

  if (config.size()==0) {
    throw MissingNodeConfiguration(n->entityBaseName());
    // return true;
  }

  // parse config string.
  tokenMachine ctm(config);
  ctm.trimParens();
  ctm.nextToken();
  vector<string> deps;
  vector<double> cpv;
  bool running=true;
  while (running) {
    string k = ctm.nextToken();
    string v = ctm.nextToken();
    if ((k.size() == 0) || (v.size() == 0))
      running=false;
    else {
      // :dep keyword specifies incoming links to the current node
      if (k == ":dep") {
	deps=tokenMachine::processAsStringVector(v);
	if (!checkConsistent(n,deps)) {
	  if (umbc::settings::getSysPropertyBool("suppressConfigErrors",false)) {
	    uLog::annotate(MCLA_WARNING, 
				n->entityName()+":  inconsistent dep vector (using default priors).");
	    return false;
	  }
	  else { 
	    sprintf(mcl_perror,"while applying CPT config: dep vector %s is inconsistent with node %s's incoming link list",v.c_str(),n->entityName().c_str());
	    umbc::exceptions::signal_exception(mcl_perror);
	  }
	}
      }
      // :cpv keyword specifies conditional probability _vector_
      else if (k == ":cpv") {
	cpv=tokenMachine::processAsDoubleVector(v);
      }
      // those are the only allowable keywords
      else {
	if (ignore_errors) {
	  uLog::annotate(MCLA_WARNING, "skipping " + n->entityName() +
			      ": unhandled keyword.");
	}
	else {
	  sprintf(mcl_perror,"while applying CPT config: unhandled keyword '%s'",k.c_str());
	  exceptions::signal_exception(mcl_perror);
	}
      }
    }
  }
  // 
  if (checkSorted(n,deps)) {
    // cout << "sort is kosher for CPT reconstruction." << endl;
    n->initPriors(deps,cpv);
  }
  else {
    // cout << "sort is not kosher for CPT reconstruction (resort)." << endl;
    resortAndInitCPT(n,deps,cpv);
  }
  return true;
}

//! checks to make sure the deps vector is consistent with incoming link
//! list of "n"
bool checkConsistent(mclNode *n,vector<string>& deps) {
  if ((unsigned int)n->inLinkCnt() == deps.size()) {
    for (llIterator nli = n->inLink_begin();
	 nli != n->inLink_end();
	 nli++) {
      vector<string>::iterator fp = find(deps.begin(),deps.end(),
					 (*nli)->sourceNode()->entityBaseName());
      if (fp == deps.end())
	return false;
    }
    /* okay, this is not really admissible in the case where there are 
       duplicates in either the source or target, but that should never 
       happen, right? */
    return true;
  }
  else
    return false;
}

//! checks to ensure that the deps vector is sorted the same way as
//! the incoming link list of "n"
bool checkSorted(mclNode *n,vector<string>& deps) {
  if ((unsigned int)n->inLinkCnt() != deps.size()) {
    sprintf(uLog::annotateBuffer,
	    "link count mismatch in checkSorted: %s linkcount= %d  depsize=%ld",
	    n->entityName().c_str(), n->inLinkCnt() ,
	    (unsigned long)deps.size());
    uLog::annotateFromBuffer(MCLA_WARNING);
    return false;
  }
  else {
    llIterator nli = n->inLink_begin();
    vector<string>::iterator dvi = deps.begin();
    while ((nli != n->inLink_end()) &&
	   (dvi != deps.end())) {
      if ((*nli)->sourceNode()->entityBaseName() != (*dvi)) {
	return false;
      }
      nli++;
      dvi++;
    }
  }
  return true;
}

//! if this worked it would autosort the deps and cp vectors to match
//! "n"'s incomcing link list so everything would get stored in the right
//! place
bool resortAndInitCPT(mclNode *n,
		      vector<string>& deps,
		      vector<double>& cpv) {
  if (ignore_errors) {
    umbc::uLog::annotate(MCLA_ERROR,"resort/init failed because resorting is not implemented, "+n->entityName()+" config failed...");
    return false;
  }
  else {
    sprintf(mcl_perror,"Dependency list for %s is improperly ordered in CPT file. resortAndInitCPT() is unimplemented.",n->entityName().c_str());
    umbc::exceptions::signal_exception(mcl_perror);
  }
  return false;  // not reached but will quiet -Wall
}
