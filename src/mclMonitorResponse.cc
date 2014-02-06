#include "mclMonitorResponse.h"
#include "mcl_symbols.h"
#include <stdio.h>

string crc_names[CRC_MAX]=
  { "ignore violation", "do nothing",
    "try again",
    "solicit help from user", "relinquish control to user",
    "run sensor diagnostic", "run effector diagnostic",
    "reset a sensor","reset an effector",
    "activate learning", "adjust parameters", "rebuild models",
    "revisit assumptions", "amend controller",
    "revise expectations", "swap algorithm", "change hlc",
  };

string respName(pkType crc) {
  return crc_names[crc_offset_base_zero(crc)]; 
}

string mclMonitorResponse::to_string() {
  char buff[1024];
  string actiontext,aborttext;
  if (recommendAbort()) aborttext = "true"; else aborttext="false";
  if (requiresAction()) actiontext = "true"; else actiontext="false";
  sprintf(buff,"response(type=%s,ref=0x%08lx,action=%s,abort=%s,text=\"%s\")",rclass().c_str(),referenceCode(),actiontext.c_str(),aborttext.c_str(),responseText().c_str());
  return buff;
}

string mclMonitorCorrectiveResponse::to_string() {
  char buff[1024];
  bool none,mul;
  string respcode = metacog::symbols::reverse_lookup("crc",responseCode(),&none,&mul);
  if (none) {
    sprintf(buff,"unknown_code_%d",responseCode());
    respcode=(string)buff;
  }
  string actiontext,aborttext;
  if (recommendAbort()) aborttext = "true"; else aborttext="false";
  if (requiresAction()) actiontext = "true"; else actiontext="false";
  sprintf(buff,
	  "response(type=%s,ref=0x%08lx,code=%s,action=%s,abort=%s,text=\"%s\")",
	  rclass().c_str(),
	  referenceCode(),
	  respcode.c_str(),
	  actiontext.c_str(),
	  aborttext.c_str(),
	  responseText().c_str());
  return buff;
}
