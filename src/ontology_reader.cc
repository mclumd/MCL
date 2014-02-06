#include "../include/umbc/text_utils.h"
#include "../include/umbc/file_utils.h"
#include "../include/umbc/token_machine.h"
#include "../include/umbc/settings.h"
#include "../include/umbc/exceptions.h"
#include "../include/umbc/file_utils.h"
#include "ontology_reader.h"
#include "linkFactory.h"
#include "mclLogging.h"
#include "mcl_api.h"

using namespace metacog;
using namespace umbc;

bool __build_ontology_from_string(mclOntology* o,string def);
bool __build_linkage_from_string(ontologymap_t& oif,string ldef);
bool __build_node_from_spec(mclOntology* o,string nodespec);
bool __make_link_from_spec(ontologymap_t& o,string linkspec);
mclOntology* __find_name_in_omap(ontologymap_t& oif, string name);
void __or_error(string msg);

bool ontology_reader::legit_ontologyname(string basename) {
  string o_fn = basename2filename(basename);
  return file_utils::file_exists(o_fn);
}

ontologyVector* ontology_reader::read_ontologies(string basename,mclFrame* f) {
  uLog::annotate(MCLA_MSG,"attempting to read ontologies for "+basename);
  ontologymap_t mymap;
  bool doNotFail = settings::getSysPropertyBool("mcl.doNotFail",false);
  string o_fn = basename2filename(basename);
  if (!read_ontologies_from_file(o_fn.c_str(),mymap)) {
    if (doNotFail)	{
      // try to soldier on...
      uLog::annotate(MCLA_ERROR,"failed to load ontology with ontology_reader... defaulting to buildCore methods.");
      uLog::annotate(MCLA_ERROR,"make sure MCL_CONFIG_PATH or mcl.configpath is set correctly.");
      return mclGenerateOntologies(f);
    }
    else {
      exceptions::signal_exception("failure to locate "+basename+" ontologies in '"+o_fn+"' (check that MCL_CONFIG_PATH or mcl.configpath is set correctly).");
    }
  }	
  else uLog::annotate(MCLA_MSG,basename+" ontologies loaded from "+o_fn);
  // okay, so this is ugly because the ontology reader is new and the
  // mclFrame is old...
  //
  // a little error checking
  if (mymap.find("indications") == mymap.end())
    umbc::exceptions::signal_exception("missing indication ontology!");
  if (mymap.find("failures") == mymap.end())
    umbc::exceptions::signal_exception("missing failure ontology!");
  if (mymap.find("responses") == mymap.end())
    umbc::exceptions::signal_exception("missing response ontology!");
  // create an ontologyvector
  ontologyVector* ov = new ontologyVector();
  // hardcode the 3 major ontologies into their hardcoded locations
  ov->push_back(mymap["indications"]);
  ov->push_back(mymap["failures"]);
  ov->push_back(mymap["responses"]);
  ((*ov)[INDICATION_INDEX])->autoActivateProperties(*(f->getPV()));
  // now tack any other stragglers onto the end...
  for (ontologymap_t::iterator i = mymap.begin();
       i != mymap.end();
       i++) {
    // we should be able to just set the frame here without ill effects
    i->second->setFrame(f);
    if ((i->first != "indications") &&
	(i->first != "failures") &&
	(i->first != "responses"))
      ov->push_back(i->second);
  }
  return ov;
  
}

