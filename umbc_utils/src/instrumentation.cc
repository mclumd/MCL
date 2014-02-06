#include "instrumentation.h"
#include "declarations.h"
#include "settings.h"
#include "logger.h"
#include "text_utils.h"
#include "file_utils.h"
#include <time.h>
#include <limits.h>
#include <float.h>
#include <fstream>

namespace umbci = umbc::instrumentation;

/***********************************************************
 ** STATIC FUNCTIONALITY / FACTORY                        **
 ***********************************************************/

umbci::dataset *umbci::make_dataset(string const &filename,
				    instrumentation_format_t format,
				    int creation_mode) {
  switch (format) {
  case umbci::CSV : {
    umbci::csv_dataset *rv = new csv_dataset(filename,creation_mode);
    return rv;
  }
  case umbci::GNUPLOT : {
    umbci::gnuplot_dataset *rv = new gnuplot_dataset(filename,creation_mode);
    return rv;
  }
  case umbci::ARFF : {
    umbci::arff_dataset *rv = new arff_dataset(filename,creation_mode);
    return rv;
  }
  }
  return NULL;
}

umbci::dataset *umbci::make_dataset(string const &filename,int creation_mode) {
  umbci::instrumentation_format_t fmt = umbci::detect_format(filename);
  return make_dataset(filename,fmt,creation_mode);
}

umbci::instrumentation_format_t umbci::detect_format(string const &filename) {
  if (umbc::textFunctions::endswith(filename,".csv"))
    return umbci::CSV;
  else if (umbc::textFunctions::endswith(filename,".arff"))
    return umbci::ARFF;
  else if (umbc::textFunctions::endswith(filename,".data") ||
	   umbc::textFunctions::endswith(filename,".out"))
    return umbci::GNUPLOT;
  else {
    string cfg_val = 
      settings::getSysPropertyString("utils.instrumentation.format",".data");
    return detect_format(cfg_val);
  }
}

/***********************************************************
 ** ABSTRACT DATASET CLASS                                **
 ***********************************************************/

umbci::dataset::dataset(string fn,int creation_mode) :
  filename(fn),write_mode(creation_mode),write_state(STATE_INIT) {
  dataset_name=file_utils::fileNameRoot(fn);
}

void umbci::dataset::add_instrument(instrument& i) { 
  is.push_back(i.clone()); 
}

void umbci::dataset::add_instrument(instrument *i) { 
  is.push_back(i); 
}

/***********************************************************
 ** ASCII DATASET CLASS (for logging data to text/csv)    **
 ***********************************************************/

umbci::ascii_dataset::ascii_dataset(string filename,int creation_mode) :
  dataset(filename,creation_mode),
  write_header(true),comment_header(false),
  autonumber_cols(false),base_index(0) {
}

string umbci::ascii_dataset::default_comment() {
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  return ascii_comment+" created "+asctime(timeinfo);
}

void umbci::ascii_dataset::writeline() {
  if (write_state == STATE_INIT) start_file();
  ofstream fileStream(filename.c_str(),ios::app);
  if (fileStream.is_open()) {
    for (instrument_list_t::iterator ili = is.begin();
	 ili != is.end();
	 ili++) {
      if (ili != is.begin()) fileStream << ascii_separator;
      fileStream << (*ili)->string_value();
    }
    fileStream << ascii_delimiter;
    fileStream.close();
  }
  else {
    uLog::annotate(UMBCLOG_ERROR,"unable to open '"+filename+"' for writing!");
  }
}

void umbci::ascii_dataset::write_hdr(ofstream& fileStream) {
  if (should_comment_header()) {
    if (comment.empty()) 
      fileStream << default_comment() << endl;
    else fileStream << comment;
  }
  int index=base_index;
  for (instrument_list_t::iterator ili = is.begin();
       ili != is.end();
       ili++) {
    if (ili != is.begin()) fileStream << ascii_separator;
    char k[16];
    sprintf(k,"(%d)",index);
    fileStream << (*ili)->get_name() << k;
    index++;
  }
  fileStream << ascii_delimiter;
}

