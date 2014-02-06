#include "mclSensorDef.h"

using namespace metacog;

spvType mclSensorDef::def_s_prop_vector[NUMBER_OF_SENSOR_PROPS] =
  { DT_SYMBOL , SC_UNSPEC };

void mclSensorDef::dumpEntity(ostream *strm) {
  *strm << "spv(" << entityName() << ") {";
  for (int i=0;i<NUMBER_OF_SENSOR_PROPS;i++) {
    *strm << dec << (int)(s_prop_vector[i]);
    if (i != NUMBER_OF_SENSOR_PROPS-1)
      *strm << ",";
    else *strm << "}";
  }
}
