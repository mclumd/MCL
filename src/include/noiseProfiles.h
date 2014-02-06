#ifndef MCL_NOISE_PROFILES
#define MCL_NOISE_PROFILES

// #include "mclEntity.h"
#include <string>
#include <map>
using namespace std;

// to do: make noise profile an MCL entity?

namespace metacog {
 
  //! base class to model sensor/observable noise
  class noiseProfile {
  public:
    noiseProfile() {};
    virtual ~noiseProfile() {};
    
    // we need a tolerate function for each of the mathematical relations
    // we might consider
    virtual double p_that_EQ(double target,double observed)=0;
    virtual double p_that_GT(double target,double observed)=0;
    virtual double p_that_LT(double target,double observed)=0;
    virtual double p_that_WI(double target,double low, double high)=0;
    virtual double p_that_WO(double target,double low, double high)=0;

    virtual string describe()=0;
    
    static noiseProfile* createNoiseProfile(int profile_id);
    static noiseProfile* createNoiseProfile(int profile_id,double param);
    static void establishNoiseProfileFor(string mcl_key, string observable,
					 noiseProfile* np);
    static noiseProfile* getNoiseProfileFor(string mcl_key, string observable);
    
    static void np_dump();

  private:
    static map<string,noiseProfile*> mcl_np_map;
    
  };
  
  //! default profile
  class perfectProfile : public noiseProfile {
  public:
    perfectProfile() : noiseProfile() {};
    virtual double p_that_EQ(double target, double observed);
    virtual double p_that_LT(double target, double observed);
    virtual double p_that_GT(double target, double observed);
    virtual double p_that_WI(double target, double low, double high);
    virtual double p_that_WO(double target, double low, double high);

    virtual string describe() { return "<perfect_np>"; };
    
  };
  
  //! a noise profile that tolerates fixed, uniform noise within specified 
  //! proportional range
  class uniformNoiseProfile : public noiseProfile {
  public:
    uniformNoiseProfile() : noiseProfile(),tol(0.0) {};
    uniformNoiseProfile(double tolerance) : noiseProfile(),tol(tolerance) {};
    
    virtual double p_that_EQ(double target, double observed);
    virtual double p_that_LT(double target, double observed);
    virtual double p_that_GT(double target, double observed);
    virtual double p_that_WI(double target, double low, double high);
    virtual double p_that_WO(double target, double low, double high);
    
    virtual string describe();

    virtual bool tolerate(double target,double observed) {
      return (((target - (target*tol)) <= observed) &&
                                         (observed <= (target + (target*tol))));
    };
    
  protected:
    double tol;
    
  };

};

/*
//! a mixin class that adds a noiseProfile
class hasNoiseFilter {
 public:
  hasNoiseFilter() : nf(noiseFilter::createNoiseProfile(MCL_NP_DEFAULT)) {};
  virtual ~hasNoiseFilter() { if (nf) delete nf; };
  virtual void setNF(noiseProfile* x) {
    if (nf) delete nf;
    nf=x;
  };
  virtual noiseProfile* getNF() { return nf; };

 protected:
  noiseProfile* nf;
};
*/

#endif