void umbci::ascii_dataset::start_file() {
  bool is_new=false;
  if (file_utils::establish_file(filename,write_mode,&is_new)) {
    if (is_new) {
      ofstream fileStream(filename.c_str(),ios::app);
      if (should_write_header()) write_hdr(fileStream);
    }
    write_state=STATE_WRITING;
  }
  else
    uLog::annotate(UMBCLOG_ERROR,"unable to open '"+filename+"' for writing!");
}

/***********************************************************
 ** CSV DATASET CLASS                                     **
 ***********************************************************/

umbci::csv_dataset::csv_dataset(string filename,int creation_mode) :
  ascii_dataset(filename,creation_mode) {
  set_ascii_delim("\n");
  set_ascii_sep(",");
  set_ascii_com("# ");
  write_header=true;
  comment_header=false;
  set_autonumber_cols(false);
  base_index=0;
}

umbci::csv_dataset::~csv_dataset() {
}


/***********************************************************
 ** GNUPLOT DATASET CLASS                                 **
 ***********************************************************/

umbci::gnuplot_dataset::gnuplot_dataset(string filename,int creation_mode) :
  ascii_dataset(filename,creation_mode) {
  set_ascii_delim("\n");
  set_ascii_sep("\t");
  set_ascii_com("# ");
  write_header=true;
  comment_header=true;
  base_index=1;
  set_autonumber_cols(true);
}

/***********************************************************
 ** ARFF DATASET CLASS                                    **
 ***********************************************************/

umbci::arff_dataset::arff_dataset(string filename,int creation_mode) :
  ascii_dataset(filename,creation_mode) {
  set_ascii_delim("\n");
  set_ascii_sep(",");
  set_ascii_com("% ");
  set_autonumber_cols(false);
  write_header  =true;
  comment_header=true;
  base_index=0;
}

string umbci::arff_dataset::arff_type(instrument *i) {
  instrumentation_dtypes_t idt = i->dtype();
  switch(idt) {
  case INTEGER:
  case DOUBLE:
    return "NUMERIC";
  case BOOLEAN:
    return i->range();
  default:
    return "STRING";
  }
}

void umbci::arff_dataset::write_hdr(ofstream& fileStream) {
  if (should_comment_header()) {
    if (comment.empty()) 
      fileStream << default_comment() << endl;
    else fileStream << comment;
  }
  fileStream << "@RELATION " << dataset_name << endl << endl;
  int index=base_index;
  for (instrument_list_t::iterator ili = is.begin();
       ili != is.end();
       ili++) {
    fileStream << "@ATTRIBUTE " << (*ili)->get_name();
    if (autonumber_cols) {
      char k[16];
      sprintf(k,"(%d)",index);
      fileStream << k;
    }
    fileStream << " " << arff_type(*ili) << endl;
    index++;
  }
  fileStream << endl << "@DATA" << endl;
}

/***********************************************************
 ** INSTRUMENT CLASSES (for specifying data sources)      **
 ** extend "instrument" if you don't find what you need   **
 ***********************************************************/

string umbci::dbl_ptr_instrument::string_value() {
  char b[80];
  sprintf(b,"%lf",value());
  return b;
}

string umbci::int_ptr_instrument::string_value() {
  char b[80];
  sprintf(b,"%d",*source);
  return b;
}

string umbci::dbl_ptr_instrument::range() {
  char b[80];
  sprintf(b,"[-%lf..%lf]",DBL_MAX,DBL_MAX);
  return b;
}

string umbci::int_ptr_instrument::range() {
  char b[50];
  sprintf(b,"[%d..%d]",INT_MIN,INT_MAX);
  return b;
}

string umbci::decl_based_instrument::range() {
  char b[80];
  cout << DBL_MAX << endl;
  sprintf(b,"[-%le .. %le]",DBL_MAX,DBL_MAX);
  // sprintf(b,"[-%c..%c]",'\236','\236');
  return b;
}

string umbci::decl_based_instrument::string_value() {
  char b[50];
  sprintf(b,"%lf",value());
  return b;
}

double umbci::decl_based_instrument::value() {
  return declarations::get_declaration_count(declaration);
}

string umbci::settable_bool_instrument::string_value() {
  if (val)
    return "true";
  else return "false";
}

string umbci::settable_bool_instrument::range() {
  return "{ true, false }";
}
