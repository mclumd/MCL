#include "buildCore.h"
#include "oNodes.h"
#include "linkFactory.h"
#include "mclConstants.h"
#include "mclFrame.h"
#include "mcl_api.h"

// #include "configManager.h"

using namespace metacog;

mclOntology * mclGenerateCoreIndicationOntology(mclFrame *f) {
  mclOntology * io = new mclOntology("indication",f);
  mclNode     *  t = NULL;

  // *********** properties
  // properties are maintained in the indications ontology
  io->addNode(t = new mclHostProperty("sensorsCanFail",io,PCI_SENSORS_CAN_FAIL));
  io->addNode(t = new mclHostProperty("effectorsCanFail",io,PCI_EFFECTORS_CAN_FAIL));
  io->addNode(t = new mclHostProperty("intentional",io,PCI_INTENTIONAL));
  io->addNode(t = new mclHostProperty("parameterized",io,PCI_PARAMETERIZED));
  io->addNode(t = new mclHostProperty("declarative",io,PCI_DECLARATIVE));
  io->addNode(t = new mclHostProperty("retrainable",io,PCI_RETRAINABLE));
  io->addNode(t = new mclHostProperty("hlcController",io,PCI_HLC_CONTROLLING));
  io->addNode(t = new mclHostProperty("htnInPlay",io,PCI_HTN_IN_PLAY));
  io->addNode(t = new mclHostProperty("planInPlay",io,PCI_PLAN_IN_PLAY));
  io->addNode(t = new mclHostProperty("actionInPlay",io,PCI_ACTION_IN_PLAY));

  io->addNode(t = new mclGeneralIndication("hostProp",io));
  t->document("supernode for host properties to ensure full linkage in PNL.");
  
  linkFactory::makeAbstractionLink(io,"intentional","hostProp");
  linkFactory::makeAbstractionLink(io,"effectorsCanFail","hostProp");
  linkFactory::makeAbstractionLink(io,"sensorsCanFail","hostProp");
  linkFactory::makeAbstractionLink(io,"parameterized","hostProp");
  linkFactory::makeAbstractionLink(io,"declarative","hostProp");
  linkFactory::makeAbstractionLink(io,"retrainable","hostProp");
  linkFactory::makeAbstractionLink(io,"hlcController","hostProp");
  linkFactory::makeAbstractionLink(io,"htnInPlay","hostProp");
  linkFactory::makeAbstractionLink(io,"planInPlay","hostProp");
  linkFactory::makeAbstractionLink(io,"actionInPlay","hostProp");
  
  // *********** fringe
  // sensor class
  io->addNode(t = new mclConcreteIndication("state",io));
  io->addNode(t = new mclConcreteIndication("resource",io));
  io->addNode(t = new mclConcreteIndication("temporal",io));
  io->addNode(t = new mclConcreteIndication("spatial",io));

//   io->addNode(t = new mclConcreteIndication("control",io));
//   io->addNode(t = new mclConcreteIndication("reward",io));

//   // unused sensor calsses
//   io->addNode(t = new mclConcreteIndication("message",io));
//   io->addNode(t = new mclConcreteIndication("ambient",io));
//   io->addNode(t = new mclConcreteIndication("objectprop",io)); // is lowercase in dynamicILinks
  //   // critical/non-critical
//   // io->addNode(t = new mclConcreteIndication("critical",io));
//   // io->addNode(t = new mclConcreteIndication("noncritical",io));
//   // value class
//   io->addNode(t = new mclConcreteIndication("discrete",io));
//   io->addNode(t = new mclConcreteIndication("ordinal",io));

  // event class, taken from EFFECT/MAINTENANCE class
  // io->addNode(t = new mclConcreteIndication("maintenance",io));
  // io->addNode(t = new mclConcreteIndication("effect",io));

  // collect unlinked fringe
  io->addNode(t = new mclIndicationCoreNode("unclassifiedIndication",io));
  t->document("A core indication node for temporarily orphaned fringe nodes.");
  linkFactory::makeIFCLink(io,"state","unclassifiedIndication");  
  linkFactory::makeIFCLink(io,"temporal","unclassifiedIndication");  
  
//   t->document("Not sure these should go.");
//   linkFactory::makeAbstractionLink(io,"message","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"ambient","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"objectprop","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"spatial","unlinked-fringe");
//   // linkFactory::makeAbstractionLink(io,"critical","unlinked-fringe");
//   // linkFactory::makeAbstractionLink(io,"noncritical","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"discrete","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"ordinal","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"maintenance","unlinked-fringe");
//   linkFactory::makeAbstractionLink(io,"effect","unlinked-fringe");

//   linkFactory::makeAbstractionLink(io,"hostProp","unlinked-fringe");
  
  // divergence class (how the sensor failed to meet expectations
  io->addNode(t = new mclGeneralIndication("divergence",io));
  t->document("sensor behaved in a manner it was not supposed to.");
  // linkFactory::makeAbstractionLink(io,"unlinked-fringe","divergence");

  io->addNode(t = new mclGeneralIndication("aberration",io));
  t->document("sensor changed when it was not supposed to.");
  linkFactory::makeAbstractionLink(io,"aberration","divergence");

//   io->addNode(t = new mclGeneralIndication("cwa-violation",io));
//   t->document("closed world assumption violated (sensor should not change).");
//   linkFactory::makeAbstractionLink(io,"cwa-violation","aberration");

//   io->addNode(t = new mclGeneralIndication("cwa-decrease",io));
//   t->document("closed world assumption violated (sensor value decreased).");
//   linkFactory::makeAbstractionLink(io,"cwa-decrease","cwa-violation");

//   io->addNode(t = new mclGeneralIndication("cwa-increase",io));
//   t->document("closed world assumption violated (sensor value increased).");
//   linkFactory::makeAbstractionLink(io,"cwa-increase","cwa-violation");
  
  io->addNode(t = new mclGeneralIndication("breakout-low",io));
  t->document("sensor fell through a floor expectation.");
  linkFactory::makeAbstractionLink(io,"breakout-low","aberration");
  
  io->addNode(t = new mclGeneralIndication("breakout-high",io));
  t->document("sensor broke through a ceiling expectation.");
  linkFactory::makeAbstractionLink(io,"breakout-high","aberration");

  io->addNode(t = new mclGeneralIndication("missed-target",io));
  t->document("a target value was missed.");
  linkFactory::makeAbstractionLink(io,"missed-target","divergence");

//  io->addNode(t = new mclConcreteIndication("missed-wrongway",io));
//  t->document("the opposite of the expectation was encountered.");
//  linkFactory::makeAbstractionLink(io,"missed-wrongway","missed-target");
  
  io->addNode(t = new mclConcreteIndication("missed-unchanged",io));
  t->document("a sensor failed to change where a change was expected.");
  linkFactory::makeAbstractionLink(io,"missed-unchanged","missed-target");

  io->addNode(t = new mclConcreteIndication("short-of-target",io));
  t->document("a sensor went lower than expected.");
  linkFactory::makeAbstractionLink(io,"short-of-target","missed-target");

  io->addNode(t = new mclConcreteIndication("long-of-target",io));
  t->document("a sensor went higher than expected.");
  linkFactory::makeAbstractionLink(io,"long-of-target","missed-target");

//   io->addNode(t = new mclGeneralIndication("over",io));
//   t->document("sensor is over what is expected.");
//   linkFactory::makeAbstractionLink(io,"long-of-target","over");
//   linkFactory::makeAbstractionLink(io,"breakout-high","over");
//   linkFactory::makeAbstractionLink(io,"cwa-increase","over");
  
//   io->addNode(t = new mclGeneralIndication("under",io));
//   t->document("sensor is under what is expected.");
//   linkFactory::makeAbstractionLink(io,"short-of-target","under");
//   linkFactory::makeAbstractionLink(io,"breakout-low","under");
//   linkFactory::makeAbstractionLink(io,"cwa-decrease","under");

//   io->addNode(t = new mclConcreteIndication("late",io));
//   t->document("expectation is late.");

//   // core

//   // this is the catchall
//   io->addNode(t = new mclGeneralIndication("unclassifiedIndication",io));
//   t->document("A catchall node for unused indication fringe nodes (they cause PNL to break.");
//   linkFactory::makeIFCLink(io,"ordinal","unclassifiedIndication");
//   linkFactory::makeIFCLink(io,"discrete","unclassifiedIndication");

//   io->addNode(t = new mclIndicationCoreNode("deadlineMissed",io));
//   t->document("a deadline for completing activity has been missed.");
//   linkFactory::makeIFCLink(io,"temporal","deadlineMissed");
//   linkFactory::makeIFCLink(io,"late","deadlineMissed");

//   io->addNode(t = new mclIndicationCoreNode("rewardNotReceived",io));
//   t->document("a reward was expected but was not received.");
//   linkFactory::makeIFCLink(io,"reward","rewardNotReceived");  
//   linkFactory::makeIFCLink(io,"under","rewardNotReceived");  

//   io->addNode(t = new mclIndicationCoreNode("resourceOverflow",io));
//   t->document("a resource level was higher than expected.");
//   linkFactory::makeIFCLink(io,"resource","resourceOverflow");  
//   linkFactory::makeIFCLink(io,"over","resourceOverflow");  

//   io->addNode(t = new mclIndicationCoreNode("resourceDeficit",io));
//   t->document("a resource level went lower than expected.");
//   linkFactory::makeIFCLink(io,"resource","resourceDeficit");  
//   linkFactory::makeIFCLink(io,"under","resourceDeficit");  

//   io->addNode(t = new mclIndicationCoreNode("failedStateChange",io));
//   t->document("a state failed to change as specified.");
//   linkFactory::makeIFCLink(io,"state","failedStateChange");
//   linkFactory::makeIFCLink(io,"missed-unchanged","failedStateChange");

  io->addNode(t = new mclIndicationCoreNode("resourceUnchanged",io));
  t->document("a resource level apparently did not change.");
  linkFactory::makeIFCLink(io,"resource","resourceUnchanged");
  linkFactory::makeIFCLink(io,"missed-unchanged","resourceUnchanged");

  io->addNode(t = new mclIndicationCoreNode("moveFailure",io));
  t->document("a resource level apparently did not change.");
  linkFactory::makeIFCLink(io,"spatial","moveFailure");
  linkFactory::makeIFCLink(io,"missed-unchanged","moveFailure");

//   io->addNode(t = new mclIndicationCoreNode("unanticipatedStateChange",io));
//   t->document("a state changed in a manner not expected to.");
//   linkFactory::makeIFCLink(io,"state","unanticipatedStateChange");
//   linkFactory::makeIFCLink(io,"aberration","unanticipatedStateChange");

//   io->addNode(t = new mclIndicationCoreNode("assertedControlUnchanged",io));
//   t->document("a control sensor failed to change when expected.");
//   linkFactory::makeIFCLink(io,"control","assertedControlUnchanged");
//   linkFactory::makeIFCLink(io,"missed-unchanged","assertedControlUnchanged");

  // ********* these are host initiated indications

  io->addNode(t = new mclHostInitiatedIndication("sensorVerifiedBroken",io));
  t->document("the host has verified that a sensor appears broken");

  io->addNode(t = new mclHostInitiatedIndication("sensorVerifiedWorking",io));
  t->document("the host has verified that a sensor is working");
   
  return io;
}

