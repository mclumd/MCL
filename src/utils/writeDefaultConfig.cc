#include "mcl.h"
#include "mclFrame.h"
#include "mclConstants.h"
#include "mcl_multiagent_api.h"
#include "mcl_ma_internal_api.h"
#include "../../include/umbc/settings.h"

// #include "ontology_configurator.h"
// #include "mclOntology.h"
// #include "oNodes.h"
// #include "linkFactory.h"
// #include "configManager.h"

#include <iostream>
#include <list>

using namespace std;
using namespace metacog;

void _mcl_config_usage(char** argv);

int main(int argc, char **argv) {
  umbc::settings::setSysProperty("suppressConfigErrors",true);
  if (argc < 4) {
    _mcl_config_usage(argv);
  }
  else {
    try {
      // this initializes MCL and loads the base configuration
      string mod = argv[1];
      string ont = argv[2];
      string dom = argv[3];
      mclMA::initializeMCL("test",0);
      // mclMA::chooseOntology("test",ont);
      if (argc > 4) {
	string agnt = argv[4];
	if (argc > 5) {
	  string cont = argv[5];
	  mclMA::configureMCL("test",ont,dom,agnt,cont);
	}
	else 
	  mclMA::configureMCL("test",ont,dom,agnt);
      }
      else
	mclMA::configureMCL("test",ont,dom);
      
      // now perform the function...
      if (mod == "cpt") {
	cout << "Starting to configure the CPTs." << endl;
	mclFrameEntryVector fev(ENTRY_NEW);
	mclFrame    *f = new mclFrame(mclMAint::getMCLFor("test"),fev);
	if (cpt_cfg::save_cpts_to_global(f)) {
	  cout << "configuration written to " 
	       << "'" << f->MCL()->getConfigKey() << "'"
	       << endl;
	  return 0;
	}
	else {
	  cout << "a problem occurred during config." << endl;
	  return 1;
	}
      }
      else if (mod == "costs") {
	cout << "Starting to configure costs." << endl;
	mclFrameEntryVector fev(ENTRY_NEW);
	mclFrame    *f = new mclFrame(mclMAint::getMCLFor("test"),fev);
      }
    }
    catch (MCLException e) {
      cout << "ERROR: "+(string)e.what() << endl;
      return -1;
    }
  }
  return 0;
}

void _mcl_config_usage(char** argv) {
  cout << "usage: " << argv[0] << " <module> <ontologybase> [domain] [agent] [controller]" << endl;
  cout << " <module> = config component (valid: cpt, cost)" << endl;
  cout << " <ontologybase> = basename of ontology in config/netdefs (ex: basic)" << endl;
  cout << " [domain] = domain directory to write to" << endl;
  cout << " [agent]  = agent directory to write to" << endl;
  cout << " [controller] = controller directory to write to" << endl;
  exit(-1);
}

      // cfg::setSystem(sys);
      // cfg::loadCPTs();
      // cfg::ignore_cfg_errors();
      // cfg::applyCPTconfig(f);
      // cfg::saveCPTs(f);
      // cfg::saveResponseCosts(f);      
