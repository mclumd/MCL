#ifndef MCL_SENSOR_DEF_H
#define MCL_SENSOR_DEF_H

/** \file
 * \brief Host sensor definitions.
 */

#include <stdlib.h>
#include <string.h>
#include "mcl_api.h"
#include "mclEntity.h"

namespace metacog {

  class mcl;

/** A defined host sensor.
 * CONFUSION ALERT!
 * Sensor defs have property vectors. They are similar in nature to the MCL
 * property vector (see #mclPropertyVector.h) but do not contain the same
 * properties and do not use the mclPropertyVector class. Sensor properties
 * are maintained locally to the sensor def as arrays!
 */
class mclSensorDef : public mclEntity {
 public:
  mclSensorDef(string name,mcl *m) : mclEntity(name),myMCL(m)
    { resetPropVec(); };

  //! sets a property of the sensor to the specified value
  void setSensorProp(spkType propKey, spvType propVal) {
    s_prop_vector[propKey]=propVal;
  }
  //! gets the value of the specified sensor property
  spvType getSensorProp(spkType propKey) { return s_prop_vector[propKey]; };
  //! tests if the value of the specified sensor property is equal to testVal
  bool testSensorProp(spkType propKey, spvType testVal) {
    return (s_prop_vector[propKey] == testVal);
  }

  //! part of the MCLEntity interface -- returns attached MCL object
  virtual mcl *MCL() { return myMCL; };

  virtual string baseClassName() { return "sensorDef"; };

  virtual void dumpEntity(ostream *strm);

  virtual mclSensorDef* clone() { return new mclSensorDef(*this); };

 protected:
  //! (global) defaults for sensor property vector
  static spvType def_s_prop_vector[NUMBER_OF_SENSOR_PROPS];
  //! actual (local) sensor property vector
  spvType            s_prop_vector[NUMBER_OF_SENSOR_PROPS];

  mcl *myMCL;

  //! resets (local) sensor property vector to (global) defaults
  void resetPropVec() { 
    memcpy(s_prop_vector,def_s_prop_vector,sizeof(spvType)*NUMBER_OF_SENSOR_PROPS); 
  };

};

};
#endif

