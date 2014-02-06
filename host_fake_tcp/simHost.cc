#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <math.h>

#include <map>

#include "umbc/logger.h"
#include "umbc/declarations.h"
#include "umbc/text_utils.h"
#include "umbc/token_machine.h"

using namespace std;
using namespace umbc;

#define BASE_EXP_GRP 0x11110000

#define FH_NUM_SENSORS 5
#define FH_MCL_KEY      "fakeHost"
#define FH_MCL_CONFIG   "fakeHost"
#define FH_MCL_ONTOLOGY "hostFake"
#define FH_S0_NOISE_COEFF 0.0
#define FH_S1_NOISE_COEFF 0.0
#define FH_FV_FAILMOD     20

int mclFD;
const char* nv[FH_NUM_SENSORS] = 
  { "noisyCos","noisySin","noisyConst","randomWalk","cos+rw" };
float  sv[FH_NUM_SENSORS];
string mcl_update_string = "";

// explanation of sensors:
// 0: noisy cos, must stay below 1
// 1: noisy sin, must stay above -1
// 2: noisy const, must stay near 0
// 3: random walk
// 4: noisy cos + sv[3]

// explanation of failure switches:
// 0: sensor break on 0 -- causes to drift upward, no longer hinged to cos
// 1: sensor break on 1 -- causes to drift downward, no longer hinged to sin
// 2: model error changing constant of random walk (causes drift)
// 3: control error that turns off limiter (allows sensor 4 to drift)

enum fvNameType { FVI_S0_BRK, FVI_S1_BRK, FVI_CONST_MDL_BRK, FVI_LIMITER_BRK,
		  FH_NUM_FAILURE_SWITCHES };
bool   fv[FH_NUM_FAILURE_SWITCHES] = {false, false, false, false};

string tell_mcl(string cmd) {
  int n=0;
  cout << " >> sending '" << cmd << "'" << endl;
  n = write(mclFD,cmd.c_str(),cmd.size());
  if (n < 0) {
    return "ERROR writing to socket";
  }
  char buffer[1024];
  bzero(buffer,1024);
  n = read(mclFD,buffer,1023);
  if (n < 0) 
    return "ERROR reading from socket";
  else {
    buffer[n-1]='\0';
    if (strncmp(buffer,"fail",4) == 0)
      uLog::annotate(UMBCLOG_ERROR," returned '" + (string)buffer + "'");
    else uLog::annotate(UMBCLOG_MSG," returned '" + (string)buffer + "'");
    return buffer;
  }
}

void initMCL() {
  char commbuff[512];
  sprintf(commbuff,"initialize(%s,8)",FH_MCL_KEY);
  tell_mcl(commbuff);
  sprintf(commbuff,"configure(%s,%s)",FH_MCL_KEY,FH_MCL_CONFIG);
  tell_mcl(commbuff);
  sprintf(commbuff,"ontology(%s,%s)",FH_MCL_KEY,FH_MCL_ONTOLOGY);
  tell_mcl(commbuff);
}

void runLimiter() {
  if (sv[4] < -1) sv[4]=-1;
  if (sv[4] >  1) sv[4]= 1;
}

