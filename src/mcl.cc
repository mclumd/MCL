#include "makeDot.h"
#include "mcl_api.h"
#include "mclConstants.h"
#include "mcl.h"
#include "../include/umbc/settings.h"
#include "../include/umbc/exceptions.h"
#include "mclFrame.h"
#include "mclLogging.h"
#include "ontology_config.h"
#include "ontology_reader.h"
#include "frameEntryVector.h"
#include "limits.h"

#include <algorithm>

using namespace metacog;
using namespace umbc;

// REB = Re Entrant Behavior
#define DEFAULT_REB_IGNORE   1 
#define DEFAULT_REB_REENTER  2
#define DEFAULT_REB_NEWFRAME 3
#define DEFAULT_REB          DEFAULT_REB_REENTER

mcl::mcl() : 
  mclEntity(),hName("no host"),cKey(""),maKey(""),
  active(false),synchronous(true),Hz(0),tickCounter(0),
  reb_policy(REB::makeREB()),sv(NULL),svl(0) {
  my_ov = new observable_vector(this,maKey+"-ov"); 
  oName=settings::getSysPropertyString("mcl.defaultOntology","basic");
}

bool mcl::setOntologyBase(string ont) { 
  if (ontology_reader::legit_ontologyname(ont)) {
    oName=ont; 
    uLog::annotate(MCLA_DBG,
		   entityName()+": ontology set to '"+getOntologyBase()+"'");
    return true;
  }
  else
    return false;
}

void mcl::setConfigKey(string kc) { 
  cKey=kc; 
  uLog::annotate(MCLA_DBG,
		    entityName()+": config key set to '"+getConfigKey()+"'");
}

void mcl::setREB(string reb)
{
  uLog::annotate(MCLA_DBG,
		    entityName()+": setting REB to '"+reb+"'");
  reb_policy = REB::makeREB(reb);
  uLog::annotate(MCLA_DBG,
		    entityName()+": set REB to '"+getREB()+"'");
}

string mcl::getREB(void)
{
  return reb_policy->name();
} 

// THIS MONITOR FUNCTION USES THE MULTI-FRAME ALGORITHM FOR DEALING WITH
// CONTEMPORANEOUS VIOLATIONS ....

/** monitor
 * 
 */
responseVector mcl::monitor() {

  responseVector frrvec;     // frame-response reply vector

  // 1. Trace our entrance
  uLog::annotate(MCLA_MSG,"[mcl/mcl]:: entering monitor call ("+getMAkey()+")");
  dump_ov();
  
  // 2. Test to see if we should enter the NAG cycle
  if (active && (synchronous || ticker.expired())) {
    if (!synchronous) {
      ticker.restartHz(Hz);
    }
    tickCounter++;
    
    // 3. Use nag to get get responses
    nag(frrvec);
    
  }
  
  // 4. Else say way we chose to do nothing
  else {
    if (active)
      umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]:: mcl throttling.");
    else 
      umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]:: mcl inactive.");
  }
  
  // 5. Check for marked (for death) groups
  deleteMarkedGroups();

  // 6. Return responses from NAG (if any)
  umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]:: exiting monitor call");
  return frrvec;
}


/** the NAG cycle.
 * 
 */
