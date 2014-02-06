#ifndef MCL_CODES_H
#define MCL_CODES_H

namespace metacog {

  /////////////////////////////////////////////////////////////////
  //
  // GENERIC CODES AND DEFINES FOLLOW....
  //

  // these are indexes into the buildCore vector...
#define INDICATION_INDEX 0
#define FAILURE_INDEX    1
#define RESPONSE_INDEX   2
  
  /////////////////////////////////////////////////////////////////
  //
  // FRAME CODES AND DEFINES FOLLOW....
  //
  
  /** framestate defines for tracking #mclFrame status.
   */
  enum framestates { 
    FRAME_NEW, //!< frame newly created
    FRAME_RESPONSE_ISSUED, //!< MCL has made a recommendation
    FRAME_RESPONSE_TAKEN,  //!< response is being/has been implemented by host
    FRAME_RESPONSE_DECLINED,  //!< response is declined by host
    FRAME_RESPONSE_ACTIVE, //!< MCL is monitoring the response
    FRAME_RESPONSE_FAILED, //!< MCL or Host signal response failure
    FRAME_RESPONSE_SUCCEEDED, //!< MCL or Host detect response success
    FRAME_RESPONSE_IGNORED,   //!< the Host is ignoring MCL
    FRAME_RESPONSE_ABORTED,//!< the Host has aborted the resp.
    FRAME_REFAIL,          //!< recurrence follows success
    FRAME_UPDATED,         //!< updated with feedback, ready for re-guide
    FRAME_DEADEND,         //!< MCL is out of suggestions
    FRAME_ERROR            //!< MCL has encountered an error
  };
  
  enum entry_code  { 
    ENTRY_UNKNOWN,     //!< uninit state
    ENTRY_NEW,         //!< new frame 
    ENTRY_HIA,         //!< for entry due to HIA
    ENTRY_VIOLATION,   //!< expectation was violated
    ENTRY_CLEAN,       //!< no violation occurs
    REENTRY_RECURRENCE,  //!< a recurrence of the violation
    REENTRY_ALIAS_VIOL,  //!< an alias violation occurred
    REENTRY_HOST_SUCCESS,//!< the host declares response "succeeded"
    REENTRY_HOST_FAIL,   //!< the host declares response "failed"
    REENTRY_HOST_ABORT,  //!< the host declares response was aborted
    REENTRY_HOST_IGNORE  //!< the host declares response was ignored
  };

  /////////////////////////////////////////////////////////////////
  //
  // Constansts that should be moved elsewhere or made paramaters
  //
#define MAXIMUM_MCL_FRAMES 64 

};

#endif
