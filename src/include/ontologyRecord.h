#include "mclEntity.h"

#include <map>

using namespace std;

namespace metacog {

  class mclFrame;

  class ontologyRecord : public frameComponent {
  public:
    ontologyRecord(mclFrame* the_frame);
    virtual ~ontologyRecord() {};

    virtual string toString();
    virtual vector<string> nodeNames()=0;
    virtual double p_for(string node)=0;

    static const double P_NONE = -1.0;

  protected:
    mclFrame* my_frame;

  private:
    int       when;

  };
  
  class ontologyRecord_map : public ontologyRecord {
  public:
    ontologyRecord_map(mclFrame* the_frame);
    virtual ~ontologyRecord_map() {};

    static const double P_NONE = -1.0;
    virtual vector<string> nodeNames();
    virtual double p_for(string node);
    virtual string baseClassName() { return "or_map"; };

  protected:

  private:
    map<string,double> p_map;

  };

  class or_comparison {
  public:
    or_comparison() {};
    virtual ~or_comparison() {};
    virtual double similarity(const ontologyRecord& or1, 
			      const ontologyRecord& or2)=0;
  };

};
