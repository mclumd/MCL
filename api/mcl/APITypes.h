#ifndef MCL_API_TYPES_H
#define MCL_API_TYPES_H

/*! \file APITypes.h
  \brief typedefs used by MCL and the MCL/Host API
*/

#include <string>
#include <vector>
#include <iostream>

using namespace std;

typedef unsigned int mclKey;  // mcl-assigned hash key

typedef unsigned int pkType;  // property key type
typedef bool         pvType;  // property value type

typedef unsigned int  spkType; // sensor property key
typedef unsigned char spvType; // sensor property value

typedef unsigned int ecType;          // expectation code type
typedef unsigned long int egkType;    // expectation group key
typedef unsigned long int resRefType; // response reference type

// monitor response requires the preceding definitions
#ifndef MCL_MONITOR_RESPONSE_H
#include "mclMonitorResponse.h"
#endif

#endif