void mcl::nag(responseVector &frrvec) {    

  frameVec       hot_frames; // frames picked by note phase

  // +-----------+
  // |    NOTE   |
  // +-----------+
  
  // 1. See if there are any new volation
  bool new_violations = note(hot_frames);
  if (new_violations) {
    umbc::uLog::annotate(MCLA_VRB,"[mcl] note detected exp. violation(s)");
  }  
  sprintf(umbc::uLog::annotateBuffer,
          "[mcl/mcl]:: there are %ld hot frame(s)",
	  (long)hot_frames.size());
  umbc::uLog::annotate(MCLA_VRB);
	
  // 2. Move pending frames to hot_frames (and frames, if necessary)
  //    These frames will be automatically pushed through assess and guide
  for (frameVec::iterator pfi = pendingFrames.begin();
        pfi != pendingFrames.end();
        pfi++) {
    safeFramePush(*pfi,&hot_frames);
    safeFramePush(*pfi,&frames);
    sprintf(umbc::uLog::annotateBuffer,
            "[mcl/note]::waking up pending frame %s ... framecount(%ld) hotcount(%ld)",(*pfi)->entityName().c_str(),(long)frames.size(),(long)hot_frames.size());
    umbc::uLog::annotateFromBuffer(MCLA_MSG);
  }
  pendingFrames.clear();
        
  // 3. If there are hot frames, continue with access and guide
  if (hot_frames.size() != 0) {
     
    // +----------+ 
    // |  ACCESS  |
    // +----------+
        
    // 4. Assess each of the hot frames
    for (frameVec::iterator fvi = hot_frames.begin();
         fvi != hot_frames.end();
         fvi++) {
      assess(*fvi);
    }
        
    // 5. Attempt to merge frames...
    // <!!!>
    // mergeFrames(frvec);
        
    // +----------+ 
    // |  GUIDE   |
    // +----------+
          
    // 6. Loop over the hot frames getting a response from guide
    for (frameVec::iterator fvi2  = hot_frames.begin();
         fvi2 != hot_frames.end();
         fvi2++) {
      mclMonitorResponse *r = guide(*fvi2);
                
      // 7. Add the response to the list
      if (r != NULL) {
        frrvec.push_back(r);
      }
      
      // 8. Or mark the frame as a dead end if there was none  
      else {
        (*fvi2)->setState(FRAME_DEADEND);
      }
    } // end for (fvi2 
  } // end if (hot_frames.size() != 0)
} // end nag()


/** this is version 2 of noteForExpGroup
 * processes the expectations for an expectation group.
 * this version is updated to utilize the REB object code for modular
 * re-entrant behavior.
 *
 * @returns true if there is a violation in the group
 *
 */
