#include "../include/umbc/settings.h"
#include "../include/umbc/text_utils.h"
#include "../include/umbc/token_machine.h"

#include "mcl_multiagent_api.h"
#include "../include/mcl/mcl_symbols.h"
#include "mclLogging.h"
#include "Exceptions.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "include/symbol_translator.h"

// Darwin (BSD) does not define this send flag
// test
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

using namespace std;
using namespace umbc;
using namespace metacog;

string handle_command(string command);
void handle_connection(int newsockfd);
string rv2string(responseVector& rv);
string process_update(string key,
                      bool* success,
                      string update);
string process_update_into(string key,
                           bool* success,
                           mclMA::observables::update& into,
                           string update);

// Global loop control flags
bool accepting = true;
bool connected = true;
string connected_key = "";
symbol_translator translator('$');

// ----------------------------------------- main
int main(int argc, char **argv) {

  // 1. Set system properties from property file and command line
  // the libraries autoload sysprops now
  // settings::readSystemProps("umbcutil");
  // settings::readSystemProps("mcl");
  settings::readSystemProps("mclserver");
  settings::args2SysProperties( argc, argv);

  // 2. Initialize logging
  if (settings::getSysPropertyString("mclserver.logfile","") == "")
    uLog::setLogToStdOut();
  else {
    uLog::setLogToFile(settings::getSysPropertyString("mclserver.logfile"));
  }

  // 3. Set up signal to terminate gracefully
  // TERM should set connected and accepting to false
  //  HUP should set only connected to false

  // 4. Set up socket for host/MCL interaction
  int mcl_port = settings::getSysPropertyInt("mcl.port",5150);
  char char_port[31];  
  snprintf(char_port, 30, "%d", mcl_port);
  int sockfd, newsockfd, clilen;
  struct sockaddr_in serv_addr, cli_addr;

  // print host info to file if requested
  if (settings::getSysPropertyString("mcl.hostfile", "") != "") {
    string hostfilename = settings::getSysPropertyString("mcl.hostfile", "");
    const char* hfname = hostfilename.c_str();
    FILE *hfptr = fopen(hfname, "w");
    if (hfptr) {
      fprintf(hfptr, "port %d\n", mcl_port);
      fprintf(hfptr, "host %s\n", getenv("HOSTNAME"));
      fprintf(hfptr, "process %d\n", getpid());
      fclose(hfptr);
    }
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    uLog::annotate(ULAT_BAD,"[mcl_srv]::error opening port "+((string)char_port));       
    uLog::annotate(ULAT_BAD,"[mcl_srv]::errno = "+((string)strerror(errno)));       
    throw EnvironmentalMCLException("Socket error: "+((string)strerror(errno)));
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(mcl_port);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0) {
    uLog::annotate(ULAT_BAD,"[mcl_srv]::bind error on port "+((string)char_port));       
    uLog::annotate(ULAT_BAD,"[mcl_srv]::errno = "+((string)strerror(errno)));       
    throw EnvironmentalMCLException("Bind error: "+((string)strerror(errno)));
  }

  listen(sockfd,5);
  clilen = sizeof(cli_addr);

  uLog::annotate(ULAT_NORMAL,"[mcl_srv]::listening to port "+((string)char_port));

  // 5. Loop while accepting new connections
  while (accepting) {

    // 7. Wait for a connection
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       ((socklen_t*)&clilen));
    uLog::annotate(ULAT_NORMAL,
                   "[tcp_agnt]:: connection from client initiated...");
    if (newsockfd < 0)
      throw EnvironmentalMCLException("Accept() error: "+((string)strerror(errno)));

    // 8. Got a connection, handle interactions
    connected = true;
    connected_key = "";
    handle_connection(newsockfd);
    close(newsockfd);
  }

  // 9. Close up shop and go home
  close(sockfd);
  pthread_exit((void*)0);
}

// ----------------------------------------- fail, succ, and more
string fail(string msg) { return "fail("+msg+")"; };
string succ(string msg) { return "ok("+msg+")"; };
string more(string msg) { return "more("+msg+")"; }; //DW 01/04/10

// ----------------------------------------- handle_connection
void handle_connection(int newsockfd) {
  char buffer[256];
  int n;

  // 1. Loop while still connected
  while (connected) {
    bzero(buffer,256);

    // 2. Read a MCL command from the socket
    n = read(newsockfd,buffer,255);

    // 2a. If read error, drop connection
    if (n < 0) {
        uLog::annotate(ULAT_NOTGOOD,
                       "[tcp_agnt]:: Error on socket read");
        connected = false;
        continue;
    }

    // 2b. Clean off trailing white space
    while (n>=0 && ((buffer[n]=='\0') || isspace(buffer[n]))) {
        buffer[n--] = '\0'; //DW 12/22/2009
    }

    // 2c. Log command text
    uLog::annotate(ULAT_SEPERATOR,"");
    uLog::annotate(ULAT_NORMAL,"[mcl_srv]::command received on socket = '"+((string)buffer)+"'");

    // 2d. Handle close connection requests
    if ((strcmp(buffer,"close") == 0) ||
        (strcmp(buffer,"bye")   == 0) ||
        (strcmp(buffer,"quit")  == 0) ||
        (strcmp(buffer,"exit")  == 0)) {  //DW 12/23/2009
      uLog::annotate(ULAT_NOTGOOD,"[mcl_srv]:: connection closed by client.");
      connected=false;
      continue;
    }

    string buffer_string     = buffer;
    string buffer_translated = translator.translate_string(buffer_string);

    // 3. Process command and return response
    int send_rc=0;
    try {
      string response = handle_command((string)buffer_translated);
      response+="\n";
      send_rc = send(newsockfd,response.c_str(),
                     response.length(),MSG_NOSIGNAL);
    }
    catch (exception e) {
      string explanation = e.what();
      explanation+= "\n";
      send_rc = send(newsockfd,explanation.c_str(),
                explanation.length(),MSG_NOSIGNAL);
    } // end try

    // 4. If unable to return a response, drop connection (or worse)
    if (send_rc < 0) {
      char errbuff[512];
      strerror_r(errno,errbuff,512);
      string exstr = errbuff;
      if (errno == EPIPE) {
#ifdef BREAK_ON_SIGPIPE
	throw EnvironmentalMCLException("Send() error: "+exstr);
#else
        uLog::annotate(ULAT_BAD,"[mcl_srv]:: broken pipe, closing connection...");
        connected=false;
        continue;
#endif
      }
      throw EnvironmentalMCLException("Send() error: "+exstr);
    } // end if send_rc

  } // end while connected

  // only one way to get here, and that is when connected=false,
  // either by explicit close, signal, or a broken pipe.
  if (connected_key.size() != 0) {
    mclMA::releaseMCL(connected_key);
    connected_key = "";
  }
}