bool ontology_reader::read_ontologies_from_file(const char* f_name,
						ontologymap_t& oif) {
  if (!file_utils::file_exists(f_name)) {
    return false;
  }
  string file_as_string = textFunctions::file2string(f_name,"#",true);
  string baseOntology = umbc::file_utils::fileNameRoot((string)f_name);
  cout << f_name << ": " << file_as_string.size() << " bytes." << endl;
  tokenMachine tltm(file_as_string);
  while (tltm.moreTokens()) {
    string tltok1 = tltm.nextToken();
    cout << "processing " << f_name << ": '" << tltok1 << "'" << endl;
    if (tltok1 == "ontology") {
      string o_name = tltm.nextToken();
      string odef   = tltm.nextToken();
      if (oif.find(o_name) == oif.end())
	oif[o_name]  = new mclOntology(o_name,baseOntology,NULL);
      __build_ontology_from_string(oif[o_name],odef);      
    }
    else if (tltok1 == "linkage") {
      string l_name = tltm.nextToken();
      string ldef   = tltm.nextToken();
      __build_linkage_from_string(oif,ldef);
    }
    else if (tltok1 == "include") {
      string if_name = tltm.nextToken();
      if (if_name[0] == '/')
	read_ontologies_from_file(if_name.c_str(),oif);
      else {
	string if_real = basename2filename(if_name);
	read_ontologies_from_file(if_real.c_str(),oif);
      }
    }
    else {
      __or_error("unknown ontology component '"+tltok1+"' found in '"+(string)f_name);
    }
  }
  return true;
}

string ontology_reader::basename2filename(string basename) {
  string fn = "";
  try {
    fn = file_utils::inEnvDirectory("MCL_CONFIG_PATH","netdefs/"+basename+".ont",NULL);
  }
  catch (file_utils::unsetEnvVariableException e) {
    uLog::annotate(MCLA_ERROR,e.what());
    fn = settings::getSysPropertyString("mcl.configpath",".")+"/netdefs/"+basename+".ont";
  }
  return fn;
}

bool __build_ontology_from_string(mclOntology* o,string def) {
  tokenMachine oltm(def);
  oltm.trimParens();
  while (oltm.moreTokens()) {
    string cspec = oltm.nextToken();
    if (cspec == "node") {
      string nodespec = oltm.nextToken();
      __build_node_from_spec(o,nodespec);
    }
    else {
      __or_error("unknown ontology component '"+cspec+"' encountered.");
    }
  }
  return true;
}

bool __build_linkage_from_string(ontologymap_t& oif,string ldef) {
  tokenMachine oltm(ldef);
  oltm.trimParens();
  while (oltm.moreTokens()) {
    string cspec = oltm.nextToken();
    if (cspec == "link") {
      string linkspec = oltm.nextToken();
      __make_link_from_spec(oif,linkspec);
    }
    else {
      __or_error("unknown linkage component '"+cspec+"' encountered.");
    }
  }
  return true;
}