bool mcl::noteForExpGroup2(frameVec& resVec,mclExpGroup* eGrp,bool complete) {

  // 1. No violations if group is marked for deletion and not completed
  // this is a make-or-break condition
  // it's legacy code and I don't remember what it is about 6/16/2010 MS
  // TODO: figure out what this does and why...  
  // DW 9/1/2010 I belive that this guards against aborted expection groups
  if (eGrp->markedForDelete() && !complete)
    return false;

  // 2. Announce ourselves
  if (complete)
    umbc::uLog::annotate(MCLA_VRB,"[mcl]:: noteForExpGrp2 call on completion " 
                         + eGrp->entityName());
  else
    umbc::uLog::annotate(MCLA_VRB,"[mcl]:: noteForExpGrp2 call to monitor " 
                         + eGrp->entityName());
  
  sprintf(uLog::annotateBuffer,"There are %d expectations in this group.",
           eGrp->expListSize());
  uLog::annotateFromBuffer(MCLA_MSG);

  // 3. Start with no violations
  bool rv=false;
  int  rvc=0;
   
  // 4. Loop for all expectations in this group
  for (expList::iterator elI = eGrp->expListHead();
        elI != eGrp->expListButt();
        elI++) {
        
    // 5. Check for a violation   
    if (((*elI)->checkAlways() && (*elI)->violation()) ||
        ((*elI)->checkOnExit() && complete && (*elI)->violation())) {
         
      // 6. Record the violation
      mclFrameEntryVector FEV(eGrp,*elI);
      eGrp->setLastViolationTick(tickCounter);
      rv=true;
      rvc++;
       
      // 7. Blog about the violation
      { 
        umbc::uLog::annotateStart(MCLA_VIOLATION);
        *umbc::uLog::log << "[mcl/mcl]::VIOLATION ~> " 
                         << (*elI)->entityName() 
                          << " in group " << hex << eGrp->get_egKey() << endl;
        umbc::uLog::annotate(MCLA_BREAK);
        umbc::uLog::annotateStart(MCLA_PRE);
        *umbc::uLog::log << "     exp>> " << (*elI)->describe() << endl;
        *umbc::uLog::log << "     OV >> " << ov2string() << endl;      
        *umbc::uLog::log << "     FEV>>" << FEV.describe() << endl;
        umbc::uLog::annotateEnd(MCLA_PRE);
        umbc::uLog::annotateEnd(MCLA_VIOLATION);
      }
       
      // 8. If debugging this is a good time to stop
      if (settings::getSysPropertyBool("mcl.breakOnViolation",false)) {
         exceptions::signal_exception("mcl.breakOnViolation=true exception");
      }
       
      // 9. Select a frame for analysis and guidance
      { 
        frameVec select_vec;
         
        // 9a. Select a frame (or frames) using the current policy
        reb_policy->selectFramesForReEntry(FEV,frames,select_vec);
         
        // 9b. If we didn't select any, create a new one
        if (select_vec.empty()) {
          FEV.vECode = ENTRY_NEW;
          mclFrame *m = new mclFrame(this,FEV);
          frames.push_back(m);
          umbc::uLog::annotateStart(MCLA_PRE);
          *umbc::uLog::log << "  New Frame being created @" <<hex<< m << endl;
          *umbc::uLog::log << "  ** eGroup @ " << hex << (unsigned long)eGrp
                           << "  key=" << eGrp->get_egKey() << endl;
          dump_eg_table();
          umbc::uLog::annotateEnd(MCLA_PRE);
          linkToIndicationFringe(m,*elI);
          resVec.push_back(m);
        }

        // 9b. A single frame was selected
        else if (select_vec.size() == 1) {
          select_vec[0]->reentry_recurrence(*elI,FEV);
          // FEV.vECode = ENTRY_VIOLATION;
          // select_vec[0]->reEnterFrame(*elI,FEV,false);
          // safeFramePush(select_vec[0],&resVec);
        }
         
        // 9c. Multiple frames were selected
        else {
          umbc::uLog::annotateStart(MCLA_PRE);
          *umbc::uLog::log << "[mcl]:: there are " << frames.size() 
                           << " frame(s) on hand." << endl;
          *umbc::uLog::log << "[mcl]:: there are " << select_vec.size() 
                           << " frame(s) selected by " 
                           << reb_policy->describe() << endl;
          umbc::uLog::annotateEnd(MCLA_PRE);
          // sort out the multi-frame madness
          throw UnimplementedException("multi-frame re-entry unimplemented.");
        }
      }
    } // if (volation
  } // for (expections
   
  // 10. If there were no voilations but the group had expectations
  if (!rv && !eGrp->empty()) {
  
    // 10a. Create a clean entry vector
    mclFrameEntryVector FEV(eGrp,NULL,ENTRY_CLEAN);
    
    // 10b. Get a frame(s) for reentry
    frameVec select_vec;
    reb_policy->selectFramesForReEntry(FEV,frames,select_vec);
    
    // 10c. If we got any, mark them as successfully reentered
    if (!select_vec.empty()) {
      for (frameVec::iterator fvi = select_vec.begin();
           fvi != select_vec.end();
           fvi++) {
        (*fvi)->reentry_success();
      } // end for (fvi)
    } // end if (!select_vec.empty)
  } // end if (!rv && ...)

  // 11. Return true if there was a violation
  if (rv) umbc::uLog::annotate(MCLA_MSG,"[mcl/note]::EG violation");
  else umbc::uLog::annotate(MCLA_MSG,"[mcl/note]::EG no violations");
  return rv;

}

/** initiates the note phase of MCL.
 * does two things - first processing pending frames, then checks current
 * expectations for new violations and processes those frames, too.
 */

