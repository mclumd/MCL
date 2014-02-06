#ifndef SRVR_SYM_TRANS_H
#define SRVR_SYM_TRANS_H

#include <string>
#include <map>

#include "../../include/umbc/token_machine.h"

using namespace std;

class symbol_translator {

 public:
  char indicator;
  symbol_translator(char symbol_indicator) : indicator(symbol_indicator),counter(0) {};
  string translate(string& symbol,bool* translated);
  string translate_string(string& symbol);

 private:
  unsigned long int counter;
  map<string,string> symbol_translations;

  bool terminates(char k);

};

class translatedParamProcessor : public umbc::paramStringProcessor {
 public:
  translatedParamProcessor(string plist);
  virtual string nextToken(symbol_translator& trans);
 private:
  
};

#endif
