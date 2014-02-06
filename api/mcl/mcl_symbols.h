#ifndef MCL_SYMBOLS_HEADER_ISDEF
#define MCL_SYMBOLS_HEADER_ISDEF

#include "APITypes.h"
#include <string>
#include <map>

using namespace std;



#define MISSING_FLOAT_VALUE -9797.9797

#define PCI_MIN_INDEX 0
#define PCI_INTENTIONAL 0
#define PCI_EFFECTORS_CAN_FAIL 1
#define PCI_SENSORS_CAN_FAIL 2
#define PCI_PARAMETERIZED 3
#define PCI_DECLARATIVE 4
#define PCI_RETRAINABLE 5
#define PCI_HLC_CONTROLLING 6
#define PCI_HTN_IN_PLAY 7
#define PCI_PLAN_IN_PLAY 8
#define PCI_ACTION_IN_PLAY 9
#define PCI_MAX 10
#define PCI_COUNT PCI_MAX

#define CRC_MIN_INDEX 10
#define CRC_IGNORE 10
#define CRC_NOOP 11
#define CRC_TRY_AGAIN 12
#define CRC_TRY_2_AGAIN 13
#define CRC_SOLICIT_HELP 14
#define CRC_RELINQUISH_CONTROL 15
#define CRC_SENSOR_DIAG 16
#define CRC_EFFECTOR_DIAG 17
#define CRC_SENSOR_RESET 18
#define CRC_EFFECTOR_RESET 19
#define CRC_ACTIVATE_LEARNING 20
#define CRC_ADJ_PARAMS 21
#define CRC_REBUILD_MODELS 22
#define CRC_REVISIT_ASSUMPTIONS 23
#define CRC_AMEND_CONTROLLER 24
#define CRC_REVISE_EXPECTATIONS 25
#define CRC_ALG_SWAP 26
#define CRC_CHANGE_HLC 27
#define CRC_RESCUE 28
#define CRC_GIVE_UP 29

#define CRC_EXTENDED_CODE 30

#define CRC_PCV_MAXINDEX 31
#define PC_VECTOR_LENGTH CRC_PCV_MAXINDEX  //!< size of PC vector (PCI + CRC)
#define CRC_MAX (PC_VECTOR_LENGTH - PCI_MAX) //!< number of CRCs total

 inline int crc_offset_base_zero(pkType code) { return code - PCI_MAX; }

#define PC_YES true      //! prop code true
#define PC_NO false     //! prop code false 

/* expectation codes */
#define EC_ILLEGAL 0x00    //!< UNUSED expectation code


/* maintenance */
#define EC_STAYUNDER 1
#define EC_STAYOVER 2
#define EC_MAINTAINVALUE 3
#define EC_WITHINNORMAL 4

#define EC_RATIO_MAINTAIN 5
#define EC_RATIO_STAYUNDER 6
#define EC_RATIO_STAYOVER 7

#define EC_REALTIME 8
#define EC_TICKTIME 9

/* effects     */
#define EC_GO_UP 10
#define EC_GO_DOWN 11
#define EC_NET_ZERO 12
#define EC_ANY_CHANGE 13
#define EC_NET_RANGE 14
#define EC_TAKE_VALUE 15

#define EC_DONT_CARE 16
#define EC_BE_LEGAL 17

/* data types     */
#define DT_INTEGER 0
#define DT_RATIONAL 1
#define DT_BINARY 2
#define DT_BITFIELD 3
#define DT_SYMBOL 4

/* sensor codes   */
#define SC_STATE 0
#define SC_CONTROL 1
#define SC_SPATIAL 2
#define SC_TEMPORAL 3
#define SC_RESOURCE 4
#define SC_REWARD 5
#define SC_AMBIENT 6
#define SC_OBJECTPROP 7
#define SC_MESSAGE 8
#define SC_COUNTER 9
#define SC_ACTOR 10
#define SC_OBJECT 11
#define SC_GOALSCENE 12
#define SC_MAINRESULT 13
#define SC_OPPONENT 14
#define SC_CHARGE 15
#define SC_TO 16
#define SC_ITEMLOCATION 17
#define SC_UNSPEC 18
#define SC_NUMCODES_LEGAL 19

/* noise profiles */
#define MCL_NP_NO_PROFILE 0
#define MCL_NP_PERFECT 1
#define MCL_NP_UNIFORM 2
#define MCL_NP_AUTOMATIC 0xFF
#define MCL_NP_DEFAULT MCL_NP_PERFECT

/* property code indexes (into the prop vector) */
#define PROP_DT 0
#define PROP_SCLASS 1
#define PROP_NOISEPROFILE 2
#define PROP_COUNT 3
#define NUMBER_OF_SENSOR_PROPS PROP_COUNT

/* miscellaneous defines ... */
#define RESREF_NO_REFERENCE 0
#define EGK_NO_EG 0
#define EGK_BASE_LEVEL -1

#define RELATION_EQUAL 0
#define RELATION_GT 1
#define RELATION_LT 2
#define RELATION_GTE 3
#define RELATION_LTE 4

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
///
/// FUNCTIONS FOR SYMBOLS NAMESPACE...
///

namespace metacog {

  namespace symbols {

    class symbol_table {
      public:
	symbol_table(string name);
        int symbol_intval(string name);
        int symbol_intval(string name,bool* exists);
        double symbol_dblval(string name);
        double symbol_dblval(string name,bool* exists);
	void symbol_def(string name,int val);
	void symbol_def(string name,double val);
	string reverse_lookup(string prefix, int value,
			      bool* none, bool* multi);

      private:
	map<string,int> intmap;
	map<string,double> dblmap;
	string tname;

    };

    extern symbol_table __mcl_global_symtable;

    int  global_symbol_intval(string name);
    int    global_symbol_intval(string name,bool* exists);
    double global_symbol_dblval(string name);
    double global_symbol_dblval(string name,bool* exists);
    void   global_symbol_def(string name,int val);
    void   global_symbol_def(string name,double val);

    int    smartval_int(string specifier, bool* error);
    string reverse_lookup(string prefix, int value, bool* none, bool* multi);

  };

};

#endif


