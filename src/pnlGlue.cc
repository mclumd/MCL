#include "pnlGlue.h"
#include "mclFrame.h"
#include "hgGlue.h"
#include "topSort.h"

// PNL GLUE IMPLEMENTATION FOR PNL-BASED INFERENCE IN MCL

////////////////////////////////////////////////////////////////
/// PNL NODE GLUE

int pnlNodeGlue::autocounter = 0;

pnlNodeGlue::~pnlNodeGlue() {}

pnlNodeGlue::pnlNodeGlue(string name,mclNode* node) :
  nodeGlue(name,node),PNLid(autocounter) { 
  autocounter++; 
  myFrameGlue = static_cast<pnlFrameGlue*>(myFrame()->getGlue(PNL_GLUE_IDENTIFIER));
}

pnlNodeGlue::pnlNodeGlue(string name,mclNode* node,int id) :
  nodeGlue(name,node),PNLid(id) { 
  myFrameGlue = static_cast<pnlFrameGlue*>(myFrame()->getGlue(PNL_GLUE_IDENTIFIER));
}

pnlFrameGlue* pnlNodeGlue::getFrameGlue() {
  if (myFrameGlue == NULL) 
    myFrameGlue = 
      static_cast<pnlFrameGlue*>(myFrame()->getGlue(PNL_GLUE_IDENTIFIER));
  return myFrameGlue;
}

bool   pnlNodeGlue::mpv() {
  return (p_true() >= 0.50);
}

double pnlNodeGlue::p_bool(bool val) {
  double pd[2];
  mpd(pd);
  return (val ? pd[PNL_TRUE_VALUE] : pd[PNL_FALSE_VALUE]);
}

void pnlNodeGlue::mpd(double* pd) {  
  getFrameGlue()->mpd(pd,getPNLid());
}

void pnlNodeGlue::set_evidence(bool val) {
//   cout << "[PNLG]: set_evidence called: frame="
//        << getFrameGlue() << " id=" << getPNLid()
//        << " val=" << val << endl;
  getFrameGlue()->set_evidence(val,getPNLid());
}

bool pnlNodeGlue::initPriors(vector<string>& d,vector<double>& v) {
  static int q=0;
  if ((q++ % 25) == 0)
    *mclLogger::mclLog << "UNABLE TO INIT PRIORS FOR PNL GLUE!!" << endl;
  return false;
}

void pnlNodeGlue::writeCPTtoArray(double* a) {
  // this is a HUGE hack
  nodeGlue* hgng = getSiblingNodeGlue(HG_GLUE_IDENTIFIER);
  if (hgng != NULL) {
    hgng->writeCPTtoArray(a);
  }
  else {
    signalMCLException("Attempting to write CPT to array in PNL; this requires HG as a sibling glue, which has not been found.");
  }
}

////////////////////////////////////////////////////////////////
/// PNL NODE GLUE

pnlFrameGlue::~pnlFrameGlue() {
  delete model;
  delete bayesNet;
}

