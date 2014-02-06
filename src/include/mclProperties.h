#ifndef MCL_PROPERTIES_H
#define MCL_PROPERTIES_H

/** \file mclProperties.h
 * \brief classes pertaining to the management of host properties.
 * Contains the functionality for a host system to express its operational
 * properties to MCL and the necessary structures for MCL to maintain and
 * retrieve those values in an organized fashion.
 */

#include <list>
#include <string>
#include "mclEntity.h"
#include "mcl_api.h"

using namespace std;

#define MAX_VECTOR_LENGTH 128

namespace metacog {

  class mcl;

/** an container of host properties and their values.
 * properties are specified as an index into the property vector (see pkType)
 * (see api/APICodes.h)
 * values are of type pvType (currently integer). The property vector also
 * maintains a boolean mask representing which values have been set explicitly
 * and which values have been left unspecified (see #propertyVectorStack)
 */
class mclPropertyVector : public mclEntity {

 public:

  mclPropertyVector() : myMCL(NULL) { reSetProperties(); };
  mclPropertyVector(mcl *m) : myMCL(m) { reSetProperties(); };

  void copyValues(mclPropertyVector& dest);

  virtual ~mclPropertyVector() {};
  virtual string baseClassName() { return "propertyVector"; };

  //! returns the MCL object to which the PV is attached.
  virtual mcl *MCL() { return myMCL; };
  //! sets the MCL object to which the PV is attached.
  void    setMCL(mcl *m) { myMCL = m; };

  //! returns a property value.
  pvType  getPropertyValue(int index) { return values[index]; };
  //! returns a property mask value (whether the property has been explicitly set).
  bool    getPropertyMaskValue(int index) { return mask[index]; };

  //! unsets a property in the vector and clears its mask value.
  void    unSetProperty(int index);
  //! sets a property to the default value #PC_YES.
  void    setProperty(int index) { values[index]=PC_YES; mask[index]=true; };
  //! sets a property to the specified value v.
  void    setProperty(int index,pvType v) { values[index]=v; mask[index]=true; };
  //! resets the property vector values to the global (static) defaults.
  //! \sa setDefaultProperty()
  //! \sa getDefaultProperty()
  void  reSetProperties();
  //! tests whether the specified property is equal to 'tv'.
  bool  testProperty(int index,pvType tv) { return (values[index]==tv); };

  //! set the default (global/static) value of a property.
  static void setDefaultProperty(int index,pvType nv);
  //! get the default (global/static) value of a property.
  static pvType getDefaultProperty(int index);

  virtual void dumpEntity(ostream *strm);

  //! string values assigned to the elements of a property vector.
  static string pnames[];
  static pvType defaults[];

 protected:

 private:
  pvType  values[MAX_VECTOR_LENGTH];
  bool    mask  [MAX_VECTOR_LENGTH];
  mcl    *myMCL;

};

/** a stack of property vectors.
 * A stack of property vectors, useful in maintaining system properties
 * as nested control is executed on the host side. Use of the property stack
 * (pushing and popping frames) is host-initiated and optional. It can be used
 * as a singleton property vector if no new frames are never requested by the
 * host.
 *
 * New when a new frame is requested, the associated property vector is
 * initialized using the values of the existing top-level vector (which 
 * starts out as the PropertyVector.defaults vector) so changes to the 
 * new property vector only mask out existing ones until the newly created
 * frame is popped off.
 */
class mclPropertyVectorStack : public mclEntity {
 public:
  mclPropertyVectorStack();
  mclPropertyVectorStack(mcl *m);

  virtual string baseClassName() { return "propertyVectorStack"; };
  virtual mcl *MCL() { return myMCL; };
  void    setMCL(mcl *m) { myMCL = m; base->setMCL(m); };

  //! returns a pointer to the top level property vector.
  mclPropertyVector *currentPV() 
    { return (stack.empty()) ? base : stack.front(); };
  //! returns a pointer to the base level (default) property vector.
  mclPropertyVector *basePV() { return base; };
  //! sets a property value for the current (top) property vector.
  void setPropertyValue(pkType index, pvType v) 
    { currentPV()->setProperty(index,v); };
  //! gets a property value for the current (top) property vector.
  pvType currentPropertyValue(pkType index) 
    { return currentPV()->getPropertyValue(index); };

  //! essentially pushes a new property vector onto the top of the PV stack.
  void newPropertyVector();
  //! pops and destroys the top level property vector.
  void popPropertyVector();

 private:
  list<mclPropertyVector *> stack;
  mclPropertyVector * base;
  mcl    *myMCL;

};

};
#endif