mclOntology * mclGenerateCoreFailureOntology(mclFrame *f) {
  mclOntology * fo = new mclOntology("failures",f);
  mclNode *t;

  fo->addNode(t = new mclFailure("failure",fo));
  t->document("class of all failures.");

  fo->addNode(t = new mclFailure("knowledgeError",fo));
  t->document("class of failures pertaining to internal knowledge and representations of the agent.");
  linkFactory::makeAbstractionLink(fo,"knowledgeError","failure");  

  fo->addNode(t = new mclFailure("plantError",fo));
  t->document("class of failures pertaining to the physical agent.");
  linkFactory::makeAbstractionLink(fo,"plantError","failure");  

//   fo->addNode(t = new mclFailure("MPAError",fo)); // model-producing algorithm
//   linkFactory::makeAbstractionLink(fo,
//                                   "knowledgeError",
// 				     "MPAError");  

//   fo->addNode(t = new mclFailure("misParameterized",fo));
//   linkFactory::makeAbstractionLink(fo,"MPAError",
// 				     "misParameterized");  

  fo->addNode(t = new mclFailure("modelError",fo));
  t->document("class of failures in which an agent's model is causing anomalies.");
  linkFactory::makeAbstractionLink(fo,"modelError","knowledgeError");

//   fo->addNode(t = new mclFailure("proceduralModelError",fo));
//   linkFactory::makeAbstractionLink(fo,"modelError",
// 				     "proceduralModelError");  

  fo->addNode(t = new mclFailure("predictiveModelError",fo));
  t->document("class of failures in which a predictive model's predictions are inaccurate.");
  linkFactory::makeAbstractionLink(fo,"predictiveModelError","modelError");  

//   fo->addNode(t = new mclFailure("timingModelError",fo));
//   linkFactory::makeAbstractionLink(fo,"predictiveModelError",
// 				     "timingModelError");  

//   fo->addNode(t = new mclFailure("underFitError",fo));
//   linkFactory::makeAbstractionLink(fo,"predictiveModelError",
// 				     "underFitError");  

//   fo->addNode(t = new mclFailure("misFitError",fo));
//   linkFactory::makeAbstractionLink(fo,"predictiveModelError",
// 				     "misFitError");  

//   fo->addNode(t = new mclFailure("overFitError",fo));
//   linkFactory::makeAbstractionLink(fo,"predictiveModelError",
// 				     "overFitError");  

//   fo->addNode(t = new mclFailure("focusError",fo));
//   linkFactory::makeAbstractionLink(fo,"modelError",
// 				     "focusError");  

//   fo->addNode(t = new mclFailure("priorityError",fo));
//   linkFactory::makeAbstractionLink(fo,"modelError",
// 				     "priorityError");  

  fo->addNode(t = new mclFailure("sensorError",fo));
  t->document("class of failures in which an agent sensor is causing anomalies.");
    linkFactory::makeAbstractionLink(fo,"sensorError","plantError");

  fo->addNode(t = new mclFailure("sensorNoise",fo));
  t->document("an expectation violation has occurred due to excessive noise.");
  linkFactory::makeAbstractionLink(fo,"sensorNoise","sensorError");
				     

  fo->addNode(t = new mclFailure("sensorMiscalibrated",fo));
  t->document("a sensor is producing erroneous readings due to mis-calibration.");
  linkFactory::makeAbstractionLink(fo,"sensorMiscalibrated","sensorError");  

  fo->addNode(t = new mclFailure("sensorMalfunction",fo));
  t->document("a sensor is not operating according to specifications.");
  linkFactory::makeAbstractionLink(fo,"sensorMalfunction","sensorError");

  fo->addNode(t = new mclFailure("sensorStuck",fo));
  t->document("a sensor is no longer changing according to specifications.");
  linkFactory::makeAbstractionLink(fo,"sensorStuck","sensorMalfunction");

  fo->addNode(t = new mclFailure("sensorNonsensical",fo));
  t->document("a sensor is producing nonsensical readings.");
  linkFactory::makeAbstractionLink(fo,"sensorNonsensical","sensorMalfunction");

//   fo->addNode(t = new mclFailure("effectorError",fo));
//   linkFactory::makeAbstractionLink(fo,"failure",
// 				     "effectorError");  
//   t->setPropertyTest(PCI_EFFECTORS_CAN_FAIL,PC_YES);

//   fo->addNode(t = new mclFailure("effectorNoise",fo));
//   linkFactory::makeAbstractionLink(fo,"effectorError",
// 				     "effectorNoise");  
//   t->setPropertyTest(PCI_EFFECTORS_CAN_FAIL,PC_YES);

//   fo->addNode(t = new mclFailure("effectorFailure",fo));
//   linkFactory::makeAbstractionLink(fo,"effectorError",
// 				     "effectorFailure");  
//   t->setPropertyTest(PCI_EFFECTORS_CAN_FAIL,PC_YES);

//   fo->addNode(t = new mclFailure("resourceError",fo));
//   linkFactory::makeAbstractionLink(fo,"failure",
// 				     "resourceError");  

//   fo->addNode(t = new mclFailure("lackOfResourceError",fo));
//   linkFactory::makeAbstractionLink(fo,"resourceError",
// 				     "lackOfResourceError");  

//   fo->addNode(t = new mclFailure("resourcePropertyError",fo));
//   linkFactory::makeAbstractionLink(fo,"resourceError",
// 				     "resourcePropertyError");  
  
//   fo->addNode(t = new mclFailure("costError",fo));
//   linkFactory::makeAbstractionLink(fo,"resourceError",
// 				     "costError");  
  
  return fo;
}