bool pnlFrameGlue::build() {
  // get the number of nodes in the 3 ontologies from the frame
  int numOfNds = myFrame()->nodeCount();
  vector<mclNode*> allMCLnodes = myFrame()->allNodesV();
  topsort::topSort(allMCLnodes);

  // next thing to do is create a pnlNodeGlue object for each node in the
  // model...
  // cout << "[PNLG]: creating PNL nodes...";
  int nodeCounter=0;

  for (vector<mclNode*>::iterator nvi = allMCLnodes.begin();
       nvi != allMCLnodes.end();
       nvi++) {
    (*nvi)->addGlue(PNL_GLUE_IDENTIFIER,
		    new pnlNodeGlue((*nvi)->entityBaseName()+"-PNLglue",
				    (*nvi),
				    nodeCounter));
    nodeCounter++;
  }

  // cout << "done." << endl;
  
  // cout << "[PNLG]: creating infrastructure for links...";
  // defines the node types used in the model
  CNodeType *nodeTypes = new CNodeType[1];
  nodeTypes[0] = CNodeType(true,2);

  // the node association associates each node (numbered from zero)
  // with a node type in the nodeTypes array
  intVector nodeAssociation = intVector(numOfNds,0);

  // This is neighbor specification
  // vector specifying the number of neighbors for each node
  int *numNeighb        = new int[numOfNds];
  // a ragged array for neighbor specification
  int **nbrs             = new int *[numOfNds];
  // a ragged array for neighbor relationship specification
  ENeighborType **orient = new ENeighborType *[numOfNds];
  // cout << "done." << endl;

  // cout << "[PNLG]: creating linkage (" << numOfNds << " nodes)...";
  for (int ni = 0; ni < numOfNds; ni++) {
    mclNode* thisNode = allMCLnodes[ni];
    // number of neighbors
    numNeighb[ni] = thisNode->inLinkCnt()+thisNode->outLinkCnt();
    // allocate the neighbor array
    nbrs[ni] = new int[numNeighb[ni]];
    // allocate the neighbor relationship array
    orient[ni] = new ENeighborType[numNeighb[ni]];
    // assign the neighbors values
    int cntr=0;
    for (llIterator inlinki = thisNode->inLink_begin();
	 inlinki != thisNode->inLink_end();
	 inlinki++) {
      mclNode* source = (*inlinki)->sourceNode();
      pnlNodeGlue* sourceGlue = static_cast<pnlNodeGlue*>(source->getGlue(PNL_GLUE_IDENTIFIER));
      nbrs[ni][cntr]=sourceGlue->getPNLid();
      orient[ni][cntr]=ntParent;
      cntr++;
    }
    for (llIterator outlinki = thisNode->outLink_begin();
	 outlinki != thisNode->outLink_end();
	 outlinki++) {
      mclNode* dest = (*outlinki)->destinationNode();
      pnlNodeGlue* destGlue = static_cast<pnlNodeGlue*>(dest->getGlue(PNL_GLUE_IDENTIFIER));
      nbrs[ni][cntr]=destGlue->getPNLid();
      orient[ni][cntr]=ntChild;      
      cntr++;
    }
  }
  // cout << "done." << endl;

  // create the graphical model
  model = CGraph::Create(numOfNds, numNeighb, nbrs, orient);
  // cout << "[PNLG]: created graphical model @ " << model << endl;
  
  // make the bayes net
  bayesNet = CBNet::Create( numOfNds , 1 /* #ntypes */, 
			    nodeTypes, &nodeAssociation.front(), model );
  
  //  cout << "[PNLG]: should have created bayes net @ " << bayesNet << endl;
  
  // allocate factors
  bayesNet->AllocFactors();
  for( int i = 0; i < numOfNds; i++ ) {
    bayesNet->AllocFactor(i);
    CFactor* pFactor = bayesNet->GetFactor(i);
//     cout << "[PNLG]: bayes net @" << bayesNet 
// 	 << " factor @ " << pFactor << endl;
    int      cells = 2*(int)pow(2,allMCLnodes[i]->inLinkCnt());
    float*   fp = new float[cells];
    for ( int q = 0; q < cells; q++)
      fp[q] = 0.5f;
    pFactor->AllocMatrix( fp, matTable );
    delete[] fp;
  }
  
  // create inference engine
  inferenceEngine = CNaiveInfEngine::Create( bayesNet );

  // now deallocate
  for (int dealli = 0 ; dealli < numOfNds; dealli++) {
    delete[] nbrs[dealli];
    nbrs[dealli]=NULL;
    delete[] orient[dealli];
    orient[dealli]=NULL;
  }
  
}


void pnlFrameGlue::mpd(double* pd,int nodeID) {
  int numQueryNds = 1;
  int queryNds[1];
  queryNds[0]=nodeID;
  
  inferenceEngine->MarginalNodes( queryNds, numQueryNds );
  const CPotential* pMarg = inferenceEngine->GetQueryJPD();
  
  if (!mclSettings::quiet) {
    int nnodes;
    const int* domain;
    pMarg->GetDomain( &nnodes, &domain );
//     std::cout<<" inference results: \n";
//     std::cout<<" probability distribution for nodes [ ";
//     for( int i = 0; i < nnodes; i++ )
//       {
// 	std::cout <<domain[i] <<" ";
//       }
//     std::cout<<"]"<<std::endl;
  }

  CMatrix<float>* pMat = pMarg->GetMatrix(matTable);
  // graphical model hase been created using dense matrix
  // so, the marginal is also dense
  EMatrixClass mtype = pMat->GetMatrixClass();
  // cout << "matrix type = " << mtype << endl;
  if( ! ( mtype == mcDense || 
	  mtype == mcNumericDense || 
	  mtype == mc2DNumericDense ) ) {
    signalMCLException("invalid matrix type returned on JPD query (PNL).");
  }

  int nEl;
  const float* data;
  static_cast<CNumericDenseMatrix<float>*>(pMat)->GetRawData(&nEl, &data);
  if (!mclSettings::quiet) {
    // cout << "nEl=" << nEl << " data @" << data << endl;
    // for( int m = 0; m < nEl; m++ )
    // std::cout << " " << data[m];
    // std::cout<<std::endl;
  }
  
  if (nEl != 2)
    signalMCLException("more than 2 elements returned in MPD (PNL).");

  for( int m = 0; m < nEl; m++ )
    pd[m]=data[m];

  // am I supposed to deallocate anything here?

}

void pnlFrameGlue::set_evidence(bool val, int nodeID) {
  int nObsNds = 1;
  int obsNds[1];
  obsNds[0]=nodeID;
  valueVector obsVals;
  obsVals.resize(1);
  obsVals[0].SetInt((val ? PNL_TRUE_VALUE : PNL_FALSE_VALUE));
  /*
  cout << "[PNLG]: creating evidence: <" << bayesNet
       << "," << nObsNds
       << "," << obsNds
       << "," << &obsVals 
       << ">" << endl;
  */
  CEvidence* pEvid = CEvidence::Create( bayesNet, nObsNds, obsNds, obsVals);
  inferenceEngine->EnterEvidence( pEvid );
}