void updateSensors() {
  static int c=0;
  // noisy cos, breakout high
  if (fv[FVI_S0_BRK]) // failure causes drift up
    sv[0]+=0.1*((float)rand()/(float)RAND_MAX)+0.05;
  else       // proper operation
    sv[0]=cosf((float)c*3.14/(float)100) + 
      (FH_S0_NOISE_COEFF * (((float)rand()/(float)RAND_MAX)-0.5));

  // noisy sin, breakout low
  if (fv[FVI_S1_BRK]) // failure causes drift down
    sv[1]+=0.1*((float)rand()/(float)RAND_MAX)-0.02;
  else       // normal operation 
    sv[1]=sinf((float)c*3.14/(float)100) + 
      (FH_S1_NOISE_COEFF * (((float)rand()/(float)RAND_MAX)-0.5));

  // noisy const, maintain val
  static float nc_driftval = 0.02;
  if (nc_driftval > 0.2) nc_driftval=0.01;
  if (fv[FVI_CONST_MDL_BRK]) { // failure causes drift
    nc_driftval*=1.05;
    sv[2]=(((double)rand()/(double)RAND_MAX)*0.20-0.10)+nc_driftval;
  }
  else sv[2]=(((double)rand()/(double)RAND_MAX)*0.20-0.10);

  // random walk
  float RWM = 0.1;
  float RWB = -0.05;
  sv[3]+=RWM*((float)rand()/(float)RAND_MAX)+RWB;

  // cos+nc
  sv[4]=cosf((float)c*3.14/float(200))+sv[2];
  c++;

  if (!fv[FVI_LIMITER_BRK]) runLimiter();

  mcl_update_string="{";
  for ( int i=0 ; i<FH_NUM_SENSORS ; i++ ) {
    char buff[256];
    bzero(buff,255);
    sprintf(buff,"%s=%.2f",nv[i],sv[i]);
    mcl_update_string+=buff;
    if (i < FH_NUM_SENSORS-1)
      mcl_update_string+=",";
  }
  mcl_update_string+="}";
}

void sXFix();
bool sXBroken();

void doSensorDiag(unsigned long ref) {
  char commbuff[512];
  sprintf(commbuff,"provideFeedback(%s,%c,%lx)",FH_MCL_KEY,(sXBroken() ? 't' : 'f'),ref);
  tell_mcl(commbuff);
}

void doSensorReset(unsigned long ref) {
  sXFix();
  char commbuff[512];
  sprintf(commbuff,"suggestionImplemented(%s,%lx)",FH_MCL_KEY,ref);
  tell_mcl(commbuff);
}

void doSugg(string sugg) {
  cout << "wanna do " << sugg << endl;
  string f = textFunctions::getFunctor(sugg);
  string p = textFunctions::getParameters(sugg);
  if (f != "response")
    cerr << " sugg from MCL not a response (" << f << ")?" << endl;
  else {
    keywordParamMachine rikwpm(p,tokenMachine::DELIMITER_PARENS);
    unsigned long ref = textFunctions::longval(rikwpm.getKWV("ref"));
    string rcode = rikwpm.getKWV("code");
    string rtype = rikwpm.getKWV("type");
    cout << " ~ ref  is " << hex << ref << endl;
    cout << " ~ code is " << rcode << endl;
    cout << " ~ type is " << rtype << endl;
    if (rtype == "suggestion") {
      if (rcode == "crc_sensor_diag") 
	doSensorDiag(ref);
      else if (rcode == "crc_sensor_reset")
	doSensorReset(ref);
      else {
	uLog::annotate(UMBCLOG_ERROR,"CODE "+rcode+" UNHANDLED!!");
	char commbuff[512];
	sprintf(commbuff,"suggestionIgnored(%s,%lx)",FH_MCL_KEY,ref);
	tell_mcl(commbuff);
      }
    }
    else if (rtype == "optionsExhausted") {
      // * ignore without telling MCL
      //
      // char commbuff[512];
      // sprintf(commbuff,"suggestionIgnored(%s,%lx)",FH_MCL_KEY,ref);
      // tell_mcl(commbuff);
    }
    else {
      uLog::annotate(UMBCLOG_ERROR,"TYPE "+rtype+" UNHANDLED!!");
      // * ignore without telling MCL
      //
      // char commbuff[512];
      // sprintf(commbuff,"suggestionIgnored(%s,%lx)",FH_MCL_KEY,ref);
      // tell_mcl(commbuff);
    }
  }
}

void doSuggs(string okR) {
  listStringProcessor pstr(textFunctions::trimParens(textFunctions::getParameters(okR)),
			   tokenMachine::DELIMITER_BRACKETS);
  while (pstr.hasMoreItems()) {
    doSugg(pstr.getNextItem());
  }
}

