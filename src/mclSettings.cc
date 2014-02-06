#include "mclSettings.h"

#include <iostream>

bool mclSettings::quiet=false;
bool mclSettings::annotate=true;
bool mclSettings::debug=false;
bool mclSettings::logFrames=false;
bool mclSettings::autoConfigCPTs=true;

class pval {
public:
  pval(int v) : intV(v) {
    boolV = (v!=0);
    char q[32];
    sprintf(q,"%d",v);
    stringV = q;
  };
  pval(string v) : stringV(v) {
    intV = (int)strtol(v.c_str(),NULL,0);
    boolV = (v=="t");
  };
  pval(bool v) : boolV(v) {
    if (v) stringV = "t"; else stringV = "f";
    if (v) intV = 1; else intV = 0;
  };

  int intV;
  bool boolV;
  string stringV;
  
};

map<string,pval*> gs_map;

void mclSettings::setSysProperty(string name, int val) {
  gs_map[name]=new pval(val);
}

void mclSettings::setSysProperty(string name, bool val) {
  gs_map[name]=new pval(val);
}

void mclSettings::setSysProperty(string name, string val) {
  gs_map[name]=new pval(val);
}

// def == default
int mclSettings::getSysPropertyInt(string n,int def) {
  if (gs_map[n]==NULL) {
    gs_map[n]=new pval(def);
    return def;
  }
  else 
    return gs_map[n]->intV;
}

bool mclSettings::getSysPropertyBool(string n,bool def) {
  if (gs_map[n]==NULL) {
    gs_map[n]=new pval(def);
    return def;
  }
  else 
    return gs_map[n]->boolV;
}

string mclSettings::getSysPropertyString(string n,string def) {
    if (gs_map[n]==NULL) {
    gs_map[n]=new pval(def);
    return def;
  }
  else 
    return gs_map[n]->stringV;
}

void mclSettings::args2SysProperties(int argc, char** args) {
  int cnt=0,acnt=0;
  while (cnt < argc) {
    if (strncmp(args[cnt],"--",2)==0) {
      // it's a keyword
      char* k = args[cnt];
      k++; k++;
      string q = k;
      cnt++;
      if (cnt == argc) {
	cerr << "malformed argument list; " << q << " has no value." << endl;
      }
      else {
	string q2 = args[cnt];
	cout << q << "=" << q2 << endl;
	setSysProperty(q,q2);
	cnt++;
      }
    }
    else {
      // stray argument
      char a[8];
      sprintf(a,"arg%d",acnt);
      string q=args[cnt];
      string q2=a;
      setSysProperty(q,q2);
      acnt++;
      cnt++;
    }
  }
}