mclOntology * mclGenerateCoreResponseOntology(mclFrame *f) {
  mclOntology * ro = new mclOntology("responses",f);
  mclNode *t;

  ro->addNode(t = new mclGeneralResponse("response",ro));
  ro->addNode(t = new mclGeneralResponse("internalResponse",ro));
  linkFactory::makeSpecificationLink(ro,"response","internalResponse");  
  ro->addNode(t = new mclGeneralResponse("plantResponse",ro));
  linkFactory::makeSpecificationLink(ro,"internalResponse","plantResponse");  
  ro->addNode(t = new mclGeneralResponse("systemResponse",ro));
  linkFactory::makeSpecificationLink(ro,"internalResponse","systemResponse");  

  ro->addNode(t = new mclGeneralResponse("runDiagnostic",ro));
  linkFactory::makeSpecificationLink(ro,"plantResponse","runDiagnostic");  
  
  /** special kind of node here... **/
  mcl_AD_InteractiveResponse* k=NULL;
  ro->addNode(k = new mcl_AD_InteractiveResponse("runSensorDiagnostic",
						 ro,CRC_SENSOR_DIAG));
  linkFactory::makeSpecificationLink(ro,"runDiagnostic",
				     "runSensorDiagnostic");
  k->onlyRunOnce();
  k->addPositive("sensorVerifiedBroken");
  k->addNegative("sensorVerifiedWorking");

  ro->addNode(t = new mclConcreteResponse("runEffectorDiagnostic",ro,CRC_EFFECTOR_DIAG));
  linkFactory::makeSpecificationLink(ro,"runDiagnostic",
				     "runEffectorDiagnostic");

  ro->addNode(t = new mclConcreteResponse("resetSensor",ro,CRC_SENSOR_RESET));
  t->document("physical reset of sensor.");
  linkFactory::makeSpecificationLink(ro,"plantResponse",
				     "resetSensor");

  ro->addNode(t = new mclConcreteResponse("resetEffector",ro,
					  CRC_EFFECTOR_RESET));
  linkFactory::makeSpecificationLink(ro,"plantResponse",
				     "resetEffector");
  
  ro->addNode(t = new mclGeneralResponse("amendKnowledgeBase",ro));
  linkFactory::makeSpecificationLink(ro,"systemResponse",
				     "amendKnowledgeBase");
  ro->addNode(t = new mclGeneralResponse("amendPredictiveModels",ro));
  linkFactory::makeSpecificationLink(ro,"amendKnowledgeBase",
				     "amendPredictiveModels");
  // ro->addNode(t = new mclGeneralResponse("amendProceduralModels",ro));
  // linkFactory::makeSpecificationLink(ro,"systemResponse",
  //  "amendKnowledgeBase");

  // ro->addNode(t = new mclConcreteResponse("activateLearningResponse",ro,CRC_ACTIVATE_LEARNING));
  // ro->addNode(t = new mclConcreteResponse("adjustParametersResponse",ro,CRC_ADJ_PARAMS));
  ro->addNode(t = new mclConcreteResponse("rebuildPredictiveModels",ro,CRC_REBUILD_MODELS));
  linkFactory::makeSpecificationLink(ro,"amendPredictiveModels",
				     "rebuildPredictiveModels");
  // ro->addNode(t = new mclConcreteResponse("revisitAssumptionsResponse",ro,CRC_REVISIT_ASSUMPTIONS));
  // ro->addNode(t = new mclConcreteResponse("amendControllerResponse",ro,CRC_AMEND_CONTROLLER));

  // linkFactory::makeSpecificationLink(ro,"modifyCopeResponse","modifyPredictiveResponse");  
  //   linkFactory::makeSpecificationLink(ro,"modifyCopeResponse","modifyProceduralResponse");  
  //   linkFactory::makeSpecificationLink(ro,"modifyPredictiveResponse","activateLearningResponse");  
  //   linkFactory::makeSpecificationLink(ro,"modifyPredictiveResponse","rebuildModelsResponse");  
  //   linkFactory::makeSpecificationLink(ro,"modifyProceduralResponse","adjustParametersResponse");
  //   linkFactory::makeSpecificationLink(ro,"modifyProceduralResponse","amendControllerResponse");
  //   linkFactory::makeSpecificationLink(ro,"modifyProceduralResponse","revisitAssumptionsResponse");
  
  // ro->addNode(t = new mclGeneralResponse("modifyAvoidResponse",ro));
  // ro->addNode(t = new mclConcreteResponse("reviseExpectationsResponse",ro,CRC_REVISE_EXPECTATIONS));
  
  // ro->addNode(t = new mclGeneralResponse("strategicChangeResponse",ro));
  // ro->addNode(t = new mclConcreteResponse("algorithmSwapResponse",ro,CRC_ALG_SWAP));
  // ro->addNode(t = new mclConcreteResponse("changeHLCResponse",ro,CRC_CHANGE_HLC));
  
  ro->addNode(t = new mclConcreteResponse("tryAgain",ro,
					  CRC_TRY_AGAIN));
  linkFactory::makeSpecificationLink(ro,"systemResponse","tryAgain");

//   linkFactory::makeSpecificationLink(ro,"response","internalResponse");
//   linkFactory::makeSpecificationLink(ro,"response","externalResponse");
//   linkFactory::makeSpecificationLink(ro,"internalResponse","plantResponse");
//   linkFactory::makeSpecificationLink(ro,"internalResponse","systemResponse");
//   linkFactory::makeSpecificationLink(ro,"externalResponse","askForHelpResponse");
//   linkFactory::makeSpecificationLink(ro,"askForHelpResponse","solicitSuggestionResponse");
//   linkFactory::makeSpecificationLink(ro,"askForHelpResponse","relinquishControlResponse");  
//   linkFactory::makeSpecificationLink(ro,"plantResponse","runDiagnosticResponse");  
//   linkFactory::makeSpecificationLink(ro,"runDiagnosticResponse","runSensorDiagnostic");  
//   linkFactory::makeSpecificationLink(ro,"runDiagnosticResponse","runEffectorDiagnostic");  
//   linkFactory::makeSpecificationLink(ro,"plantResponse","testHypothesisResponse");  
//   linkFactory::makeSpecificationLink(ro,"systemResponse","modifyKnowledgeResponse");  
//   linkFactory::makeSpecificationLink(ro,"systemResponse","strategicChangeResponse");  
//   linkFactory::makeSpecificationLink(ro,"systemResponse","tryAgainResponse");

//   linkFactory::makeSpecificationLink(ro,"modifyKnowledgeResponse","modifyAvoidResponse");  
//   linkFactory::makeSpecificationLink(ro,"modifyKnowledgeResponse","modifyCopeResponse");  

//   linkFactory::makeSpecificationLink(ro,"strategicChangeResponse","changeHLCResponse");  
//   linkFactory::makeSpecificationLink(ro,"strategicChangeResponse","algorithmSwapResponse");    

//   linkFactory::makeSpecificationLink(ro,"modifyAvoidResponse","reviseExpectationsResponse");  

  return ro;
}