void limiterBreak() { fv[FVI_LIMITER_BRK] = true; }
void limiterFix()   { fv[FVI_LIMITER_BRK] = false; }
bool limiterBroken(){ return fv[FVI_LIMITER_BRK]; }
void s0Break()      { fv[FVI_S0_BRK] = true; }
void s1Break()      { fv[FVI_S1_BRK] = true; }
void sXFix()        { fv[FVI_S0_BRK] = false; fv[FVI_S1_BRK] = false; }
bool sXBroken()     { return (fv[FVI_S0_BRK] || fv[FVI_S1_BRK]); }
void conModelBreak(){ fv[FVI_CONST_MDL_BRK] = true; }
void conModelFix()  { fv[FVI_CONST_MDL_BRK] = false; }
bool conModBroken() { return fv[FVI_CONST_MDL_BRK]; }
string FVAS() {
  string rv="[ ";
  for (int i=0;i<FH_NUM_FAILURE_SWITCHES;i++) {
    if (fv[i]) rv+="t "; else rv+="f ";    
  }
  rv+="]";
  return rv;
}

void maybeFAIL(int k) {
  if ((k % FH_FV_FAILMOD) == 0) {
    int findex = (k / FH_FV_FAILMOD) % FH_NUM_FAILURE_SWITCHES;
    uLog::annotate(UMBCLOG_WARNING,"FAILURE OCCURRED.");
    switch (findex) {
      case FVI_S0_BRK:
	s0Break(); break;
      case FVI_S1_BRK:
	s1Break(); break;
      case FVI_CONST_MDL_BRK:
	conModelBreak(); break;
      case FVI_LIMITER_BRK:
	limiterBreak(); break;
      default:
	uLog::annotate(UMBCLOG_ERROR," illegal break out of range.");
	break;
      }
  }
  uLog::annotate(UMBCLOG_DBG,"FV="+FVAS());
}

string monitor(string key) {
  string sv="monitor("+key+","+mcl_update_string+")";
  string rv=tell_mcl(sv);
  doSuggs(rv);
  return rv;
}

void declare_self(string key,string name) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"declareObservableSelf(%s,%s)",key.c_str(),name.c_str());
  tell_mcl(commbuff);
}

void set_prop_self(string key,string name,string prop,string value) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"setObsPropSelf(%s,%s,%s,%s)",
	  key.c_str(),name.c_str(),prop.c_str(),value.c_str());
  tell_mcl(commbuff);
}

void set_np_self(string key,string name,string profile,double param) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"setObsNoisePropSelf(%s,%s,%s,%.4lf)",
	  key.c_str(),name.c_str(),profile.c_str(),param);
  tell_mcl(commbuff);
}

void declare_eg(string key,unsigned int group) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"declareEG(%s,0x%8x)",
	  key.c_str(),group);
  tell_mcl(commbuff);
}

void declare_exp(string key,unsigned int grp,
		 const char* ectype,double arg) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"declareExp(%s,0x%8x,%s,%.3lf)",
	  key.c_str(),grp,ectype,arg);
  tell_mcl(commbuff);
}

void declare_sexp(string key,unsigned int grp,const char* sname,
		 const char* ectype,double arg) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"declareSelfExp(%s,0x%8x,%s,%s,%.3lf)",
	  key.c_str(),grp,sname,ectype,arg);
  tell_mcl(commbuff);
}

void registerHIA(string key,string HIA,string activate) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"registerHIA(%s,%s,%s)",
	  key.c_str(),HIA.c_str(),activate.c_str());
  tell_mcl(commbuff);

}

void set_prop(string key,string prop,string value) {
  char commbuff[1024];
  bzero(commbuff,1024);
  sprintf(commbuff,"setPropertyDefault(%s,%s,%s)",
	  key.c_str(),prop.c_str(),value.c_str());
  tell_mcl(commbuff);

}

