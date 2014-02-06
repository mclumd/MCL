#ifndef API_CODES_H
#define API_CODES_H

#include "APITypes.h"

/////////////////////////////////////////////////////////////////
//
// GENERIC CODES AND DEFINES FOLLOW....
//
/*! \file
  \brief defines and enums for MCL operability.
*/

#define MISSING_FLOAT_VALUE -9797.9797

//!brief Property Code-Indications.
/*! these property codes are for the host system to specify
 properties that apply to the current state of control in the MCL
 property vector. */
enum pci_enum 
  { PCI_INTENTIONAL,        //!< control is currently active/intentional
    PCI_EFFECTORS_CAN_FAIL, //!< host effectors are failable
    PCI_SENSORS_CAN_FAIL,   //!< host sensors are failable
    PCI_PARAMETERIZED,      //!< host control is parameterized
    PCI_DECLARATIVE,        //!< host control uses declarative knowledge
    PCI_RETRAINABLE,        //!< host controller is retrainable
    PCI_HLC_CONTROLLING,    //!< host uses high level control to choose goals
    PCI_HTN_IN_PLAY,        //!< host is using an HTN
    PCI_PLAN_IN_PLAY,       //!< host is executing a plan
    PCI_ACTION_IN_PLAY,     //!< host is executing an action
    PCI_MAX                 //!< maximum number of PCIs
  };

//! Concrete Response Codes.
/*!  these response codes are for the host system to specify
 responses that are applicable to the currently executing control block. */
enum crc_enum
  { CRC_IGNORE=PCI_MAX,     //!< response code: ignore the violation
    CRC_NOOP,               //!< response code: MCL takes No Operation
    CRC_TRY_AGAIN,          //!< response code: Host try same control block
    CRC_SOLICIT_HELP,       //!< response code: ask for help
    CRC_RELINQUISH_CONTROL, //!< response code: let user control system
    CRC_SENSOR_DIAG,        //!< response code: run sensor diagnostic
    CRC_EFFECTOR_DIAG,      //!< response code: run effector diagnostic
    CRC_SENSOR_RESET,       //!< response code: reset / repair sensor
    CRC_EFFECTOR_RESET,     //!< response code: reset / repair effector
    CRC_ACTIVATE_LEARNING,  //!< response code: activate/reactivate learning
    CRC_ADJ_PARAMS,         //!< response code: adjust/optimize parameters
    CRC_REBUILD_MODELS,     //!< response code: rebuild underlying models
    CRC_REVISIT_ASSUMPTIONS,//!< response code: revisit control assumptions
    CRC_AMEND_CONTROLLER,   //!< response code: modify/repair control structure
    CRC_REVISE_EXPECTATIONS,//!< response code: revise controller expectations
    CRC_ALG_SWAP,           //!< response code: swap out underlying algorithm
    CRC_CHANGE_HLC,         //!< response code: change high level control goals
    PC_VECTOR_LENGTH        //!< response code: maximum length of PC vector
};

inline int crc_offset_base_zero(pkType code) { return code-PCI_MAX; }

#define CRC_MAX PC_VECTOR_LENGTH-PCI_MAX

// values

//! Property Code "true" value.
#define PC_YES true
//! Property Code "false" value.
#define PC_NO  false

/////////////////////////////////////////////////////////////////
//
// EXPECTATION-RELATED CODES FOLLOW....
//
//  expectations are what help MCL determine when something has
//  gone wrong. the following are the "types" of expectations that
//  the host system can post on its own.
//

#define EC_ILLEGAL       0x00 //!< UNUSED expectation code

// maintenance
#define EC_STAYUNDER     0x01 //!< expectation: stay under spec value
#define EC_STAYOVER      0x02 //!< expectation: stay over spec value
#define EC_MAINTAINVALUE 0x04 //!< expectation: maintain spec value
#define EC_WITHINNORMAL  0x08 //!< expectation: stay within range of spec value

#define EC_REALTIME      0x10 //!< expectation: compl. before realtime deadline
#define EC_TICKTIME      0x11 //!< expectation: compl. before tick deadline

// effects
#define EC_GO_UP         0x20 //!< effect: go up from current value
#define EC_GO_DOWN       0x21 //!< effect: go down from current value
#define EC_NET_ZERO      0x22 //!< effect: no net change at conclusion
#define EC_ANY_CHANGE    0x23 //!< effect: any change at all on conclusion
#define EC_NET_RANGE     0x24 //!< effect: conclude within some range
#define EC_TAKE_VALUE    0x30 //!< effect: change to specified value

/////////////////////////////////////////////////////////////////
//
// SENSOR-RELATED CODES FOLLOW....
//
//  sensor defs are the API for the host system to express to MCL
//  what sensors it has access to and what their characteristics are
//

// first - type blocks for linking into the indication fringe

//! Data Types.
/*! Used to specify the data type of a sensor. */
enum data_types { DT_INTEGER, DT_RATIONAL, DT_BINARY, DT_BITFIELD, DT_SYMBOL };

//! Sensor Classes.
/*! Used to specify the class of a sensor. */
enum sensor_classes { 
  SC_STATE, 
  SC_CONTROL, 
  SC_SPATIAL, 
  SC_TEMPORAL, 
  SC_RESOURCE, 
  SC_REWARD, 
  SC_AMBIENT, 
  SC_OBJECTPROP, 
  SC_MESSAGE, 
  SC_COUNTER,
  SC_UNSPEC 
};

#define SC_NUMCODES_LEGAL SC_UNSPEC+1

//! Noise Filter Codes.
/*! Used to specify the parameters of noise tolerant filters. */
enum noise_profiles { 
  MCL_NP_NO_PROFILE=0x0,
  MCL_NP_PERFECT,
  MCL_NP_UNIFORM,
  MCL_NP_AUTOMATIC=0xFF
};

#define MCL_NP_DEFAULT MCL_NP_PERFECT

//! Property Code Indexes.
/*! Used to specify the index into the sensor property codes. */
enum property_codes { PROP_DT, PROP_SCLASS, PROP_NOISEPROFILE, NUMBER_OF_SENSOR_PROPS };

#endif