// ----------------------------------------- initialize
// initialize(str key)
// initialize(str key, int hrz)

string command_initialize(string key,
                          paramStringProcessor psp) {

  // 1. Monitor frequency is optional
  if (psp.hasMoreParams()) {
    int hrz = textFunctions::numval(psp.getNextParam());
    mclMA::initializeMCL(key, hrz);
  }
  else
    mclMA::initializeMCL(key, 0);

  // 2. Save MCL key
  connected_key = key;
  uLog::annotate(ULAT_NORMAL,"[mcl_srv]::set connected key to ''"+key+"'");

  // 3. Initialize always succeeds
  return succ("initialized '"+key+"'.");
}

// ----------------------------------------- ontology
// ontology(str key, str ontology)

// 8/10/2011 -- 'ontology' command deprecated and ontology parameter 
// MS           added to 'configure' command

/* deprecated
string command_ontology(string key,
                        paramStringProcessor psp) {
  // 1. Ontology name is required
  if (!psp.hasMoreParams())
    return fail("missing ONTOLOGY parameter.");
  string o = psp.getNextParam();

  // 2. Execute the api call and report results
  if (mclMA::chooseOntology(key,o))
    return succ("ontology for " + key + " set to " + o);
  return fail("ontology "+o+" not available.");
}
*/

// ----------------------------------------- configure
// configure(str key, str ontology, str domain)
// configure(str key, str ontology, str domain, str agent)
// configure(str key, str ontology, str domain, str agent, str controller)

// 8/10/2011 -- 'ontology' command deprecated and ontology parameter 
// MS           added to 'configure' command

string command_configure(string key,
                         paramStringProcessor psp) {

  // 0. Ontology name is required
  if (!psp.hasMoreParams())
    return fail("missing ONTOLOGY parameter.");
  string o_name = psp.getNextParam();

  // 1. Domain name is required
  if (!psp.hasMoreParams())
    return fail("missing DOMAIN NAME parameter.");
  string dom_name = psp.getNextParam();

  // 2. Agent name is optional
  if (psp.hasMoreParams()) {
    string agent_name = psp.getNextParam();

    // 3. Contoller name is optional
    if (psp.hasMoreParams()) {
      string controller_name = psp.getNextParam();

      // 4. Configure with domain, agent, and controller
      if (mclMA::configureMCL(key,o_name,dom_name,agent_name,
			      controller_name))
        return succ("configured " + key + " with " + o_name + ":" +
                    dom_name + "/" + agent_name +
                    "/" + controller_name + ".");
      return fail("error configuring " + key + " with " + o_name + ":" +
                  dom_name + "/" + agent_name +
                  "/" + controller_name + ".");
    }

    // 5. Configure with domain and agent
    if (mclMA::configureMCL(key,o_name,dom_name,agent_name))
      return succ("configured " + key + " with " + o_name + ":" +
                  dom_name + "/" + agent_name + ".");
    return fail("error configuring " + key + " with " + o_name + ":" +
                dom_name + "/" + agent_name + ".");
  }

  // 6. Comfigure with just domain name
  if (mclMA::configureMCL(key,o_name,dom_name))
    return succ("configured " + key + " with " + o_name + ":" +
                dom_name + ".");
  return fail("error configuring " + key + " with " + o_name + ":" +
              dom_name + ".");
}

// ----------------------------------------- newDefaultPV
// newDefaultPV(str key)

string command_newDefaultPV(string key) {
  mclMA::newDefaultPV(key);
  return succ("default PV created for " + key + ".");
}

// ----------------------------------------- popDefaultPV
// popDefaultPV(str key)

string command_popDefaultPV(string key) {
  mclMA::popDefaultPV(key);
  return succ("default PV popped for " + key + ".");
}

// ----------------------------------------- resetDefaultPV
// resetDefaultPV(str key)

string command_resetDefaultPV(string key) {
  mclMA::reSetDefaultPV(key);
  return succ("default PV reset for " + key + ".");
}

// ----------------------------------------- setPropertyDefault
// setPropertyDefault(str key, str ind, str pv)

string command_setPropertyDefault(string key,
                                  paramStringProcessor psp) {
  bool sv_fail = false;

  // 1. Get ind, error if none
  if (!psp.hasMoreParams())
    return fail("missing PROPERTY_KEY parameter for " + key + ".");
  string zind = psp.getNextParam();
  int ind = symbols::smartval_int(zind, &sv_fail);
  if (sv_fail)
    return fail("PROPERTY_KEY '"+zind+"' not found for '"+key+"'."); 

  // 2. Get pv, error if none
  if (!psp.hasMoreParams())
    return fail("missing VALUE parameter for "+ zind + ".");
  string zpv = psp.getNextParam();
  int pv = symbols::smartval_int(zpv, &sv_fail);
  if (sv_fail)
    return fail("VALUE '"+zpv+"' not found for '"+key+"'."); 

  // 3. Call MCL API to set property default
  mclMA::setPropertyDefault(key, ind, pv);

  // 4. Return success
  return succ("default property for " + zind + " set to " + zpv + ".");
}

// ----------------------------------------- setEGPropoerty
// setEGPropoerty(str key, str egkey, str pkey, str pval)

string command_setEGPropoerty(string key,
                              paramStringProcessor psp) {
  bool sv_fail = false;

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EGKEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Get pkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing PKEY parameter.");
  string zpkey = psp.getNextParam();
  pkType pkey = symbols::smartval_int(zpkey, &sv_fail);
  if (sv_fail)
    return fail("PKEY '"+zpkey+"' not found for '"+zegkey+"'."); 

  // 3. Get pval, error if none
  if (!psp.hasMoreParams())
    return fail("missing PVAL parameter.");
  string zpnum = psp.getNextParam();
  int pnum = symbols::smartval_int(zpnum, &sv_fail);
  if (sv_fail)
    return fail("PVAL '"+zpnum+"' not found for '"+zegkey+"'."); 
  pvType pval = (pnum == 0) ? false : true;

  // 4. Call MCI api to set EG property
  cerr << "setEGProperty may have problems: "
       << "egkType is a pointer type, pval is a bool using smartval_int"
       << endl;
  mclMA::setEGProperty(key,egkey,pkey,pval);

  // 5. Return success
  return succ("property set.");
}