void registerSensors() {

  // noisy cos
  declare_self(FH_MCL_KEY,nv[0]);
  set_prop_self(FH_MCL_KEY,nv[0],"prop_sclass","sc_resource");
  set_prop_self(FH_MCL_KEY,nv[0],"prop_dt","dt_rational");
  set_np_self(FH_MCL_KEY,nv[0],"mcl_np_uniform",0.1);

  // noisy sin
  declare_self(FH_MCL_KEY,nv[1]);
  set_prop_self(FH_MCL_KEY,nv[1],"prop_sclass","sc_state");
  set_prop_self(FH_MCL_KEY,nv[1],"prop_dt","dt_rational");
  set_np_self(FH_MCL_KEY,nv[1],"mcl_np_uniform",0.1);

  // noisy const
  declare_self(FH_MCL_KEY,nv[2]);
  set_prop_self(FH_MCL_KEY,nv[2],"prop_sclass","sc_ambient");
  set_prop_self(FH_MCL_KEY,nv[2],"prop_dt","dt_rational");
  set_np_self(FH_MCL_KEY,nv[2],"mcl_np_uniform",0.1);
  
  // random walk
  declare_self(FH_MCL_KEY,nv[3]);
  set_prop_self(FH_MCL_KEY,nv[3],"prop_sclass","sc_spatial");
  set_prop_self(FH_MCL_KEY,nv[3],"prop_dt","dt_rational");
  // set_np_self(FH_MCL_KEY,nv[3],"mcl_np_uniform",0.1);
  
  // cos+rw
  declare_self(FH_MCL_KEY,nv[4]);
  set_prop_self(FH_MCL_KEY,nv[4],"prop_sclass","sc_unspec");
  set_prop_self(FH_MCL_KEY,nv[4],"prop_dt","dt_rational");
  // set_np_self(FH_MCL_KEY,nv[3],"mcl_np_uniform",0.1);

//   for (int i=0; i<FH_NUM_SENSORS; i++) 
//     mclMA::observables::dump_obs_self(FH_MCL_KEY,nv[i]);

//   mclMA::dumpNPT();

}

void declareExps() {

  declare_eg(FH_MCL_KEY,BASE_EXP_GRP);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[0],"ec_stayunder",1.01);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[1],"ec_stayunder",1.01);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[0],"ec_stayover",-1.01);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[1],"ec_stayover",-1.01);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[2],"ec_maintainvalue",0);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[4],"ec_stayunder",1.01);
  declare_sexp(FH_MCL_KEY,BASE_EXP_GRP,nv[4],"ec_stayover",-1.01);

}

void registerHIAs() {
  registerHIA(FH_MCL_KEY,"inconvenientTruthException","effectorError");
}

void setPV() {
  
  set_prop(FH_MCL_KEY,"pci_intentional","pc_yes");
  set_prop(FH_MCL_KEY,"pci_parameterized","pc_yes");
  set_prop(FH_MCL_KEY,"pci_sensors_can_fail","pc_yes");	
  set_prop(FH_MCL_KEY,"pci_effectors_can_fail","pc_yes");

}

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       return -1;
    }
    portno = atoi(argv[2]);
    mclFD = socket(AF_INET, SOCK_STREAM, 0);
    if (mclFD < 0) {
      cerr << "ERROR opening socket" << endl;
      return -1;
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
      cerr << "ERROR, no such host" << endl;
      return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serv_addr.sin_addr.s_addr,
	  server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(mclFD,(const struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
      cerr << "ERROR connecting" << endl;
      return -1;
    }

    initMCL();
    registerSensors();
    setPV();
    declareExps();
    registerHIAs();

    char b[32];
    cout << "press return to start.";
    cin.getline(b,31);

    int cntr=0;
    while (true) {
      maybeFAIL(cntr++);
      updateSensors();
      monitor(FH_MCL_KEY);
      usleep(250000);
    }
    
    return 0;

}