bool mcl::note(frameVec& rvec) {
 
  // 1. Assume no exceptions
  bool rv = false; 

  // 2. Announce ourselves
  umbc::uLog::annotate(MCLA_MSG,"[mcl/note]::entering note phase");
  sprintf(umbc::uLog::annotateBuffer,"[mcl/note]::received %ld pending frame(s)",
	  (long)pendingFrames.size());
  umbc::uLog::annotateFromBuffer(MCLA_MSG);
  
  // 3. Loop over the expectation groups
  for (egMap::iterator egI=expGroups.begin();egI!=expGroups.end();egI++) {
    
    // 4. run through expectations
    mclExpGroup *tg = *egI;
    if (tg != NULL) {
    
      // 5. Create (find) a frame for each new violation
      rv = noteForExpGroup2(rvec,tg,false);
    }
  }

  // 5. Return true if there was a violation 
  if (rv) umbc::uLog::annotate(MCLA_MSG,"[mcl/note]::leaving note phase with a violation");
  else umbc::uLog::annotate(MCLA_MSG,"[mcl/note]::leaving note phase with no violations");
  return rv;
}

bool mcl::assess(mclFrame *m) { 
  
  // 1. Here we could do frame merging and importance assignment...
  
  // 2. Assess this frame
  m->assess();
  
  // 3. Always return true
  return true;
}

mclMonitorResponse* mcl::guide(mclFrame* m) { 
  return m->guide();
}  

float mcl::sensorValue(string selfSensorName) {
  return my_ov->v_double(selfSensorName);
}

float mcl::sensorValue(string objName, string obsName) {
  return my_ov->v_double(objName,obsName);
}

float mcl::sensor_value_pp(string sName) {
  return my_ov->v_double_pp(sName);
}

mclSensorDef *mcl::getSensorDef(string name) {
  return my_ov->get_sensorDef_pp(name);
}

legalspec *mcl::getLegalSpec(string name) {
  return my_ov->get_legalSpec_pp(name);
}

////////////////

void mcl::set_noise_profile(string selfOname,spvType npKey) {
  noiseProfile* theNP = noiseProfile::createNoiseProfile(npKey);
  my_ov->establish_np(selfOname,theNP);
  set_obs_prop_self(selfOname,PROP_NOISEPROFILE,npKey);
}

void mcl::set_noise_profile(string selfOname,spvType npKey,double param) {
  noiseProfile* theNP = noiseProfile::createNoiseProfile(npKey,param);
  my_ov->establish_np(selfOname,theNP);
  set_obs_prop_self(selfOname,PROP_NOISEPROFILE,npKey);
}

void mcl::set_noise_profile(string objnm,string obsnm,spvType npKey) {
  noiseProfile* theNP = noiseProfile::createNoiseProfile(npKey);
  my_ov->establish_np(objnm,obsnm,theNP);
  set_obs_prop(objnm,obsnm,PROP_NOISEPROFILE,npKey);
}

void mcl::set_noise_profile(string objnm,string obsnm,
			    spvType npKey,double param) {
  noiseProfile* theNP = noiseProfile::createNoiseProfile(npKey,param);
  my_ov->establish_np(objnm,obsnm,theNP);
  set_obs_prop(objnm,obsnm,PROP_NOISEPROFILE,npKey);
}

////////////////

mclHIADef *mcl::getHIADef(string name) {
  for (HIAVec::iterator hdvi=HIAdefs.begin();
       hdvi != HIAdefs.end();
       hdvi++)
    if ((*hdvi)->matchesName(name))
      return *hdvi;
  return NULL;
}

