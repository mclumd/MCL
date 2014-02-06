#include <vector>
using namespace std;

/** \file
 *  \brief common typedefs.
 */

namespace metacog {
  class mclLink;
  class intraontologicalLink;
  class interontologicalLink;
  
  typedef vector<mclLink *> linkList;
  
  typedef linkList::iterator     llIterator;
};