// ----------------------------------------- registerHIA
// registerHIA(str key, str name, str nodename)

string command_registerHIA(string key,
                          paramStringProcessor psp) {

  // 1. Get name, error if none
  if (!psp.hasMoreParams())
    return fail("missing NAME parameter.");
  string zname = psp.getNextParam();

  // 2. Get nodename, error if none
  if (!psp.hasMoreParams())
    return fail("missing NODENAME parameter.");
  string znode = psp.getNextParam();

  // 3. Call MCL api to register HIA
  mclMA::HIA::registerHIA(key,zname,znode);

  // 4. Return success
  return succ("HIA registered.");
}

// ----------------------------------------- signalHIA
// signalHIA(str key, str name, str nodename)

string command_signalHIA(string key,
                          paramStringProcessor psp) {

  // 1. Get name, error if none
  if (!psp.hasMoreParams())
    return fail("missing NAME parameter.");
  string zname = psp.getNextParam();

  // 2. Get referent, use if available
  if (!psp.hasMoreParams()) {
    string zref = psp.getNextParam();
    resRefType ref = textFunctions::longval(zref);
    mclMA::HIA::signalHIA(key,zname,ref);
  } 
  else 
    mclMA::HIA::signalHIA(key,zname);

  // 4. Return success
  return succ("HIA signalled.");
}

// ----------------------------------------- declareEG
// declareEG(str key, str egkey)
// declareEG(str key, str egkey, str parentref)
// declareEG(str key, str egkey, str parentref, str reftype)

string command_declareEG(string key,
                          paramStringProcessor psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. If not parent ref use short form
  if (!psp.hasMoreParams()) {
      mclMA::declareExpectationGroup(key,egkey);
      return succ("expectation group declared (no parent/ref).");
  }

  // 3. Else get parent ref
  string zparkey = psp.getNextParam();
  egkType parkey = textFunctions::unumval(zparkey);

  // 4. Get optional ref type
  string zref = "NOREF";
  resRefType ref = (resRefType)RESREF_NO_REFERENCE;
  if (!psp.hasMoreParams()) {
    zref = psp.getNextParam();
    ref = (resRefType)textFunctions::unumval(zref);
  }

  // 5. Call MCL api to declare long form Expectation Group
  mclMA::declareExpectationGroup(key,egkey,parkey,ref);

  // 6. Return success
  return succ("expectation group declared (with parent/ref).");
}

// ----------------------------------------- EGabort
// EGabort(str key, str egkey)
string command_EGabort(string key,
                          paramStringProcessor psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Call MCL api to abort expectation group
  mclMA::expectationGroupAborted(key,egkey);

  // 3. Return success
  return succ("expectation group aborted.");
}

// ----------------------------------------- EGcomplete
// EGcomplete(str key, str egkey)
string command_EGcomplete(string key,
                          paramStringProcessor psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Handle deprecated short form
  //    expectationGroupComplete(k,ke) is deprecated.
  //    Please use the update version or suffer dire consequences.
  if (!psp.hasMoreParams()) {
    mclMA::expectationGroupComplete(key,egkey);
    return succ("EG " + zegkey+ " Completed (no update).");
  }

  // 3. Else complete Expection Group with sensor update
  string updates = psp.getNextParam();
  bool okay=false;
  mclMA::observables::update this_update;
  string rv = process_update_into(key,&okay,this_update,updates);
  mclMA::expectationGroupComplete(key,egkey,this_update);
  if (okay) return succ("EG " + zegkey + " Completed.");
  else return fail("EG " + zegkey + " completion failed: "+rv);
}

// ----------------------------------------- declareExp
// declareExp(str key, str egkey, str code, str arg)

string command_declareExp(string key,
                          paramStringProcessor psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Get ec_code, error if none
  if (!psp.hasMoreParams())
    return fail("missing EC_CODE parameter.");
  bool sv_fail = false;
  string zcode = psp.getNextParam();
  egkType code = symbols::smartval_int(zcode,&sv_fail);
  if (sv_fail)
    return fail("failed to parse EC_CODE "+zcode+".");

  // 3. Get ARG parameter, error if none
  if (!psp.hasMoreParams())
    return fail("missing ARG parameter.");
  string zarg = psp.getNextParam();
  double arg = textFunctions::dubval(zarg);

  // 4. Call the MCL api to declare the expectation
  mclMA::declareExpectation(key,egkey,code,arg);

  // 5. Return success
  return succ("general expectation declared.");
}

// ----------------------------------------- declareSelfExp
// declareSelfExp(str key, str egkey, str observ, str code)
// declareSelfExp(str key, str egkey, str observ, str code, str arg)

string declSelfExp2(string& key,egkType egk, ecType eCode, vector<string>& rp) {
  mclMA::declareSelfExpectation(key,egk,rp.at(0),eCode);
  return succ("self expectation declared (1 arg).");
}

string declSelfExp3(string& key,egkType egk, ecType eCode, vector<string>& rp) {
  double arg = textFunctions::dubval(rp.at(1));
  mclMA::declareSelfExpectation(key,egk,rp.at(0),eCode,arg);
  return succ("self expectation declared (1+1 arg).");
}

string declSelfExp5(string& key,egkType egk, ecType eCode, vector<string>& rp) {
  double arg1 = textFunctions::dubval(rp.at(2));
  double arg2 = textFunctions::dubval(rp.at(3));
  mclMA::declareSelfExpectation(key,egk,rp.at(0),rp.at(1),eCode,arg1,arg2);
  return succ("self expectation declared (2+2 arg).");
}

