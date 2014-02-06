#include <stdio.h>
#include "include/symbol_translator.h"

string symbol_translator::translate(string& symbol,bool* translated) {
  if (symbol[0] == indicator) {
    if (translated != NULL) *translated = true;
    if (symbol_translations.count(symbol)==0) {
      int tc = counter++;
      char cnt[50];
      sprintf(cnt,"%d",tc);
      symbol_translations[symbol]=cnt;
    }
    return symbol_translations[symbol];
  }
  else {
    if (translated != NULL) *translated = false;
    return symbol;
  }
}

bool symbol_translator::terminates(char k) {
  return (isspace(k) || (k == ',') || (k == '(') || (k == '[') || (k == '{') ||
	  (k == ')') || (k == ']') || (k == '}'));
}

string symbol_translator::translate_string(string& input) {
  string output = "";
  string symbol = "";
  bool   syming = false;
  for (int i = 0;i<input.size();i++) {
    if (syming) {
      if (terminates(input[i])) {
	output+=translate(symbol,NULL);
	output+=input[i];
	syming = false;
	symbol = "";
      }
      else symbol+=input[i];
    }
    else if (input[i] == indicator) {
      syming = true;
      symbol+=input[i];
    }
    else output+=input[i];
  }
  if (syming) output+=translate(symbol,NULL);
  return output;
}

string translatedParamProcessor::nextToken(symbol_translator& trans) {
  string q = paramStringProcessor::nextToken();
  return trans.translate(q,NULL);
}
