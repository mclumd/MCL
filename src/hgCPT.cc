#include <math.h>
#include "hgGlue.h"

using namespace metacog;

// The CPT class has been moved inside of the hgGlue system.
// This is because some of the CPT-based computations require
// looking at the p_true values of the incoming factors, and 
// these calculations should be done using hgGlue rather than 
// anything else

// here's how we index into the CPT:
// each input gets a bit in a bitfield of size = |ins|
// the bit # it gets is its index into ins
// so, the first node in ins gets bit 0, and so on
// now, if node N is "true", then that bit is a 1, otherwise zero

CPT::CPT(linkList *in_links) : ins(in_links) {
  t_size= (int)pow(2,in_links->size());
  table = new double[t_size];
  initializeUniform();
}

CPT::~CPT() {
  delete[] table;
}

double CPT::discrete_p() {
  int index=0;
  for (llIterator inI = ins->begin();
       inI != ins->end();
       inI++) {
    index = (index >> 1);
    if ((*inI)->sourceNode()->p_true(HG_GLUE_IDENTIFIER) > 0.5)
      index++;
  }
  return table[index];
}

double CPT::p() {
  if (size() == 1)
    return table[0];
  else 
    return p_priors();
}

void CPT::dumpEntity(ostream *o) {
  char b[512];
  b[0]='\0';
  for (llIterator inI = ins->begin();
       inI != ins->end();
       inI++) {
    sprintf(b,"%s%-21s ",b,(*inI)->sourceNode()->entityBaseName().c_str());
  }
  *o << b << "P" << endl;
  for (int i=0;i<t_size;i++) {
    b[0]='\0';
    int lli = 0;
    for (llIterator inI2 = ins->begin();
	 inI2 != ins->end();
	 inI2++) {
      int bmsk = 1 << lli;
      // sprintf(b,"%s[%d,%d,%d]",b,bmsk,lli,i);
      if ((i & bmsk) != 0)
	sprintf(b,"%s%-21s ",b,"TRUE");
      else
	sprintf(b,"%s%-21s ",b,"FALSE");
      lli++;
    }
    sprintf(b,"%s%4.2f",b,table[i]);
    *o << b << endl;
  }
}

void CPT::initializeUniform() {
  for (int i=0;i<t_size;i++)
    table[i]=0.5;
}

void CPT::initializeVector(double v[]) {
  for (int i=0;i<t_size;i++)
    table[i]=v[i];
}


void CPT::initializeVector(vector<double>& v) {
  int i=0;
  for (vector<double>::iterator vdi = v.begin();
       vdi != v.end();
       vdi++) {
    table[i]=*vdi;
    i++;
  }
}

double CPT::p_priors() {
  // computes P based on prior conditional probabilities
  // cum_p = cummulative p value (return)
  double cum_p = 0;
  // loop over entire table
  for (int i=0;i<t_size;i++) {
    // lli counts what column of the table we are looking at as we iterate
    // over incoming links
    int lli = 0;
    // cell_p is the probability that of the condition (row)
    double  cell_p = 1.0;
    // iterate over incoming links
    for (llIterator inI2 = ins->begin();
	 inI2 != ins->end();
	 inI2++) {
      // make a bitmask for this column by shifting 1 over lli bits
      int bmsk = 1 << lli;
      // if the bitmask & the iterator for the table are nonzero than the
      // condition is TRUE for this row
      if ((i & bmsk) != 0) {
	// cell for node is TRUE
	cell_p *= (*inI2)->sourceNode()->p_true(HG_GLUE_IDENTIFIER);
      }
      else {
	// cell for node is FALSE (invert p)
	cell_p *= (1 - (*inI2)->sourceNode()->p_true(HG_GLUE_IDENTIFIER));
      }
      lli++;
    }
    // add to the cummulative p value
    // the cell probability times the conditional probability
    cum_p += cell_p * table[i];
  }
  return cum_p;
}