// TODO: clean this up and fit it more neatly into the framework
mclMonitorResponse *mcl::activateNodeDirect(string name,resRefType referent) {
  // establish a frame
  bool newFrame=false;
  mclFrame* thisFrame=referent2frame(referent);
  mclFrameEntryVector andFEV(ENTRY_HIA);
  if (thisFrame==NULL) {
    thisFrame = new mclFrame(this,andFEV);
    thisFrame->setEntityName("hii-"+name+"-frame");
    newFrame  = true;
  }
  
  // output
  umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]::node "+ name +
		      " activated direct from host ~~ " +
		      thisFrame->entityName());

  // find named node
  mclNode* thisNode = thisFrame->findNamedNode(name);
  if (thisNode == NULL) {
    mclMonitorResponse *r = new mclInternalErrorResponse();
    r->setResponse("could not find node "+name);
    if (newFrame) 
      delete thisFrame;
    // could go this way....
    // else thisFrame->setState(FRAME_ERROR);
    return r;
  }

  // bookkeeping
  if (newFrame) safeFramePush(thisFrame,&frames);
  // thisNode->assertTrue();
  thisNode->set_evidence(true);

  // now we should do assess then guide
  assess(thisFrame);
  mclMonitorResponse *r = guide(thisFrame);
  return r;

}

mclMonitorResponse *mcl::signalHIA(string name,resRefType referent) {
  mclHIADef * hia = getHIADef(name);
  if (hia == NULL) 
    return activateNodeDirect(name,referent);
  else
    return activateNodeDirect(hia->targetNodeName(),referent);
}

mclMonitorResponse *mcl::signalHIA(string name) {
  return signalHIA(name,RESREF_NO_REFERENCE);
}

mclFrame* mcl::mostRecentFrame() {
  mclFrame* cmrf = NULL;
  int blu = INT_MIN;
  for (frameVec::iterator fvi = frames.begin();
       fvi != frames.end();
       fvi++) {
    if ((*fvi)->lastUpdate() >= blu) {
      blu=(*fvi)->lastUpdate();
      cmrf=*fvi;
    }
  }
  return cmrf;
}

mclFrame* mcl::referent2frame(resRefType r) {
  for (frameVec::iterator fvi = frames.begin();
       fvi != frames.end();
       fvi++) {
    if (r == (*fvi)->frame_id)
      return (*fvi);
  }
  return NULL;
}

mclFrame* mcl::recoverExistingFrame(resRefType r,egkType e,egkType p) {
  {
    mclFrame* rv = referent2frame(r);
    if (rv) {
      if (!umbc::settings::quiet)
	umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]::recovering frame by direct reference @" + (rv)->entityName());
      return (rv);
    }
  }
  for (frameVec::iterator fvi = frames.begin();
       fvi != frames.end();
       fvi++) {
    if ((*fvi)->get_vegKey() == e) {
      umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]::recovering frame by similarity @" + (*fvi)->entityName());
      return (*fvi);
    }
  }
  umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]::no existing frames match incoming case.");
  return NULL;
}

void mcl::deleteExpGroup(mclExpGroup* egp) {
  if (egp != NULL)
    delete egp;
  egMap::iterator egpi = find(expGroups.begin(),expGroups.end(),egp);
  expGroups.erase(egpi);
}

void mcl::deleteMarkedGroups() {
  egMap deleteds;
  for (egMap::iterator egI=expGroups.begin();egI!=expGroups.end();egI++) {
    // run through expectations
    mclExpGroup *tg = (*egI);
    if ((tg != NULL) && (tg->markedForDelete()))
      deleteds.push_back(tg);
  }
  while (!deleteds.empty()) {
    deleteExpGroup(deleteds.back());
    deleteds.pop_back();
  }
}

void mcl::markExpGroupForDelete(egkType eg_key) {
  mclExpGroup* egp = getExpGroup(eg_key);
  if (egp != NULL)
    egp->markDelete();
}

void mcl::markExpGroupForDelete(mclExpGroup* egp) {
   if (egp != NULL)
      egp->markDelete();
}

void mcl::expectationGroupAborted(egkType eg_key) {
  mclExpGroup* egrp = getExpGroup(eg_key);
  sprintf(umbc::uLog::annotateBuffer,"[mcl/mcl]::expectation group 0x%lx is declared aborted.",(unsigned long)eg_key);
  umbc::uLog::annotateFromBuffer(MCLA_MSG);

  markExpGroupForDelete(egrp);
  // we don't do any of the effects checking or anything...
  // but should we???
}