string command_declareSelfExp(string& key,
                              paramStringProcessor& psp) {
  vector<string> params;
  psp.paramListRest(params);
  if (params.size() < 2) return fail("not enough arguments.");

  egkType egkey = (egkType)textFunctions::unumval(params.at(0));
  params.erase(params.begin());
  bool sv_fail = false;
  ecType eCode = symbols::smartval_int(params.at(0),&sv_fail);
  if (sv_fail)
    return fail("failed to parse EC_CODE "+params.at(0)+".");
  params.erase(params.begin());

  switch(params.size()) {
  case 1:
    // sensor
    return declSelfExp2(key,egkey,eCode,params);
  case 2:
    // sensor, value
    return declSelfExp3(key,egkey,eCode,params);
  case 4:
    // sensor, sensor, value, v2
    return declSelfExp5(key,egkey,eCode,params);
  default:
    return fail("no matching argument list for declareSelfExp."); 
  }
}

  /*
string command_declareSelfExp(string key,
                              paramStringProcessor& psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Get ec_code, error if none
  if (!psp.hasMoreParams())
    return fail("missing EC_CODE parameter.");
  bool sv_fail = false;
  string zcode = psp.getNextParam();
  egkType code = symbols::smartval_int(zcode,&sv_fail);
  if (sv_fail)
    return fail("failed to parse EC_CODE "+zcode+".");

  // 3. Get observable, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE parameter.");
  string obs = psp.getNextParam();

  // 4. If no more arguments, use short form api call
  if (!psp.hasMoreParams()) {
    mclMA::declareSelfExpectation(key,egkey,obs,code);
    return succ("self expectation declared (0-arg).");
  }

  // 5. Get ARG parameter
  string zarg = psp.getNextParam();
  double arg = textFunctions::dubval(zarg);

  // 6. Call the MCL api to declare the self expectation
  mclMA::declareSelfExpectation(key,egkey,obs,code,arg);

  // 7. Return success
  return succ("self expectation declared (1-arg).");
}
  */

// ----------------------------------------- declareObjExp
// declareObjExp(str key, str egkey, str obj, str observ, str code)
// declareObjExp(str key, str egkey, str obj, str observ, str code, str arg)

string command_declareObjExp(string& key,
                             paramStringProcessor& psp) {

  // 1. Get egkey, error if none
  if (!psp.hasMoreParams())
    return fail("missing EG_KEY parameter.");
  string zegkey = psp.getNextParam();
  egkType egkey = (egkType)textFunctions::unumval(zegkey);

  // 2. Get ec_code, error if none
  if (!psp.hasMoreParams())
    return fail("missing EC_CODE parameter.");
  bool sv_fail = false;
  string zcode = psp.getNextParam();
  egkType code = symbols::smartval_int(zcode,&sv_fail);
  if (sv_fail)
    return fail("failed to parse EC_CODE "+zcode+".");

  // 3. Get object, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT parameter.");
  string obj = psp.getNextParam();

  // 4. Get observable, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE parameter.");
  string obs = psp.getNextParam();

  // 5. If no more arguments, use short form api call
  if (!psp.hasMoreParams()) {
    mclMA::declareObjExpectation(key,egkey,obj,obs,code);
    return succ("obj expectation declared (0-arg).");
  }

  // 6. Get ARG parameter
  string zarg = psp.getNextParam();
  double arg = textFunctions::dubval(zarg);

  // 6. Call the MCL api to declare the self expectation
  mclMA::declareObjExpectation(key,egkey,obj,obs,code,arg);

  // 7. Return success
  return succ("obj expectation declared (1-arg).");
}

// ----------------------------------------- suggestionImplemented
// suggestionImplemented(str key, str referent)

string command_suggestionImplemented(string key,
                                     paramStringProcessor psp) {

  // 1. Get referent, error if none
  if (!psp.hasMoreParams())
    return fail("missing REFERENT parameter.");
  string zref = psp.getNextParam();
  resRefType ref = (resRefType)textFunctions::unumval(zref);

  // 2. Call MCL api
  mclMA::suggestionImplemented(key,ref);

  // 3. Return success
  return succ("Suggestion " + zref + " Implemented.");
}

// ----------------------------------------- suggestionFailed
// suggestionFailed(str key, str referent)

string command_suggestionFailed(string key,
                                paramStringProcessor psp) {

  // 1. Get referent, error if none
  if (!psp.hasMoreParams())
    return fail("missing REFERENT parameter.");
  string zref = psp.getNextParam();
  resRefType ref = (resRefType)textFunctions::unumval(zref);

  // 2. Call MCL api
  mclMonitorResponse* mcmr = mclMA::suggestionFailed(key,ref);
  string rv = mcmr->to_string();
  delete mcmr;

  // 3. Return success
  return succ(rv);

}

// ----------------------------------------- suggestionDeclined
// suggestionDeclined(str key, str referent)

string command_suggestionDeclined(string key,
				  paramStringProcessor psp) {

  // 1. Get referent, error if none
  if (!psp.hasMoreParams())
    return fail("missing REFERENT parameter.");
  string zref = psp.getNextParam();
  resRefType ref = (resRefType)textFunctions::unumval(zref);

  // 2. Call MCL api
  mclMonitorResponse* mcmr = mclMA::suggestionDeclined(key,ref);
  string rv = mcmr->to_string();
  delete mcmr;

  // 3. Return success
  return succ(rv);
}

// ----------------------------------------- suggestionIgnored
// suggestionIgnored(str key, str referent)

string command_suggestionIgnored(string key,
                                 paramStringProcessor psp) {

  // 1. Get referent, error if none
  if (!psp.hasMoreParams())
    return fail("missing REFERENT parameter.");
  string zref = psp.getNextParam();
  resRefType ref = (resRefType)textFunctions::unumval(zref);

  // 2. Call MCL api
  mclMA::suggestionIgnored(key,ref);

  // 3. Return success
  return succ("Suggestion " + zref + " Ignored.");
}

// ----------------------------------------- provideFeedback
// provideFeedback(str key, str referent, bool fb)

string command_provideFeedback(string key,
                                 paramStringProcessor psp) {

  // 1. Get referent, error if none
  if (!psp.hasMoreParams())
    return fail("missing REFERENT parameter.");
  string zref = psp.getNextParam();
  resRefType ref = (resRefType)textFunctions::unumval(zref);

  // 2. Get feedback, error if none
  if (!psp.hasMoreParams())
    return fail("missing FEEDBACK parameter.");
  string zfb = psp.getNextParam();
  bool fb = textFunctions::boolval(zfb);

  // 2. Call MCL api
  mclMA::provideFeedback(key,fb,ref);

  // 3. Return success
  return succ("Feedback to " + zref + " noted.");
}

