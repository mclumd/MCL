#include "mcl_symbols.h"
#include "umbc/text_utils.h"
#include <string>
#include <iostream>
#include <string.h>
#include <assert.h>

using namespace std;
using namespace metacog;

int chk_symbol_intval(char *buff) {
  bool exists; 
  int rv = symbols::global_symbol_intval(buff,&exists);
  if (exists)
    cout << "intval(" << buff << ") = " << rv << endl;
  else
    cout << "'" << buff << "' has no value." << endl;
  return rv;
}

int chk_smartval_int(char *buff) {
  bool error; 
  int rv = symbols::smartval_int(buff,&error);
  if (!error)
    cout << "smartval_int(" << buff << ") = " << rv << endl;
  else
    cout << "smartval_int(" << buff << ") had an error." << endl;
  return rv;
}

string scNodeLookup(spvType scCode);

int main(int argc,char ** argv) {
  char buff[128];

  assert(chk_symbol_intval("pci_declarative") == PCI_DECLARATIVE);
  assert(chk_symbol_intval("crc_ignore") == CRC_IGNORE);
  assert(chk_symbol_intval("crc_noop") == CRC_NOOP);
  assert(chk_symbol_intval("crc_adj_params") == CRC_ADJ_PARAMS);
  // assert(chk_symbol_intval("crc_get_a_real_job") == 0);

  assert(chk_smartval_int("pci_declarative") == PCI_DECLARATIVE);
  assert(chk_smartval_int("crc_ignore") == CRC_IGNORE);
  assert(chk_smartval_int("crc_noop") == CRC_NOOP);
  assert(chk_smartval_int("crc_adj_params") == CRC_ADJ_PARAMS);
  // assert(chk_smartval_int("crc_get_a_real_job") == 0);
  assert(chk_smartval_int("123") == 123);
  

  while (1) {
    cout << "testSYM> ";
    cin.getline(buff,127);
    if (strcmp(buff,"exit") == 0)
       break;
    chk_symbol_intval(buff);
    chk_smartval_int(buff);
    cout << "SCNODE: " << scNodeLookup(umbc::textFunctions::numval(buff)) << endl;
  }

  // metacog::symbols::define_mcl_symbols();
  // cout << "value of 'pci_declarative' is " 
  //   << symbols::global_symbol_intval("pci_declarative") << endl;
}
