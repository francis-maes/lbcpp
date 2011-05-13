/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitNetworkRequest.h       | Network Request of Work Unit    |
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

class WorkUnitNetworkRequest : public Object
{
public:
  WorkUnitNetworkRequest(ExecutionContext& context,
                 const String& projectName, const String& source, const String& destination, WorkUnitPtr workUnit,
                 size_t requiredCpus = 1, size_t requiredMemory = 2, size_t requiredTime = 10);
  WorkUnitNetworkRequest() {}
  
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

  const size_t& getRequiredMemory() const
    {return requiredMemory;}

  const size_t& getRequiredTime() const
    {return requiredTime;}

  void selfGenerateIdentifier()
    {identifier = generateIdentifier();}
  
  juce::int64 getCreationTime()
    {return identifier.getLargeIntValue();}
  
  WorkUnitPtr getWorkUnit(ExecutionContext& context) const
    {return workUnit ? workUnit->createObjectAndCast<WorkUnit>(context) : WorkUnitPtr();}
  
protected:
  friend class WorkUnitNetworkRequestClass;

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

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

class ExecutionTraceNetworkResponse : public Object
{
public:
  ExecutionTraceNetworkResponse(ExecutionContext& context, const String& identifier, ExecutionTracePtr executionTrace)
    : identifier(identifier), trace(new XmlElement())
    {this->trace->saveObject(context, executionTrace, T("trace"));}
  ExecutionTraceNetworkResponse(const String& identifier)
    : identifier(identifier) {}
  ExecutionTraceNetworkResponse() {}

  const String& getIdentifier() const
    {return identifier;}
  
  ExecutionTracePtr getExecutionTrace(ExecutionContext& context) const
    {return trace ? trace->createObjectAndCast<ExecutionTrace>(context) : ExecutionTracePtr();}

protected:
  friend class ExecutionTraceNetworkResponseClass;

  String identifier;
  XmlElementPtr trace;
};

typedef ReferenceCountedObjectPtr<ExecutionTraceNetworkResponse> ExecutionTraceNetworkResponsePtr;

class NetworkArchive : public Object
{
public:
  NetworkArchive(WorkUnitNetworkRequestPtr request, ExecutionTraceNetworkResponsePtr response)
    : request(request), response(response) {}
  NetworkArchive() {}
  
  const WorkUnitNetworkRequestPtr& getWorkUnitNetworkRequest() const
    {return request;}
  
  const ExecutionTraceNetworkResponsePtr& getExecutionTraceNetworkResponse() const
    {return response;}
  
protected:
  friend class NetworkArchiveClass;
  
  WorkUnitNetworkRequestPtr request;
  ExecutionTraceNetworkResponsePtr response;
};

typedef ReferenceCountedObjectPtr<NetworkArchive> NetworkArchivePtr;

extern ClassPtr workUnitNetworkRequestClass;
extern ClassPtr networkResponseClass;

}; /* namespace */

#endif // !LBCPP_NETWORK_REQUEST_H_
