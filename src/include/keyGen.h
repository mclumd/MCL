#include "APITypes.h"

/** \file
 *  \brief contains a hash function.
 */

//! computes a hash key for string q.
//! hash keys are between 0x00000000 0x0FFFFFFF
//! and are computed with the following simple hash function:
//! @code
//!   int hk=0;
//!   for (int k=2; k < (int)q.length(); k++)
//!    hk = (hk<<4)^(hk>>28)^q[k];
//!   hk &= 0x0FFFFFFF;
//!   return hk;
//! @endcode
mclKey computeKey(string q);
