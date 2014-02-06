#include "smileGlue.h"
#include "mclFrame.h"
#include "hgGlue.h"
#include "topSort.h"
#include "mclLogging.h"
#include "Exceptions.h"

using namespace metacog;

// SMILE GLUE IMPLEMENTATION FOR SMILE-BASED INFERENCE IN MCL

////////////////////////////////////////////////////////////////
/// SMILE GLUE

string smileGlue::smilizedName(string name) {
  string rv = name;
  std::replace( rv.begin(), rv.end(), '-', '_' );  
  return rv;
}

#define SMILE_TRUENAME  "y"
#define SMILE_FALSENAME "n"

#define SMILE_TRUEINDEX  1
#define SMILE_FALSEINDEX 0

int smileGlue::smileIndex2configIndex(int smileI,
				  vector<string>& depsAsCreated,
				  vector<string>& hgD) {
  int rv = 0;
  // loop over bits in the smileI
  for (size_t smI=0; smI < depsAsCreated.size(); smI++) {
    // if the bit is set in the smileI, then we need to add to the rv
    if ((smileI & (1 << smI)) != 0) {
      // compute where depAsCreated[smI] is in hgD
      int q2hg=-1;
      for (size_t hgI=0; hgI < hgD.size(); hgI++) {
	if (depsAsCreated[smI] == hgD[hgI])
	  { q2hg = hgI; break; }
      }
      // set the bit in the return vector accordingly
      if (q2hg == -1) {
	throw OntologyException("error mapping smile index to config index: usually indicates smile dependency could not be found in home-grown network dependency list: "+depsAsCreated[smI]);
      }
      
      // this bit-mash is for straight reordering
      /*  rv |= (1 << q2hg); */
      
      // this bit-mash is for reordering when the config indexing scheme
      // is reversed from the smile indexing scheme (which it is)
      rv |= (1 << (depsAsCreated.size() - q2hg -1));
    }
  }
  return rv;
}

////////////////////////////////////////////////////////////////
/// SMILE NODE GLUE

smileNodeGlue::~smileNodeGlue() {}

smileNodeGlue::smileNodeGlue(string name,mclNode* node) :
  nodeGlue(name,node),myFrameGlue(NULL) { 
  if (getFrameGlue() == NULL) {
    throw GlueException("smile frame glue is NULL [cannot create smileNodeGlue before smileFrameGlue has been attached to mclFrame]");
  }
  DSL_network* theNet = getFrameGlue()->getNet();
  string thisNodesSMILEname = smileGlue::smilizedName(node->entityBaseName());
  smileID = theNet->AddNode(DSL_CPT,(char*)thisNodesSMILEname.c_str());
  cout << "[SMILEG]: creating nodeglue in " << theNet
       << " for " << thisNodesSMILEname.c_str() << " id=" 
       << smileID << endl;
  cout << "[SMILEG]: immediate searchback (" << thisNodesSMILEname 
       << ") returns " << theNet->FindNode(thisNodesSMILEname.c_str())
       << endl;

  if (smileID < 0) {
    if (smileID != theNet->FindNode(thisNodesSMILEname.c_str()))
      throw OntologyConstructionException("SMILE::AddNode failed but the named node exists... is the node '"+thisNodesSMILEname+"' multiply defined?");
    else 
      throw OntologyConstructionException("SMILE::AddNode failed for unknown reason.");
  }
  else if (theNet->FindNode(thisNodesSMILEname.c_str()) < 0) {
      umbc::uLog::annotate(MCLA_WARNING,"[SMILEG]: Successfully added a SMILE node but was unable to find it using FindNode ... consequences are unknown.");
  }

  DSL_idArray someNames;
  someNames.Add(SMILE_FALSENAME);
  someNames.Add(SMILE_TRUENAME);
  theNet->GetNode(smileID)->Definition()->SetNumberOfOutcomes(someNames);
  // because we are topologically sorting, we should be able to 
  // create arcs here
  for (llIterator ili = node->inLink_begin();
       ili != node->inLink_end();
       ili++) {
    int src_n = theNet->FindNode((char*)smileGlue::smilizedName((*ili)->sourceNode()->entityBaseName()).c_str());
    /*
    cout << "[SMILEG]: adding arc " << (*ili)->sourceNode()->entityBaseName()
	 << "(" << src_n << ") ~> " << node->entityBaseName() << "("
	 << smileID << ")" << endl;
    */
    depsAsCreated.push_back((*ili)->sourceNode()->entityBaseName());
    theNet->AddArc(src_n,smileID);
  }

  // now create uniform priors
  DSL_sysCoordinates coo(*(theNet->GetNode(smileID)->Definition()));
  do {
    coo.UncheckedValue() = 0.5;
  } while (coo.Next() != DSL_OUT_OF_RANGE);

  theNet->UpdateBeliefs();

  //   DSL_sysCoordinates coo2(*(theNet->GetNode(smileID)->Value()));
  //   cout << "[SMILEG]: iup @" << name << ": ";
  //   do {
  //     cout << "[" << coo2.UncheckedValue() << "]";
  //   } while (coo2.Next() != DSL_OUT_OF_RANGE);
  // cout << endl;
}