bool __build_node_from_spec(mclOntology* o,string nodespec) {
  // cerr << "nodespec=<" << nodespec << ">" << endl;
  string ntype = textFunctions::getFunctor(nodespec);
  string npara = textFunctions::getParameters(nodespec);
  keywordParamMachine pm(npara);
  string nname = pm.getKWV("name");
  string ndoc  = pm.getKWV("doc");
  // cout << "adding node \"" << nname << "\" type=" << ntype << endl;
  if (ntype == "hostProp") {
    string nprops = pm.getKWV("prop");
    bool   err    = false;
    pkType nprop = symbols::smartval_int(nprops,&err);
    mclHostProperty* nn = new mclHostProperty(nname,o,nprop);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "HII") {
    mclHostInitiatedIndication* nn = new mclHostInitiatedIndication(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "genInd") {
    mclGeneralIndication* nn = new mclGeneralIndication(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "concInd") {
    mclConcreteIndication* nn = new mclConcreteIndication(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "iCore") {
    mclIndicationCoreNode* nn = new mclIndicationCoreNode(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "failure") {
    mclFailure* nn = new mclFailure(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "genResponse") {
    mclGeneralResponse* nn = new mclGeneralResponse(nname,o);
    nn->document(ndoc);
    o->addNode(nn);
  }
  else if (ntype == "concResponse") {
    string rcodes = pm.getKWV("code");
    bool   err    = false;
    pkType rcodei = symbols::smartval_int(rcodes,&err);
    sprintf(uLog::annotateBuffer,
	    "reading concrete response(N=%s,%d NN=%s)",
	    rcodes.c_str(),rcodei,nname.c_str());
    umbc::uLog::annotateFromBuffer(MCLA_ERROR);
    if (err) {
      __or_error("using extended response code for "+rcodes);
      rcodei = CRC_EXTENDED_CODE;
    }
    mclConcreteResponse* nn = new mclConcreteResponse(nname,o,rcodei);
    if (rcodei == CRC_EXTENDED_CODE) nn->setRespCodeExtended(rcodes);
    nn->document(ndoc);
    string rcosts = pm.getKWV("cost");
    if (rcosts != "") {
      pkType rcostd = textFunctions::dubval(rcosts);
      nn->setCost(rcostd);
    }
    o->addNode(nn);
  }
  else if (ntype == "interactive") {
    bool   err    = false;
    string rcodes = pm.getKWV("code");
    string onces = pm.getKWV("runOnce");
    string yess  = pm.getKWV("yes");
    string nos   = pm.getKWV("no");
    pkType rcodei = symbols::smartval_int(rcodes,&err);
    if (err) {
      __or_error("failed to lookup response code "+rcodes);
      rcodei = CRC_EXTENDED_CODE;
    }
    mcl_AD_InteractiveResponse* nn = 
      new mcl_AD_InteractiveResponse(nname,o,rcodei);
    if (rcodei == CRC_EXTENDED_CODE) nn->setRespCodeExtended(rcodes);
    nn->addPositive(yess);
    nn->addNegative(nos);
    if (textFunctions::boolval(onces)) 
      nn->onlyRunOnce();
    nn->document(ndoc);
    string rcosts = pm.getKWV("cost");
    if (rcosts != "") {
      pkType rcostd = textFunctions::dubval(rcosts);
      nn->setCost(rcostd);
    }
    o->addNode(nn);
  }
  else {
    __or_error("unknown node type '"+ntype+"' encountered.");
  }
  return true;
}

bool __make_link_from_spec(ontologymap_t& oif, string linkspec) {
  string ltype = textFunctions::getFunctor(linkspec);
  string lpara = textFunctions::getParameters(linkspec);
  keywordParamMachine pm(lpara);
  string src = pm.getKWV("src");
  mclOntology* srco = __find_name_in_omap(oif,src);
  string dst  = pm.getKWV("dst");
  mclOntology* dsto = __find_name_in_omap(oif,dst);
  // cout << "adding link \"" << src << "\" => \"" << dst << "\" type=" << ltype << endl;
  if (!srco || !dsto) {
    string me="";
    if (!srco) me+=src + " ";
    if (!dsto) me+=dst + " ";
    __or_error("could not find node(s): "+me);
    return false;
  }
  else if (ltype == "abstraction") {
    linkFactory::makeAbstractionLink(srco,src,dst);
  }
  else if (ltype == "IFC") {
    linkFactory::makeIFCLink(srco,src,dst);
  }
  else if (ltype == "specification") {
    linkFactory::makeSpecificationLink(srco,src,dst);
  }
  else if (ltype == "diagnostic") {
    linkFactory::makeDiagnosticLink(srco,src,dsto,dst);
  }
  else if (ltype == "inhibitory") {
    linkFactory::makeInhibitoryLink(srco,src,dsto,dst);
  }
  else if (ltype == "support") {
    linkFactory::makeSupportLink(srco,src,dsto,dst);
  }
  else if (ltype == "prescriptive") {
    linkFactory::makePrescriptiveLink(srco,src,dsto,dst);
  }
  else {
    __or_error("unknown link type '"+ltype+"' encountered.");
  }
  return true;
}

mclOntology* __find_name_in_omap(ontologymap_t& oif, string name) {
  for (ontologymap_t::iterator i = oif.begin(); i != oif.end(); i++) {
    if (i->second->findNamedNode(name))
      return i->second;
  }
  return NULL;
}

void __or_error(string msg) {
  if (settings::getSysPropertyBool("mcl.or.trapAsExceptions")) {
    exceptions::signal_exception(msg);
  }
  else {
    uLog::annotate(MCLA_ERROR," ! "+msg);
  }
}