void makeInterontologicalLinks(ontologyVector *ov) {
  mclOntology *iv = (*ov)[INDICATION_INDEX];
  mclOntology *fv = (*ov)[FAILURE_INDEX];
  mclOntology *rv = (*ov)[RESPONSE_INDEX];

  // i ~~> f
  linkFactory::makeDiagnosticLink(iv,"unclassifiedIndication",fv,
				  "failure");

  linkFactory::makeDiagnosticLink(iv,"resourceUnchanged",fv,"sensorStuck");
  linkFactory::makeDiagnosticLink(iv,"moveFailure",fv,"sensorStuck");
  // linkFactory::makeDiagnosticLink(iv,"moveFailure",fv,"effectorError");
  linkFactory::makeDiagnosticLink(iv,"resourceUnchanged",fv,
				  "predictiveModelError");
//   linkFactory::makeDiagnosticLink(iv,"failedStateChange",fv,"effectorFailure");

//   linkFactory::makeDiagnosticLink(iv,"deadlineMissed",fv,"effectorFailure");
//   linkFactory::makeDiagnosticLink(iv,"deadlineMissed",fv,"timingModelError");

  linkFactory::makeDiagnosticLink(iv,"sensorsCanFail",
				  fv,"sensorMalfunction");
//   linkFactory::makeDiagnosticLink(iv,"sensorsCanFail",
// 				  fv,"sensorStuck");
//   linkFactory::makeDiagnosticLink(iv,"sensorsCanFail",
// 				  fv,"sensorMalfunction");

  linkFactory::makeDiagnosticLink(iv,"sensorVerifiedBroken",
 				  rv,"runSensorDiagnostic");
  linkFactory::makeDiagnosticLink(iv,"sensorVerifiedWorking",
 				  rv,"runSensorDiagnostic");

  linkFactory::makeDiagnosticLink(iv,"sensorVerifiedBroken",
 				  rv,"resetSensor");
  linkFactory::makeDiagnosticLink(iv,"sensorVerifiedWorking",
 				  rv,"resetSensor");

  // f ~~> r
  linkFactory::makePrescriptiveLink(fv,"sensorMalfunction",
 				    rv,"runSensorDiagnostic");
  linkFactory::makePrescriptiveLink(fv,"predictiveModelError",
 				    rv,"amendPredictiveModels");
//   linkFactory::makePrescriptiveLink(fv,"effectorFailure",
// 				    rv,"runEffectorDiagnostic");

}

ontologyVector *metacog::mclGenerateOntologies(mclFrame *f) {
  try {
    vector<mclOntology *> *ov = new vector<mclOntology *>;
    ov->push_back(mclGenerateCoreIndicationOntology(f));
    ov->push_back(mclGenerateCoreFailureOntology(f));
    ov->push_back(mclGenerateCoreResponseOntology(f));
    makeInterontologicalLinks(ov);
    ((*ov)[0])->autoActivateProperties(*(f->getPV()));
    return ov;
  }
  catch (const char *c) {
    terminate();
  }
}