// ----------------------------------------- monitor
// monitor(str key, sensor_update)

string command_monitor(string key,
                       paramStringProcessor psp) {

  // 1. Get sensor update, error if none
  if (!psp.hasMoreParams())
    return fail("missing sensor update.");
  string zsensors = psp.getNextParam();
  bool okay=true;
  mclMA::observables::update this_update;
  string rv = process_update_into(key,&okay,this_update,zsensors);
  if (!okay)
    return fail("Invalid sensor update: " + rv);

  // 2. Call MCL api
  responseVector rev = mclMA::monitor(key,this_update);

  // 3. convert to response and destroy the evidence
  string returnString = rv2string(rev);
  uLog::annotate(MCLA_DBG,"returning: "+rv2string(rev));
  mclMonitorResponse::clean_out(rev);

  // 4. Return success
  return succ(returnString);

}

// ----------------------------------------- declareObservableSelf
// declareObservableSelf(str key, str oname, defv)
// declareObservableSelf(str key, str oname)

string command_declareObservableSelf(string key,
                                     paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. If defv,  declare with defv
  if (psp.hasMoreParams()) {
    string zdefv = psp.getNextParam();
    double defv = textFunctions::numval(psp.getNextParam());
    mclMA::observables::declare_observable_self(key,oname,defv);
    return succ("declared '"+oname+"'.");
  }

  // 3. Else use short form
  mclMA::observables::declare_observable_self(key,oname);
  return succ("declared self-obs '"+oname+"'.");
}

// ----------------------------------------- declareObservableObjType
// declareObservableObjType(str key, str oname)

string command_declareObservableObjType(string key,
                                        paramStringProcessor psp) {

  // 1. Get object type name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_TYPENAME parameter.");
  string tname = psp.getNextParam();

  // 2. Call MCL api
  mclMA::observables::declare_observable_object_type(key, tname);

  // 3. Return success
  return succ("declared object type '"+tname+"'.");
}

// ----------------------------------------- declareObjField
// declareObjField(str key, str tname, str fname)

string command_declareObjField(string key,
                               paramStringProcessor psp) {

  // 1. Get object type name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_TYPENAME parameter.");
  string tname = psp.getNextParam();

  // 2. Get object field name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_FIELDNAME parameter.");
  string fname = psp.getNextParam();

  // 3. Call MCL api
  mclMA::observables::declare_object_type_field(key,tname,fname);

  // 4. Return success
  return succ("declared obj-field '"+tname+"."+fname+"'.");
}

// ----------------------------------------- noticeObj
// noticeObj(str key, str tname, str oname)

string command_noticeObj(string key,
                        paramStringProcessor psp) {

  // 1. Get object type name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_TYPENAME parameter.");
  string tname = psp.getNextParam();

  // 2. Get object name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_NAME parameter.");
  string oname = psp.getNextParam();

  // 3. Call MCL api
  try {
    mclMA::observables::notice_object_observable(key,tname,oname);
  }
  catch (MCLException e) {
    return fail(e.what());
  }

  // 4. Return success
  return succ("noticed '"+tname+"::"+oname+"'.");
}

// ----------------------------------------- unnoticeObj
// unnoticeObj(str key, str o)

string command_unnoticeObj(string key,
                        paramStringProcessor psp) {

  // 1. Get object name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Call MCL api
  mclMA::observables::notice_object_unobservable(key,oname);

  // 3. Return success
  return succ("un-noticed '"+oname+"'.");
}

// ----------------------------------------- updateObs
// updateObs(str key, str oname, str value)

string command_updateObs(string key,
                         paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 1. Get defv, error if none
  if (!psp.hasMoreParams())
    return fail("missing VALUE parameter.");
  string value = psp.getNextParam();
  double defv = textFunctions::numval(value);

  // 2. Call MCL api
  mclMA::observables::update_observable(key,oname,defv);

  // 3. Return success
  return succ("updated '"+oname+"'.");
}

// ----------------------------------------- updateObjObs
// updateObjObs(str key, str oname, str fname, str value)

string command_updateObjObs(string key,
                         paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing FIELD_NAME parameter.");
  string fname = psp.getNextParam();

  // 3. Get defv, error if none
  if (!psp.hasMoreParams())
    return fail("missing VALUE parameter.");
  string value = psp.getNextParam();
  double defv = textFunctions::numval(value);

  // 4. Call MCL api
  mclMA::observables::update_observable(key,oname,fname,defv);

  // 5. Return success
  return succ("updated '"+oname+"'.");
}

// ----------------------------------------- setObsPropSelf
// setObsPropSelf(str key, str oname, str prop_key, str value)

string command_setObsPropSelf(string key,
                              paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing PROP_KEY parameter.");
  string zpkey = psp.getNextParam();
  bool sv_fail;
  int pkey = symbols::smartval_int(zpkey,&sv_fail);
  if (sv_fail)
    return fail("PROP_KEY '"+zpkey+"' not found for '"+oname+"'."); 

  // 3. Get defv, error if none
  if (!psp.hasMoreParams())
    return fail("missing PROP_VALUE parameter.");
  string value = psp.getNextParam();
  int pval = symbols::smartval_int(value,&sv_fail);
  if (sv_fail)
    return fail("PROP_VALUE '"+value+"' not found for '"+oname+"'."); 

  // 4. Call MCL api
  mclMA::observables::set_obs_prop_self(key,oname,pkey,pval);

  // 5. Return success
  return succ("set prop for '"+oname+"'.");
}

// ----------------------------------------- setObsPropObj
// setObsPropObj(str key, str oname, str fname, str prop_key, str value)

string command_setObsPropObj(string key,
                             paramStringProcessor psp) {

  // 1. Get object name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJECT_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Get field name, error if none
  if (!psp.hasMoreParams())
    return fail("missing FIELD_NAME parameter.");
  string fname = psp.getNextParam();

  // 2. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing PROP_KEY parameter.");
  string zpkey = psp.getNextParam();
  bool sv_fail;
  int pkey = symbols::smartval_int(zpkey,&sv_fail);
  if (sv_fail)
    return fail("PROP_KEY '"+zpkey+"' not found for '"+oname+"'."); 

  // 3. Get defv, error if none
  if (!psp.hasMoreParams())
    return fail("missing PROP_VALUE parameter.");
  string value = psp.getNextParam();
  int pval = textFunctions::numval(value);
  if (sv_fail)
    return fail("PROP_VALUE '"+value+"' not found for '"+oname+"'."); 

  // 4. Call MCL api
  mclMA::observables::set_obs_prop(key,oname,fname,pkey,pval);

  // 5. Return success
  return succ("set prop for '"+oname+"'.");
}

