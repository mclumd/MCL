#ifndef UMBC_INSTRUMENTATION_HEADER
#define UMBC_INSTRUMENTATOIN_HEADER

#include "file_utils.h"
#include <iostream>
#include <string>
#include <list>
using namespace std;

/** \file 
 * \brief a class that provides instrumentation for experiments.
 * a framework for specifying system measurables and writing them to
 * a flat text file
 */

namespace umbc {

  namespace instrumentation {

    enum instrumentation_format_t { GNUPLOT, CSV, ARFF };
    enum instrumentation_dtypes_t { DOUBLE, INTEGER, DISCRETE, BOOLEAN };

    class dataset;
    class instrument;
    typedef list<instrument*> instrument_list_t;

    class dataset {
    public:
    dataset(string fn,int creation_mode);
      virtual void writeline()=0;
      virtual void set_comment(string &hdr_comment) { comment = hdr_comment; };
      virtual string comment_prefix()=0;
      void add_instrument(instrument& i);
      void add_instrument(instrument *i);
      void set_hdr_comment(string hdr);

      enum state_t { STATE_INIT, STATE_WRITING };

    protected:
      virtual void start_file()=0;
      string  filename,dataset_name,comment;
      instrument_list_t is;
      int     write_mode;
      state_t write_state;

    };

    class ascii_dataset : public dataset {
    public:
      ascii_dataset(string filename,int creation_mode);

      virtual void writeline();
      virtual void start_file();

      virtual ~ascii_dataset() {};
      
    protected:
      void set_ascii_delim(string delim) { ascii_delimiter=delim; };
      void set_ascii_sep  (string sep)   { ascii_separator=sep; };
      void set_ascii_com  (string com)   { ascii_comment=com; };
      void set_autonumber_cols(bool val) { autonumber_cols=val; };
      virtual bool should_write_header() { return write_header; };
      virtual bool should_comment_header() { return comment_header; };
      virtual string comment_prefix() { return ascii_comment; };
      
      virtual string default_comment();

      bool   write_header,comment_header,autonumber_cols;
      string ascii_delimiter,ascii_separator,ascii_comment;
      int    base_index;
      virtual void write_hdr(ofstream& fileStream);

    };

   class csv_dataset : public ascii_dataset {
    public:
      csv_dataset(string filename, int creation_mode);
      virtual ~csv_dataset();

    };
    
   class gnuplot_dataset : public ascii_dataset {
    public:
      gnuplot_dataset(string filename, int creation_mode);
      virtual ~gnuplot_dataset() {};

    };
    
   class arff_dataset : public ascii_dataset {
    public:
      arff_dataset(string filename, int creation_mode);

    protected:
      virtual void write_hdr(ofstream& fileStream);

    private:
      string arff_type(instrument *i);

    };
   
    dataset *make_dataset(string const &filename,
			  int creation_mode = file_utils::MODE_MOVE);
    dataset *make_dataset(string const &filename,
			  instrumentation_format_t format,
			  int creation_mode = file_utils::MODE_MOVE);

    instrumentation_format_t detect_format(string const &filename);

    class instrument {
    public:
      instrument(string name) : nm(name) {};
      virtual double value()=0;
      virtual string string_value()=0;
      virtual string range()=0;
      virtual instrument* clone()=0;
      virtual string get_name() { return nm; };
      virtual instrumentation_dtypes_t dtype()=0;
      virtual ~instrument() {};
      
    protected:
      string nm;
      
    };

    class int_ptr_instrument : public instrument {
    public:
      int_ptr_instrument(string name, int* src) : 
	instrument(name),source(src) {};
      virtual double value() { return *source; };
      virtual instrument* clone() { return new int_ptr_instrument(*this); };
      virtual instrumentation_dtypes_t dtype() { return INTEGER; };
      virtual string string_value();
      virtual string range();
      
    private:
      int* source;
    };
    
    class dbl_ptr_instrument : public instrument {
    public:
      dbl_ptr_instrument(string name, double* src) : 
	instrument(name),source(src) {};
      virtual double value() { return *source; };
      virtual instrument* clone() { return new dbl_ptr_instrument(*this); };
      virtual instrumentation_dtypes_t dtype() { return DOUBLE; };
      virtual string string_value();
      virtual string range();
      
    private:
      double* source;
    };
    
    class decl_based_instrument : public instrument {
    public:
      decl_based_instrument(string name, string decl) :
	instrument(name),declaration(decl) {};
      virtual double value();
      virtual instrument* clone() { return new decl_based_instrument(*this); };
      virtual instrumentation_dtypes_t dtype() { return DOUBLE; };
      virtual string string_value();
      virtual string range();
      
    private:
      string declaration;
      
    };
  
    class settable_bool_instrument : public instrument {
    public:
    settable_bool_instrument(string name, bool defval) :
	instrument(name),val(defval) {};
	void setval(bool v) { val = v; }
	virtual double value() { return (val ? 1.0 : 0.0); }
	virtual instrument* clone() 
	{ return new settable_bool_instrument(*this); };
	virtual instrumentation_dtypes_t dtype() { return BOOLEAN; };
	virtual string string_value();
    	virtual string range();
    
    private:
	bool val;
    };
  }
}

#endif