void mcl::expectationGroupComplete(egkType eg_key) {
  mclExpGroup* egrp = getExpGroup(eg_key);
  markExpGroupForDelete(egrp);

  sprintf(umbc::uLog::annotateBuffer,"[mcl/mcl]::expectation group 0x%lx is declared complete.",(unsigned long)eg_key);
  umbc::uLog::annotateFromBuffer(MCLA_MSG);

  // okay, we have to do two things:
  // (1) check for effects violations on the way out
  // (2) check for frame recovery so we can update state/status

  if (egrp == NULL) {
    // this has to be a check/logic error on the host side...
    sprintf(umbc::uLog::annotateBuffer,"[mcl/mcl]::expectation group 0x%lx is declared specified as complete but cannot be found.",(unsigned long)eg_key);
    umbc::uLog::annotateFromBuffer(MCLA_WARNING);
    return;
  }
  else {
    if (noteForExpGroup2(pendingFrames,egrp,true)) {
      // a new violation occurred...
      // the appropriate frame should have been pulled and added to
      // pendingFrames (within the NFEG2), so there's really nothing 
      // to do here... I think (MS 6/15/2010)
    }
    else {
      // the frame exited peacefully, we basically want to try to 
      // recover a frame and update its success field if one exists
    }
  }
}

void mcl::declareExpectationGroup(egkType key,
				  egkType parent,
				  resRefType ref) {
  mclExpGroup* eeg = getExpGroup(key);
  if ((eeg != NULL) && !eeg->markedForDelete()) {
    char pe[512];
    sprintf(pe,"Multiply defined expectation group @0x%08lx",(unsigned long)key);
    umbc::exceptions::signal_exception(pe);
  }
  else {
    if (eeg !=NULL)
      umbc::uLog::annotate(MCLA_MSG,"[mcl/mcl]::Redefinition of EG okay because old one is completing.");
    addExpGroup(key,new mclExpGroup(this,key,parent,ref));
  }
}

mclExpGroup* mcl::getExpGroup(egkType key, bool returnMarkedGroups) {
  for (egMap::iterator egi = expGroups.begin();
       egi != expGroups.end();
       egi++) {
    if (((*egi)->get_egKey() == key) && 
	(returnMarkedGroups || !(*egi)->markedForDelete()))
      return (*egi);
  }
  return NULL;
}

void mcl::dump_eg_table() {
  uLog::annotate(MCLA_DBG,"EG Table:");
  for (egMap::iterator egi = expGroups.begin();
       egi != expGroups.end();
       egi++) {
    string pf="";
    if ((*egi)->markedForDelete())
      pf="[X] ";
    else pf="[ ]";
    sprintf(uLog::annotateBuffer,
	    "%s *eGrp=0x%8lx eGrp->key=0x%8lx\n",pf.c_str(),
	    (unsigned long)(*egi),
	    (unsigned long)(*egi)->get_egKey());
    uLog::annotateFromBuffer(MCLA_DBG);
  }
}

bool mcl::maybeRetireFrame(mclFrame* the_frame) {
  // decide whether or not to throw out the frame
  return false;
}

bool mcl::mustNotRetireFrame(mclFrame* the_frame) {
  // decide whether or not the frame must be kept
  return true;
}

