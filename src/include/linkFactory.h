#ifndef MCL2_LINKFACTORY_H
#define MCL2_LINKFACTORY_H

#include "mclOntology.h"
#include <string>

using namespace std;

/** \file
 * \brief Link construction code.
 */

namespace metacog {

/** A static factory class that provides functions for making links.
 *  static functions perform automated bookkeeping.
 */
class linkFactory {
 public:
  //! Make a link from Indication Fringe to Core.
  static bool makeIFCLink(mclOntology *srco,string sname,
			  string dname);
  //! Make a intraontological specification link.
  static bool makeSpecificationLink(mclOntology *srco,string sname,
				    string dname);
  //! Make a intraontological abstraction link.
  static bool makeAbstractionLink(mclOntology *srco,string sname,
				  string dname);
  //! Make a diagnostic link from Inidcation Core to Failure.
  static bool makeDiagnosticLink(mclOntology *srco,string sname,
				 mclOntology *dnco,string dname);
  //! Make a inhibitory link from Inidcation Core to Failure.
  static bool makeInhibitoryLink(mclOntology *srco,string sname,
				 mclOntology *dnco,string dname);
  //! Make a support link from Inidcation Core to Failure.
  static bool makeSupportLink(mclOntology *srco,string sname,
			      mclOntology *dnco,string dname);
  //! Make a prescriptive link from Failure to Response.
  static bool makePrescriptiveLink(mclOntology *srco,string sname,
				   mclOntology *dnco,string dname);

};
};

#endif
