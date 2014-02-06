#ifndef MCL_FRINGE_LINKAGE_H
#define MCL_FRINGE_LINKAGE_H

#include "mcl_internal_types.h"

/** \file
 * \brief class definitions for MCL's observable database
 */

namespace metacog {

  class produces_linktags  {
  public:

    virtual ~produces_linktags() {};

    //! generates link tags for the expectation.
    //! link tags are strings that refer to indication fringe nodes and
    //! define evidence for auto-activating those nodes when the expectation
    //! is violated.
    virtual void addLinkTags(linkTags_t& rv)=0;

  protected:
    produces_linktags() {};

  };
  
};

#endif