int mcl::retireFrames(int maximum) {
  // Try to keep frames from growing without bounds
  // Returns number of frames retired
  
  // 1. Determine how many frames we need to lose
  int needed = frames.size() - maximum;
  int retired = 0;
  
  // 2. Not much to do if already under the limit
  if (needed <= 0) return retired;

  // 3. Loop looking for volenteer retireees
  for (frameVec::iterator fvi = frames.begin();
       fvi != frames.end();
       fvi++) {
    if (maybeRetireFrame(*fvi)) {
        // something that removes the frame
        ++retired;
    }    
  }     
    
  // 4. All done if enough took the buyout
  if (retired >= needed) return retired;
  
  // 5. Try pushing out some more
  for (frameVec::iterator fvi = frames.begin();
       fvi != frames.end() && retired < needed;
       fvi++) {
    if (!mustNotRetireFrame(*fvi)) {
        // something that removes the frame
        ++retired;
    }    
  }     
    
  // 6. Return number of frames that we let go
  return retired;  
}


void mcl::safeFramePush(mclFrame* nf,frameVec* fv) {
  if (find(fv->begin(),fv->end(),nf) == fv->end())
    fv->push_back(nf);
}

void mcl::markForReassess(mclFrame* nf) {
  safeFramePush(nf,&pendingFrames);
}

void mcl::processSuggImplemented(mclFrame* m) {
  // it might be smart to do something else here, like
  // record the time at which the suggestion was accepted (for durative
  // & diagnostic purposes)
  umbc::uLog::annotate(MCLA_MSG,"[mcl/hostAPI]:: host has indicated that response to "+ m->entityName() +" has been implemented.");
  m->reentry_implementing();
}

mclMonitorResponse* mcl::processSuggDeclined(mclFrame* m) {
  umbc::uLog::annotate(MCLA_MSG,"[mcl/hostAPI]:: host has indicated that response to "+ m->entityName() +" has been declined.");
  return m->reentry_declining();
}

mclMonitorResponse* mcl::processSuggFailed(mclFrame* m) {
  sprintf(umbc::uLog::annotateBuffer,"[mcl/mcl]::the host has signaled a response failure for frame 0x%lx", (long unsigned int)m);
  umbc::uLog::annotateFromBuffer(MCLA_MSG);
  return m->reentry_failing();
}

// THESE SHOULD ALL GIVE IMMEDIATE FEEDBACK !!

void mcl::processSuggIgnored(mclFrame* m) {
  sprintf(umbc::uLog::annotateBuffer,"[mcl/mcl]::the host has signaled a response was ignored for frame 0x%lx", (long unsigned int)m);
  umbc::uLog::annotateFromBuffer(MCLA_MSG);
  m->reentry_ignoring();
}

// THESE SHOULD ALL GIVE IMMEDIATE FEEDBACK !!

void mcl::processHostFeedback(bool hostReply, mclFrame* frame) { 
  frame->processFeedback(hostReply); 
  // don't like this -- should process on-cycle
  markForReassess(frame);
  umbc::uLog::annotate(MCLA_MSG,
		      "[mcl/mcl]:: the following frame is now pending: " + 
		      frame->entityBaseName());
}

void mcl::processInternalException(MCLException& E) {
  uLog::annotate(MCLA_ERROR,"MCLException: "+((string)E.what()));
}

//////////////////////////////////////////////////////////////////////////////
// OUTPUT STUFF
//////////////////////////////////////////////////////////////////////////////

void mcl::dumpMostRecentFrame() {
   mclFrame *mrf = frames.front();
   if (mrf != NULL) {
      mrf->dumpEntity(&cout);
   }  
}

void mcl::dumpFrameVec(frameVec& fv) {
  using namespace umbc;
  uLog::annotateStart(MCLA_PRE);
  *uLog::log << "Frame Vector: " << endl;
  for (frameVec::iterator fvi = fv.begin(); fvi!=fv.end(); fvi++) {
    *uLog::log << (*fvi)->describe() << endl;
  }
  uLog::annotateEnd(MCLA_PRE);
}

void mcl::dumpFrameDot(int which) {
  if (which >= (int)frames.size() || (which < 0)) 
    *umbc::uLog::log << "[mcl/mcl]::illegal frame index " << which << endl;
  else {
    mclFrame* x = frames[which];
    writeAll("dumps/"+ontology_configurator::cKey2path(cKey)+".dot",x);
  }
}
