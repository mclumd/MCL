#ifndef ONTOLOGY_CONFIG_H
#define ONTOLOGY_CONFIG_H

#include "mclFrame.h"

namespace metacog {

  typedef map<string,string> cpt_configuration_type;

  class ontology_configuration {
  public:
    ontology_configuration(string ontology,string domain);
    ontology_configuration(string ontology,string domain, string agent);
    ontology_configuration(string ontology,string domain, string agent, 
			   string controller);
    string configuration_id() { return conf_id; };
    
    bool apply_rc_config(mclFrame* f);
    bool apply_cpt_config(mclFrame* f);
    bool apply_config(mclFrame* f) {
      bool rv = true;
      rv &= apply_rc_config(f);
      rv &= apply_cpt_config(f);
      return rv;    
    }

  protected:
    static string cptFile(string pth,string o_name);
    
  private:
    string conf_id;
    string onto_id;
    string source_path;
    string write_path;
    int    specificity; // the number of components to the config path
    
    cpt_configuration_type cpt_configuration;
    
    // path recovery/construction
    bool load_initial_config();
    string determine_paths();
    string determine_write_path();
    string  determine_read_path();
    bool construct_write_path();
    bool write_config();
    string relax_config_path(string &relax_path);
    bool ensure_file_readable(string filename);
    bool ensure_read_path(string &path);
    bool ensure_write_path(string &path);
    bool make_path(string &path);
    
    // conditional prob tables
    bool load_cpts();
    bool save_cpts(mclFrame* f);
    
    // response costs
    bool load_response_costs();
    bool save_response_costs();
    
  };

  namespace ontology_configurator {
    // returns a cKey
    string require_config(string ontology,string domain);
    string require_config(string ontology,string domain, string agent);
    string require_config(string ontology,string domain, string agent, string controller);
    
    string cKey2path(string ck);
    string configPath(mclEntity* m,string fn);
    
    string cKey_for(string domain);
    string cKey_for(string domain, string agent);
    string cKey_for(string domain, string agent, string controller);
    
    bool apply_config(mclFrame* f);
    
  };

  namespace cpt_cfg {
    bool load_cpts(cpt_configuration_type& dest,string path);
    bool save_cpts(mclFrame *f);
    bool save_cpts(string path,mclFrame *f);
    
    bool save_cpts_to_global(mclFrame *f);
    bool save_cpts_to_cwd(mclFrame *f);
    bool save_cpts_to_home(mclFrame *f);
    
    bool save_cpts_to_global(string path, mclFrame *f);
    bool save_cpts_to_cwd(string path, mclFrame *f);
    bool save_cpts_to_home(string path, mclFrame *f);
    
    bool apply_cpt_config(cpt_configuration_type& dest,mclFrame *f);
    bool apply_cpt_config(cpt_configuration_type& dest,mclOntology *o);
    bool apply_cpt_config(mclNode *n,string config);
    
    bool ignore_cfg_errors();
    
  };

  namespace rc_cfg {
    bool load_response_costs(string path);
    bool save_response_costs(string path,mclFrame *f);
    bool apply_rc_config(mclFrame *f);
    bool apply_rc_config(mclOntology *o);
    bool apply_rc_config(mclNode *n,string config);
  };

};

#endif