// ----------------------------------------- setObsNoisePropSelf
// setObsNoisePropSelf(str key, str oname, str prop_key)
// setObsNoisePropSelf(str key, str oname, str prop_key, double value)

string command_setObsNoisePropSelf(string key,
                                   paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Get noise profile key, error if none
  if (!psp.hasMoreParams())
    return fail("missing NOISE_PROFILE_KEY parameter.");
  bool sv_fail;
  string znpkey = psp.getNextParam();
  int npkey = symbols::smartval_int(znpkey,&sv_fail);
  if (sv_fail)
    return fail("NOISE_PROFILE_KEY '"+znpkey+"' not found for '"+oname+"'."); 

  // 3. If no arguments, use short form
  if (!psp.hasMoreParams()) {
    mclMA::observables::set_obs_noiseprofile_self(key,oname,npkey);
    return succ("set noise profile for '"+oname+"' (0 args).");
  }

  // 4. Get noise profile parameters
  string parms = psp.getNextParam();
  double nparg = textFunctions::numval(parms);

  // 5. Call MCL api
  sprintf(uLog::annotateBuffer,"noise profile parameters for %s are %d(%lf)",oname.c_str(),npkey,nparg);
  uLog::annotateFromBuffer(MCLA_DBG);
  mclMA::observables::set_obs_noiseprofile_self(key,oname,npkey,nparg);

  // 6. Return success
  return succ("set noise profile for '"+oname+"' (1 arg).");
}

// ----------------------------------------- setObsNoisePropObj
// setObsNoisePropObj(str key, str oname, str fname, str prop_key, str value)

string command_setObsNoisePropObj(string key,
                                  paramStringProcessor psp) {

  // 1. Get obserable name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSERVABLE_NAME parameter.");
  string oname = psp.getNextParam();

  // 2. Get field name, error if none
  if (!psp.hasMoreParams())
    return fail("missing FIELD_NAME parameter.");
  string fname = psp.getNextParam();

  // 3. Get noise profile key, error if none
  if (!psp.hasMoreParams())
    return fail("missing NOISE_PROFILE_KEY parameter.");
  bool sv_fail;
  string znpkey = psp.getNextParam();
  int npkey = symbols::smartval_int(znpkey,&sv_fail);
  if (sv_fail)
    return fail("NOISE_PROFILE_KEY '"+znpkey+"' not found for '"+fname+"'."); 

  // 4. If no arguments, use short form
  if (!psp.hasMoreParams()) {
    mclMA::observables::set_obj_obs_noiseprofile(key,oname,fname,npkey);
    return succ("set noise profile for '"+oname+"' (0 args).");
  }

  // 5. Get noise profile parameters
  string parms = psp.getNextParam();
  double narg = textFunctions::numval(parms);

  // 6. Call MCL api
  mclMA::observables::set_obj_obs_noiseprofile(key,oname,fname,npkey,narg);

  // 7. Return success
  return succ("set noise profile for '"+oname+"' (1 arg).");
}

// ----------------------------------------- updateObservables
// updateObservables(str key, update)

string command_updateObservables(string key,
                                 paramStringProcessor psp) {

  // 1. Get update, error if none
  if (!psp.hasMoreParams())
    return fail("no update found.");
  string update = psp.getNextParam();

  // 2. Process updae
  bool okay=false;
  string rv = process_update(key,&okay,update);

  // 3. Return success (or failure)
  if (okay)
    return succ(rv);
  else
    return fail(rv);
}

// ----------------------------------------- addObsLegalValDef
// addObsLegalValDef(str key, str objname, str obsname, double lval)
// adds legalvals to the DEFINITION

string command_addObsLegalValDef(string key,
                                 paramStringProcessor psp) {

  // 1. Get object type, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJTYPE parameter.");
  string objname = psp.getNextParam();

  // 2. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 3. Get value, error if none
  if (!psp.hasMoreParams())
    return fail("missing LEGALVAL parameter.");
  string value = psp.getNextParam();
  double lval = textFunctions::numval(value);

  // 4. Call MCL api
  mclMA::observables::add_obs_legalval_def(key,objname,obsname,lval);

  // 5. Return success (or failure)
  return succ("Added " + objname + "::" + obsname + " legal value.");
}

// ----------------------------------------- addObsLegalValObj
// addObsLegalValObj(str key, str objname)

string command_addObsLegalValObj(string key,
                                 paramStringProcessor psp) {

  // 1. Get object type, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJTYPE parameter.");
  string objname = psp.getNextParam();

  // 2. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 3. Get value, error if none
  if (!psp.hasMoreParams())
    return fail("missing LEGALVAL parameter.");
  string value = psp.getNextParam();
  double lval = textFunctions::numval(value);

  // 4. Call MCL api
  mclMA::observables::add_obs_legalval_obj(key,objname,obsname,lval);

  // 5. Return success (or failure)
  return succ("Added " + objname + "::" + obsname + " legal value.");
}

// ----------------------------------------- addObsLegalValSelf
// addObsLegalValSelf(str key, str objname)

string command_addObsLegalValSelf(string key,
                                  paramStringProcessor psp) {

  // 1. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 2. Get value, error if none
  if (!psp.hasMoreParams())
    return fail("missing LEGALVAL parameter.");
  string value = psp.getNextParam();
  double lval = textFunctions::numval(value);

  // 4. Call MCL api
  mclMA::observables::add_obs_legalval_self(key,obsname,lval);

  // 5. Return success (or failure)
  return succ("Added " + obsname + " legal value.");
}


// ----------------------------------------- addObsLegalRangeDef
// addObsLegalRangeDef(str key, str objname, str obsname, double min, double max)

