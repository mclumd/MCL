#ifndef MCL_EXCEPTIONS_H
#define MCL_EXCEPTIONS_H

#include <exception>
#include <string>
#include "APITypes.h"

using namespace std;

namespace metacog {

  class MCLException : public exception {
  public:
  MCLException() 
    : exception(),msg("unspecified exception.") {};
  MCLException(string m) 
    : exception(),msg(m) {};
    
    virtual ~MCLException() throw() {};
    
    virtual const char *what() {
      return msg.c_str();
    };

  protected:
    string msg;
  };

  class EnvironmentalMCLException : public MCLException {
  public:
  EnvironmentalMCLException(string m) : MCLException(m) {};
  };

  class HostProtocolException : public MCLException {
  public:
  HostProtocolException(string m) : MCLException(m) {};
  };

  class InternalMCLException : public MCLException {
  public:
  InternalMCLException(string m) : MCLException(m) {};
  };

  class UnimplementedException : public InternalMCLException {
  public:
  UnimplementedException(string m) : InternalMCLException(m) {};
  };

  class IllegalFrameState : public InternalMCLException {
  public:
  IllegalFrameState(string m) : InternalMCLException(m) {};
  };

  class GlueException : public InternalMCLException {
  public:
  GlueException(string m) : InternalMCLException(m) {};
  };

  class OntologyException : public InternalMCLException {
  public:
  OntologyException(string m) : InternalMCLException(m) {};
  };

  class OntologyConstructionException : public OntologyException {
  public:
  OntologyConstructionException(string m) : OntologyException(m) {};
  };

  class MissingFringeNode : public OntologyException {
  public:
  MissingFringeNode(string node) : 
    OntologyException("Cannot find fringe node "+node) {};
  };

  class MissingNodeConfiguration  : public OntologyException {
  public:
  MissingNodeConfiguration(string node) : 
    OntologyException("No configuration for node "+node) {};
  };

  class NoActiveResponseToAddress : public HostProtocolException {
  public:
  NoActiveResponseToAddress(string routine)  : 
    HostProtocolException("Advice was given by the host but there is no active response in the referent frame: "+routine) {};
  };

  class MissingReferentException : public HostProtocolException {
  public:
    MissingReferentException(string routine) 
      : HostProtocolException("the following protocol requires a referent: "+routine) {};
  };

  class MissingObjectException : public HostProtocolException {
  public:
    MissingObjectException(string oname)
      : HostProtocolException("reference to missing object "+oname) {};
  };

  class MissingObjectDefException : public HostProtocolException {
  public:
    MissingObjectDefException(string oname)
      : HostProtocolException("reference to missing object definition "+oname) {};
  };

  class BadDeclaration : public HostProtocolException {
  public:
    BadDeclaration(string oname)
      : HostProtocolException("bad declaration: "+oname) {};
  };

  class MissingSensorException : public HostProtocolException {
  public:
    MissingSensorException(string sname)
      : HostProtocolException("reference to missing sensor "+sname) {};
  };

  class BadCode : public HostProtocolException {
  public:
    BadCode(string sname)
      : HostProtocolException("bad code: "+sname) {};
  };

  class BadKey : public HostProtocolException {
  public:
    BadKey(string sname)
      : HostProtocolException("bad key: "+sname) {};
  };

  class MissingEGException : public HostProtocolException {
  public:
  MissingEGException(egkType egk) : HostProtocolException("") {
      char b[256];
      sprintf(b,"Attempt to add exp to non-existant EG @0x%lx",egk);
      msg = b;
    };
  };

  class RemovedFromAPIException : public HostProtocolException {
  public:
  RemovedFromAPIException(string function) :
    HostProtocolException(function+" is deprecated and illegal.") {};
  };

};


#endif