smileFrameGlue* smileNodeGlue::getFrameGlue() {
  if (myFrameGlue == NULL) 
    myFrameGlue = 
      static_cast<smileFrameGlue*>(myFrame()->getGlue(SMILE_GLUE_IDENTIFIER));
  return myFrameGlue;
}

bool   smileNodeGlue::mpv() {
  return (p_true() >= 0.50);
}

double smileNodeGlue::p_bool(bool val) {
  DSL_network* theNet = getFrameGlue()->getNet();
  // this should probably move to set_evidence()
  // cout << "[SMILEG]: updating beliefs -p-" << endl;
  theNet->UpdateBeliefs();
  // cout << "ncc 'value' = " << theNet->GetNode(smileID)->Value() << endl;
  DSL_sysCoordinates coo(*(theNet->GetNode(smileID)->Value()));

  coo[0] = (val ? SMILE_TRUEINDEX : SMILE_FALSEINDEX);
  coo.GoToCurrentPosition();
  double P_childisyes = coo.UncheckedValue();
  /*
  cout << "[SMILEG]: P("
       << myNode()->entityBaseName()
       << ")="
       << P_childisyes 
       << endl;
  */
  return P_childisyes;
}

void smileNodeGlue::mpd(double* pd) {  
  umbc::uLog::annotate(MCLA_WARNING,"[SMILEG]: mpd not working...");
}

void smileNodeGlue::set_evidence(bool val) {
  // cout << "[SMILEG]: set_evidence called: frame="
  // << getFrameGlue() << " id=" << getSMILEid()
  // << " val=" << val << endl;
  // getFrameGlue()->set_evidence(val,getSMILEid());
  DSL_network* theNet = getFrameGlue()->getNet();
  theNet->GetNode(smileID)->Value()->SetEvidence((val ? SMILE_TRUEINDEX : 
						  SMILE_FALSEINDEX));
  theNet->UpdateBeliefs();
  // cout << "[SMILEG]: updating beliefs -s-" << endl;
}

bool smileNodeGlue::initPriors(vector<string>& d,vector<double>& v) {
  // get a coordinate systems into the CPT
  DSL_network* theNet = getFrameGlue()->getNet();
  DSL_sysCoordinates coo(*(theNet->GetNode(smileID)->Definition()));
  coo.GoFirst();
  int smileIdx = 0;
  do {
    int hgIdx = smileGlue::smileIndex2configIndex(smileIdx,depsAsCreated,d);
    // we need to check for 0.0 probability since SMILE interprets this very
    // strictly as "impossible"
    double cv = v[hgIdx];
    if ( v[hgIdx] == 1 ) cv = .999999;
    if ( v[hgIdx] == 0 ) cv = .000001;
    coo.UncheckedValue() = (1 - cv);
    coo.Next();
    coo.UncheckedValue() = cv; 
    smileIdx++;
  } while (coo.Next() != DSL_OUT_OF_RANGE);

  theNet->UpdateBeliefs();
  return true;
}

void smileNodeGlue::writeCPTtoArray(double* v) {
  DSL_network* theNet = getFrameGlue()->getNet();
  DSL_sysCoordinates coo(*(theNet->GetNode(smileID)->Definition()));
  coo.GoFirst();
  int smileIdx = 0;
  do {
    // A MONSTER HACK!! PLEASE BE CAREFUL SHOULD YOU EVER CHOOSE TO USE THIS
    int hgIdx = smileGlue::smileIndex2configIndex(smileIdx,depsAsCreated,depsAsCreated);
    // THE HACK IS IN THE USAGE OF "depsAsCreated" FOR BOTH THE SOURCE
    // DEPENDENCY LIST AND THE TARGET DEPENDENCY LIST
    // advance the coordinate system past the "false" entry
    coo.Next();
    v[hgIdx] = coo.UncheckedValue(); 
    smileIdx++;
  } while (coo.Next() != DSL_OUT_OF_RANGE);
}

////////////////////////////////////////////////////////////////
/// SMILE FRAME GLUE

smileFrameGlue::~smileFrameGlue() { }

bool smileFrameGlue::build() {
  // get the number of nodes in the 3 ontologies from the frame
  vector<mclNode*> allMCLnodes = myFrame()->allNodesV();
  topsort::topSort(allMCLnodes);

  // next thing to do is create a smileNodeGlue object for each node in the
  // model...
  umbc::uLog::annotate(MCLA_VRB,"[SMILEG]: creating SMILE nodes...");
  for (vector<mclNode*>::iterator nvi = allMCLnodes.begin();
       nvi != allMCLnodes.end();
       nvi++) {
    // cout << ">>>" << (*nvi)->entityBaseName() << endl;
    (*nvi)->addGlue(SMILE_GLUE_IDENTIFIER,
		    new smileNodeGlue((*nvi)->entityBaseName()+"-SMILEglue",
				      (*nvi)));
  }
  theNet.SetDefaultBNAlgorithm(DSL_ALG_BN_LAURITZEN);
  return true;
}

void smileFrameGlue::dumpToFile(string fn) {
  getNet()->WriteFile(fn.c_str());
}