string command_addObsLegalRangeDef(string key,
                                   paramStringProcessor psp) {

  // 1. Get object type, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJTYPE parameter.");
  string objname = psp.getNextParam();

  // 2. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 3. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMIN parameter.");
  string minvalue = psp.getNextParam();
  double rmin = textFunctions::numval(minvalue);

  // 4. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMAX parameter.");
  string maxvalue = psp.getNextParam();
  double rmax = textFunctions::numval(maxvalue);

  // 5. Call MCL api
  mclMA::observables::set_obs_legalrange_def(key,objname,obsname,
                                                     rmin,rmax);

  // 6. Return success (or failure)
  return succ("Added " + objname + "::" + obsname + " range.");
}

// ----------------------------------------- addObsLegalRangeObj
// addObsLegalRangeObj(str key, str objname, str obsname, double min, double max)

string command_addObsLegalRangeObj(string key,
                                   paramStringProcessor psp) {

  // 1. Get object type, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBJTYPE parameter.");
  string objname = psp.getNextParam();

  // 2. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 3. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMIN parameter.");
  string minvalue = psp.getNextParam();
  double rmin = textFunctions::numval(minvalue);

  // 4. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMAX parameter.");
  string maxvalue = psp.getNextParam();
  double rmax = textFunctions::numval(maxvalue);

  // 5. Call MCL api
  mclMA::observables::set_obs_legalrange_obj(key,objname,obsname,
                                                     rmin,rmax);

  // 6. Return success (or failure)
  return succ("Added " + objname + "::" + obsname + " range.");
}

// ----------------------------------------- addObsLegalRangeSelf
// addObsLegalRangeSelf(str key, str objname, str obsname, double min, double max)

string command_addObsLegalRangeSelf(string key,
                                    paramStringProcessor psp) {

  // 1. Get observation name, error if none
  if (!psp.hasMoreParams())
    return fail("missing OBSNAME parameter.");
  string obsname = psp.getNextParam();

  // 2. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMIN parameter.");
  string minvalue = psp.getNextParam();
  double rmin = textFunctions::numval(minvalue);

  // 3. Get min value, error if none
  if (!psp.hasMoreParams())
    return fail("missing RANGEMAX parameter.");
  string maxvalue = psp.getNextParam();
  double rmax = textFunctions::numval(maxvalue);

  // 4. Call MCL api
  mclMA::observables::set_obs_legalrange_self(key,obsname,rmin,rmax);

  // 5. Return success (or failure)
  return succ("Added " + obsname + " range.");
}

// ----------------------------------------- setREB
// setREB(str key, str reb)

string command_setREB(string key,
                      paramStringProcessor psp) {

  // 1. Get reb name, error if none
  if (!psp.hasMoreParams())
    return fail("missing Reentrant Behavor name parameter.");
  string rebname = psp.getNextParam();

  // 2. Call MCL api
  try {
    mclMA::setREB(key,rebname);
  }
  catch (MCLException e) {
    return fail(e.what());
  }

  // 3. Return success
  return succ("REB set to '"+rebname+"'.");
}

// ----------------------------------------- getREB
// getREB(str key, str reb)

string command_getREB(string key,
                      paramStringProcessor psp) {

  string rebname;

  // 1. Call MCL api
  mclMA::getREB(key,rebname);

  // 2. Return success
  return succ("REB is '"+rebname+"'.");
}

// ----------------------------------------- startMCL
// startMCL(str key)

string command_startMCL(string key,
                        paramStringProcessor psp) {

  // 1. Call MCL api
  bool prior = mclMA::startMCL(key);

  // 2. Return success
  if (prior) return succ("MCL for '"+key+"' already running.");
  else       return succ("MCL for '"+key+"' started.");
}

// ----------------------------------------- stopMCL
// stopMCL(str key)

string command_stopMCL(string key,
                       paramStringProcessor psp) {

  // 1. Call MCL api
  bool prior = mclMA::stopMCL(key);

  // 2. Return success
  if (prior) return succ("MCL for '"+key+"' stopped.");
  else       return succ("MCL for '"+key+"' already stopped.");
}

// ----------------------------------------- terminate
// terminate(str key)

string command_terminate(string key,
                         paramStringProcessor psp) {

  // 1. Use saved key if none given
  if (key.size() == 0)
    key = connected_key;
    connected_key = "";

  // 2. If key == saved key, forget saved key
  if (key == connected_key)
    connected_key = "";

  // 3. Call MCL api if we have a key
  if (key.size() > 0)
      mclMA::releaseMCL(key);

  // 4. Bring down our connection
  connected = false;

  // 5. Log the action
  uLog::annotate(ULAT_NORMAL,"[mcl_srv]:: connection closed by terminate.");

  // 6. Return success
  return succ("Terminated '" + key + "'.");
}

// ----------------------------------------- terminateWithExtremePrejudice
// terminateWithExtremePrejudice(str key)

string command_terminateWithExtremePrejudice(string key,
                                             paramStringProcessor psp) {

  // 1. Use saved key if none given
  if (key.size() == 0)
    key = connected_key;
    connected_key = "";

  // 2. If key == saved key, forget saved key
  if (key == connected_key)
    connected_key = "";

  // 3. Call MCL api if we have a key
  if (key.size() > 0)
      mclMA::releaseMCL(key);

  // 4. Bring down our connection and the server
  connected = false;
  accepting = false;

  // 5. Log the action
  uLog::annotate(ULAT_NOTGOOD,"[mcl_srv]:: server closed by terminate.");

  // 6. Return success
  return succ("Terminated '" + key + "'.");
}

// ----------------------------------------- handle_command
// cmd(key, params)

