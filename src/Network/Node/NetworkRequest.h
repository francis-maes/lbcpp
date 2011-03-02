/*-----------------------------------------.---------------------------------.
| Filename: NetworkRequest.h               | Network Request                 |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_REQUEST_H_
# define LBCPP_NETWORK_REQUEST_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class NetworkRequest : public Object
{
public:
  NetworkRequest(ExecutionContext& context,
                 const String& projectName, const String& source, const String& destination, WorkUnitPtr workUnit,
                 size_t requiredCpus = 1, size_t requiredMemory = 2, size_t requiredTime = 10);
  NetworkRequest() {}
  
 void setIdentifier(const String& identifier)
    {this->identifier = identifier;}
  
  const String& getIdentifier() const
    {return identifier;}

  const String& getProjectName() const
    {return projectName;}

  const String& getSource() const
    {return source;}

  const String& getDestination() const
    {return destination;}

  void selfGenerateIdentifier()
    {identifier = generateIdentifier();}
  
  juce::int64 getCreationTime()
    {return identifier.getLargeIntValue();}
  
  WorkUnitPtr getWorkUnit(ExecutionContext& context) const
    {return workUnit ? workUnit->createObjectAndCast<WorkUnit>(context) : WorkUnitPtr();}
  
protected:
  friend class NetworkRequestClass;

  String identifier;
  String projectName;
  String source;
  String destination;
  XmlElementPtr workUnit;
  size_t requiredCpus;
  size_t requiredMemory; // Gb
  size_t requiredTime; // Hour
  
  static juce::int64 lastIdentifier;
  
  static String generateIdentifier();
};

typedef ReferenceCountedObjectPtr<NetworkRequest> NetworkRequestPtr;

class NetworkResponse : public Object
{
public:
  NetworkResponse(ExecutionContext& context, const String& identifier, ExecutionTracePtr executionTrace)
    : identifier(identifier), trace(new XmlElement())
    {this->trace->saveObject(context, executionTrace);}
  NetworkResponse(const String& identifier)
    : identifier(identifier) {}
  NetworkResponse() {}

  const String& getIdentifier() const
    {return identifier;}
  
  ExecutionTracePtr getExecutionTrace(ExecutionContext& context) const
    {return trace ? trace->createObjectAndCast<ExecutionTrace>(context) : ExecutionTracePtr();}

protected:
  friend class NetworkResponseClass;

  String identifier;
  XmlElementPtr trace;
};

typedef ReferenceCountedObjectPtr<NetworkResponse> NetworkResponsePtr;

class NetworkArchive : public Object
{
public:
  NetworkArchive(NetworkRequestPtr request, NetworkResponsePtr response)
    : request(request), response(response) {}
  NetworkArchive() {}
  
  const NetworkRequestPtr& getNetworkRequest() const
    {return request;}
  
  const NetworkResponsePtr& getNetworkResponse() const
    {return response;}
  
protected:
  friend class NetworkArchiveClass;
  
  NetworkRequestPtr request;
  NetworkResponsePtr response;
};

typedef ReferenceCountedObjectPtr<NetworkArchive> NetworkArchivePtr;

extern ClassPtr networkRequestClass;
extern ClassPtr networkResponseClass;

}; /* namespace */

#endif // !LBCPP_NETWORK_REQUEST_H_
