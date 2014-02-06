#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <stdlib.h>
#include "symbols.h"
#include "../include/umbc/text_utils.h"
#include "../include/umbc/token_machine.h"

using namespace std;
using namespace umbc;

#define PREAMBLE_H_NAME "/utils/symbols.header.start"
#define PROLOGUE_H_NAME "/utils/symbols.header.end"
#define PREAMBLE_S_NAME "/utils/symbols.src.start"
#define PROLOGUE_S_NAME "/utils/symbols.src.end"
#define DEFS_FILE_NAME "/utils/symbols.def"
#define HEADER_OUT_NAME "/api/mcl/mcl_symbols.h"
#define SRC_OUT_NAME "/src/mcl_symbols.cc"

map<string,int> __SYMBOLS_PREFIX_TABLE;

void define_prefix(string prefix) {
  __SYMBOLS_PREFIX_TABLE[prefix]=0;
}
void define_prefix(string prefix,int start) {
  __SYMBOLS_PREFIX_TABLE[prefix]=start;
}
int inc_prefix_count(string prefix) {
  int rv = __SYMBOLS_PREFIX_TABLE[prefix];
  __SYMBOLS_PREFIX_TABLE[prefix] = rv+1;
  return rv;
}  
int prefix_size(string prefix) {
  return __SYMBOLS_PREFIX_TABLE[prefix];
}  

void process_line(tokenMachine& tm,ofstream& header,ofstream& source,int linenum) {
  string directive = tm.nextToken();
  source << "  ";
  if (directive == "prefix") {
    if (tm.moreTokens()) {
      string prefix = tm.nextToken();
      if (!tm.moreTokens()) 
	define_prefix(prefix);
      else {
	string startStr = tm.nextToken();
	int start = textFunctions::numval(startStr);
	define_prefix(prefix,start);
      }
    }
    else
      { cerr << "line " << linenum << ": missing NAME in PREFIX, exiting..." << endl; exit(-1); }
  }
  else if (directive == "resume") {
    if (!tm.moreTokens()) 
      { cerr << "line " << linenum << ": missing NEWPREFIX in RESUME, exiting..." << endl; exit(-1); }
    else {
      string newprefix = tm.nextToken();
      if (!tm.moreTokens())
	{ cerr << "line " << linenum << ": missing OLDPREFIX in RESUME, exiting..." << endl; exit(-1); }
      else {
	string oldprefix = tm.nextToken();
	define_prefix(newprefix,prefix_size(oldprefix));
      }
    }
  }
  else if (directive == "psym") {
    if (!tm.moreTokens()) 
      { cerr << "line " << linenum << ": missing SYMBOL in PSYM, exiting..." << endl; exit(-1); }
    else {
      string symbol = tm.nextToken();
      if (!tm.moreTokens())
	{ cerr << "line " << linenum << ": missing PREFIX in PSYM, exiting..." << endl; exit(-1); }
      else {
	string prefix = tm.nextToken();
	int val = inc_prefix_count(prefix);
	header << "#define " << prefix << "_" << symbol << " " << val << endl;
	string text_key = textFunctions::str2lower(prefix+"_"+symbol);
	source << "symbol_def"
	       << "(\"" << text_key << "\"," 
	       << prefix << "_" << symbol << ");" << endl;
      }
    }
  }
  else if (directive == "size") {
    if (!tm.moreTokens()) 
      { cerr << "line " << linenum << ": missing SYMBOL in SIZE, exiting..." << endl; exit(-1); }
    else {
      string symbol = tm.nextToken();
      if (!tm.moreTokens())
	{ cerr << "line " << linenum << ": missing PREFIX in PSYM, exiting..." << endl; exit(-1); }
      else {
	string prefix = tm.nextToken();
	int val = prefix_size(prefix);
	header << "#define " << prefix << "_" << symbol << " " << val << endl;
	string text_key = textFunctions::str2lower(prefix+"_"+symbol);
	// assumes this will be an int when the variable is named
	// should do some error checking here instead...
	source << "symbol_def"
	       << "(\"" << text_key << "\"," 
	       << prefix << "_" << symbol << ");" << endl;
      }
    }	
  }
  else if (directive == "header") {
    header << tm.rest() << endl;
  }
  else if (directive == "def") {
    if (!tm.moreTokens()) 
      { cerr << "line " << linenum << ": missing SYMBOL in DEF, exiting..." << endl; exit(-1); }
    else {
      string symbol = tm.nextToken();
      if (!tm.moreTokens())
	{ cerr << "line " << linenum << ": missing PREFIX in PSYM, exiting..." << endl; exit(-1); }
      else {
	string assign = tm.nextToken();
	header << "#define " << symbol << " " << assign << tm.rest() << endl;
	string text_key = textFunctions::str2lower(symbol);
	// assumes this will be an int when the variable is named
	// should do some error checking here instead...
	source << "symbol_def"
	       << "(\"" << text_key << "\"," 
	       << symbol << ");" << endl;
      }
    }
  }
  else if (directive == "") {
    header << endl;
    source << endl;
  }
  else {
    cerr << "unknown directive '" << directive << "', exiting..." << endl;
    exit(-1);
  }
}

int main(int argc, char ** argv) {
  if (argc < 2) {
    cerr << "usage: " << argv[0] << " mcl_build_dir" << endl;
    return 1;
  }
  else {
    string basedir = argv[1];

    string head_fn = basedir+HEADER_OUT_NAME;
    string src_fn  = basedir+SRC_OUT_NAME;

    ofstream header(head_fn.c_str(),ios::out);

    if (!header.is_open()) {
      cerr << "could not open '" << head_fn << "'" << endl;
      return 1;
    }
    else {
      
      ofstream source(src_fn.c_str(),ios::out);

      if (!source.is_open()) {
	cerr << "could not open '" << src_fn << "'" << endl;
	return 1;
      }
      else {

	// write preamble
	{
	  string h_pre_fn = basedir + PREAMBLE_H_NAME;
	  string h_preamble = textFunctions::file2string(h_pre_fn,"",false);
	  header << h_preamble << endl;
	}

	{
	  string s_pre_fn = basedir + PREAMBLE_S_NAME;
	  string s_preamble = textFunctions::file2string(s_pre_fn,"",false);
	  source << s_preamble << endl;
	}
	
	string defs_fn = argv[1]; defs_fn += DEFS_FILE_NAME;
	ifstream defstream(defs_fn.c_str());
	int linenum = 0;
	if (!defstream.is_open()) {
	  cerr << "could not open '" << defs_fn << "'" << endl;
	  return 1;
	}
	else {
	  string line;
	  source << "metacog::symbols::symbol_table::symbol_table(string name) :tname(name) {" << endl;
	  while (!defstream.eof()) {
	    linenum++;
	    getline (defstream,line);
	    if (line.substr(0,2)=="/*") {
	      header << line << endl;
	      source << line << endl;
	    }
	    else {
	      tokenMachine tm(line);
	      process_line(tm,header,source,linenum);
	    }
	  }
	  source << "}" << endl << endl;
	}

	{
	  string h_pro_fn = basedir + PROLOGUE_H_NAME;
	  string h_prologue = textFunctions::file2string(h_pro_fn,"",false);
	  header << h_prologue << endl;
	}

	{
	  string s_pro_fn = basedir + PROLOGUE_S_NAME;
	  string s_prologue = textFunctions::file2string(s_pro_fn,"",false);
	  source << s_prologue << endl;
	}
      
      }
      header.close();
    }
  }
  return 0;
}