string handle_command(string command) {

  // 1. Get the command name and the command body
  string cmd = textFunctions::getFunctor(command);
  string params = textFunctions::getParameters(command);
  uLog::annotate(ULAT_ALT1,"cmd["+cmd+"] parms["+params+"]");
  paramStringProcessor psp(params);

  // 2. Catch any problems with command parsing or execution
  try {

    // 3. All command bodies start with an MCL key
    string key = psp.getNextParam();

    // 4. Call specific command handler based on command name
    //    TODO: make this a lookup in a dictionary, cmd->function
    if (cmd == "initialize")
      return command_initialize(key, psp);
    else if (cmd == "ontology") // return command_ontology(key, psp);
      throw RemovedFromAPIException("chooseOntology");
    else if (cmd == "configure")
      return command_configure(key, psp);
    else if (cmd == "newDefaultPV")
      return command_newDefaultPV(key);
    else if (cmd == "popDefaultPV")
      return command_popDefaultPV(key);
    else if (cmd == "setPropertyDefault")
      return command_setPropertyDefault(key, psp);
    else if (cmd == "setEGPropoerty")
      return command_setEGPropoerty(key, psp);
    else if (cmd == "resetDefaultPV")
      return command_newDefaultPV(key);
    else if (cmd == "registerHIA")
      return command_registerHIA(key, psp);
    else if (cmd == "signalHIA")
      return command_signalHIA(key, psp);
    else if (cmd == "declareEG")
      return command_declareEG(key, psp);
    else if (cmd == "EGabort")
      return command_EGabort(key, psp);
    else if (cmd == "EGcomplete")
      return command_EGcomplete(key, psp);
    else if (cmd == "declareExp")
      return command_declareExp(key, psp);
    else if (cmd == "declareSelfExp")
      return command_declareSelfExp(key, psp);
    else if (cmd == "declareObjExp")
      return command_declareObjExp(key, psp);
    else if (cmd == "declareDelayedExp")
      return fail("unimplemented.");
    else if (cmd == "suggestionDeclined")
      return command_suggestionDeclined(key, psp);
    else if (cmd == "suggestionImplemented")
      return command_suggestionImplemented(key, psp);
    else if (cmd == "suggestionFailed")
      return command_suggestionFailed(key, psp);
    else if (cmd == "suggestionIgnored")
      return command_suggestionIgnored(key, psp);
    else if (cmd == "provideFeedback")
      return command_provideFeedback(key, psp);
    else if (cmd == "monitor")
      return command_monitor(key, psp);
    else if (cmd == "declareObservableSelf")
      return command_declareObservableSelf(key, psp);
    else if (cmd == "declareObservableObjType")
      return command_declareObservableObjType(key, psp);
    else if (cmd == "declareObjField")
      return command_declareObjField(key, psp);
    else if (cmd == "noticeObj")
      return command_noticeObj(key, psp);
    else if (cmd == "unnoticeObj")
      return command_unnoticeObj(key, psp);
    else if (cmd == "updateObs")
      return command_updateObs(key, psp);
    else if (cmd == "updateObjObs")
      return command_updateObjObs(key, psp);
    else if (cmd == "getOV")
      return succ(mclMA::observables::ov_as_string(key));
    else if (cmd == "setObsPropSelf")
      return command_setObsPropSelf(key, psp);
    else if (cmd == "setObsPropObj")
      return command_setObsPropObj(key, psp);
    else if (cmd == "setObsNoisePropSelf")
      return command_setObsNoisePropSelf(key, psp);
    else if (cmd == "setObsNoisePropObj")
      return command_setObsNoisePropObj(key, psp);
    else if (cmd == "updateObservables")
      return command_updateObservables(key, psp);
    else if (cmd == "addObsLegalValDef")
      return command_addObsLegalValDef(key, psp);
    else if (cmd == "addObsLegalValObj")
      return command_addObsLegalValObj(key, psp);
    else if (cmd == "addObsLegalValSelf")
      return command_addObsLegalValSelf(key, psp);
    else if (cmd == "setObsLegalRangeDef")
      return command_addObsLegalRangeDef(key, psp);
    else if (cmd == "setObsLegalRangeObj")
      return command_addObsLegalRangeObj(key, psp);
    else if (cmd == "setObsLegalRangeSelf")
      return command_addObsLegalRangeSelf(key, psp);
    else if (cmd == "setREB")
      return command_setREB(key, psp);
    else if (cmd == "getREB")
      return command_getREB(key, psp);
    else if (cmd == "startMCL")
      return command_startMCL(key, psp);
    else if (cmd == "stopMCL")
      return command_stopMCL(key, psp);
    else if (cmd == "terminate")
      return command_terminate(key, psp);
    else if (cmd == "terminateWithExtremePrejudice")
      return command_terminateWithExtremePrejudice(key, psp);
    else if (cmd == "st") {
      psp.reset();
      string q = psp.getNextParam();
      cout << "translate '"+command+"' -> "+translator.translate_string(command) << endl;
      return "ok("+translator.translate_string(command)+")";
    }
    else
      return fail("unknown command '"+ cmd + "'.");

  // 5. Report exceptions as command failures
  }
  catch (exception e) {
    return fail(e.what());
  }

  // 6. Should never reach here
  return fail("internal error.");
}

// ----------------------------------------- process_update_into
string process_update_into(string key,bool* success,
                           mclMA::observables::update& into,string update) {
  keywordParamMachine kwm(update);
  while (kwm.hasMoreParams()) {
    string kwp   = kwm.getNextKWP();
    string oname = kwm.keyword_of(kwp);
    string oval  = kwm.value_of(kwp);
    // oval = translator.translate(oval,NULL);
    double doval = textFunctions::dubval(oval);
    cout << "  ass~> " << oname << " := " << doval << endl;
    if (oname.find(".") != string::npos) {
      string oostr = textFunctions::substChar(oname, '.', ' ');
      tokenMachine ootm(oostr);
      string o1 = ootm.nextToken();
      string o2 = ootm.nextToken();
      into.set_update(o1,o2,doval);
    }
    else {
      into.set_update(oname,doval);
    }
  }
  if (success) *success=true;
  return "update success.";
}

// ----------------------------------------- process_update
string process_update(string key,
                      bool* success,
                      string update) {
  keywordParamMachine kwm(update);
  while (kwm.hasMoreParams()) {
    string kwp   = kwm.getNextKWP();
    string oname = kwm.keyword_of(kwp);
    string oval  = kwm.value_of(kwp);
    // oval = translator.translate(oval,NULL);
    double doval = textFunctions::dubval(oval);
    cout << "  ass~> " << oname << " := " << doval << endl;
    if (oname.find(".") != string::npos) {
      string oostr = textFunctions::substChar(oname, '.', ' ');
      tokenMachine ootm(oostr);
      string o1 = ootm.nextToken();
      string o2 = ootm.nextToken();
      mclMA::observables::update_observable(key,o1,o2,doval);
    }
    else {
      mclMA::observables::update_observable(key,oname,doval);
    }
  }
  if (success) *success=true;
  return "update success.";
}


// ----------------------------------------- rv2string
string rv2string(responseVector& rv) {
  string r="[";
  for (int i=0;i<(int)rv.size();i++) {
    if (i!=0) r+=",";
    r+=rv[i]->to_string();
  }
  return r+"]";
}

// ----------------------------------------- end server.cc
