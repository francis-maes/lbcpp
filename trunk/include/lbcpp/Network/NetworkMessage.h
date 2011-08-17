/*-----------------------------------------.---------------------------------.
| Filename: NetworkMessage.h               | Network Message                 |
| Author  : Julien Becker                  |                                 |
| Started : 10/08/2011 14:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_MESSAGE_H_
# define LBCPP_NETWORK_MESSAGE_H_

# include <lbcpp/Core/XmlSerialisation.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include <lbcpp/Network/WorkUnitNetworkRequest.h>

namespace lbcpp
{

class NetworkMessage : public Object
{
public:
  lbcpp_UseDebuggingNewOperator

protected:
  NetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<NetworkMessage> NetworkMessagePtr;

extern ClassPtr networkMessageClass;

class WorkUnitRequestNetworkMessage : public NetworkMessage
{
public:
  WorkUnitRequestNetworkMessage(ExecutionContext& context, size_t sourceIdentifier,
                                const WorkUnitPtr& workUnit,
                                const String& projectName, const String& source, const String& destination,
                                size_t requiredCpus = 1, size_t requiredMemory = 2, size_t requiredTime = 10)
    : sourceIdentifier(sourceIdentifier),
      request(new WorkUnitNetworkRequest(context,
                                         workUnit,
                                         projectName, source, destination,
                                         requiredCpus, requiredMemory, requiredTime))
    {}

  size_t getSourceIdentifier() const
    {return sourceIdentifier;}

  WorkUnitNetworkRequestPtr getWorkUnitNetworkRequest() const
    {return request;}

protected:
  friend class WorkUnitRequestNetworkMessageClass;

  size_t sourceIdentifier;
  WorkUnitNetworkRequestPtr request;
  
  WorkUnitRequestNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<WorkUnitRequestNetworkMessage> WorkUnitRequestNetworkMessagePtr;
extern ClassPtr workUnitRequestNetworkMessageClass;

class WorkUnitAcknowledgementNetworkMessage : public NetworkMessage
{
public:
  WorkUnitAcknowledgementNetworkMessage(size_t sourceIdentifier, const String& uniqueIdentifier)
    : sourceIdentifier(sourceIdentifier), uniqueIdentifier(uniqueIdentifier) {}

  size_t getSourceIdentifier() const
    {return sourceIdentifier;}

  String getUniqueIdentifier() const
    {return uniqueIdentifier;}

protected:
  friend class WorkUnitAcknowledgementNetworkMessageClass;

  size_t sourceIdentifier;
  String uniqueIdentifier;

  WorkUnitAcknowledgementNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<WorkUnitAcknowledgementNetworkMessage> WorkUnitAcknowledgementNetworkMessagePtr;
extern ClassPtr workUnitAcknowledgementNetworkMessageClass;

class WorkUnitResultNetworkMessage : public NetworkMessage
{
public:
  WorkUnitResultNetworkMessage(ExecutionContext& context, const String& uniqueIdentifier, const Variable& result)
    : uniqueIdentifier(uniqueIdentifier), result(new XmlElement())
    {this->result->saveVariable(context, result, T("result"));}

  WorkUnitResultNetworkMessage(ExecutionContext& context, const String& uniqueIdentifier, const XmlElementPtr& result)
    : uniqueIdentifier(uniqueIdentifier), result(result) {}

  String getUniqueIdentifier() const
    {return uniqueIdentifier;}

  Variable getResult(ExecutionContext& context) const
    {return result->createVariable(context);}

  XmlElementPtr getXmlElementResult() const
    {return result;}

protected:
  friend class WorkUnitResultNetworkMessageClass;

  String uniqueIdentifier;
  XmlElementPtr result;

  WorkUnitResultNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<WorkUnitResultNetworkMessage> WorkUnitResultNetworkMessagePtr;
extern ClassPtr workUnitResultNetworkMessageClass;

class GetWorkUnitResultNetworkMessage : public NetworkMessage
{
public:
  GetWorkUnitResultNetworkMessage(const String& uniqueIdentifier)
    : uniqueIdentifier(uniqueIdentifier) {}

  String getUniqueIdentifier() const
    {return uniqueIdentifier;}

protected:
  friend class GetWorkUnitResultNetworkMessageClass;

  String uniqueIdentifier;

  GetWorkUnitResultNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<GetWorkUnitResultNetworkMessage> GetWorkUnitResultNetworkMessagePtr;
extern ClassPtr getWorkUnitResultNetworkMessageClass;

class GetWaitingWorkUnitsNetworkMessage : public NetworkMessage
{
public:
  GetWaitingWorkUnitsNetworkMessage(const String& gridName)
    : gridName(gridName) {}

  String getGridName() const
    {return gridName;}

protected:
  friend class GetWaitingWorkUnitsNetworkMessageClass;

  String gridName;

  GetWaitingWorkUnitsNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<GetWaitingWorkUnitsNetworkMessage> GetWaitingWorkUnitsNetworkMessagePtr;
extern ClassPtr getWaitingWorkUnitsNetworkMessageClass;

class WorkUnitRequestsNetworkMessage : public NetworkMessage
{
public:
  WorkUnitRequestsNetworkMessage(const std::vector<WorkUnitNetworkRequestPtr>& workUnitRequests)
    : requests(workUnitRequests) {}

  const std::vector<WorkUnitNetworkRequestPtr>& getWorkUnitRequests() const
    {return requests;}

protected:
  friend class WorkUnitRequestsNetworkMessageClass;

  std::vector<WorkUnitNetworkRequestPtr> requests;

  WorkUnitRequestsNetworkMessage() {}
};

typedef ReferenceCountedObjectPtr<WorkUnitRequestsNetworkMessage> WorkUnitRequestsNetworkMessagePtr;
extern ClassPtr workUnitRequestsNetworkMessageClass;

class ExecutionTracesNetworkMessage : public NetworkMessage
{
public:
  const std::vector< std::pair<String, XmlElementPtr> >& getXmlElementExecutionTraces() const
    {return traces;}

  void addExecutionTrace(ExecutionContext& context, const String& uniqueIdentifier, const ExecutionTracePtr& trace)
  {
    XmlElementPtr element = new XmlElement();
    element->saveObject(context, trace, T("trace"));
    traces.push_back(std::pair<String, XmlElementPtr>(uniqueIdentifier, element));
  }

protected:
  friend class ExecutionTracesNetworkMessageClass;

  std::vector< std::pair<String, XmlElementPtr> > traces;
};

typedef ReferenceCountedObjectPtr<ExecutionTracesNetworkMessage> ExecutionTracesNetworkMessagePtr;
extern ClassPtr executionTracesNetworkMessageClass;

class CloseCommunicationNetworkMessage : public NetworkMessage
  {};

typedef ReferenceCountedObjectPtr<CloseCommunicationNetworkMessage> CloseCommunicationNetworkMessagePtr;
extern ClassPtr closeCommunicationNetworkMessageClass;

}; /* namespace lbcpp */

#endif // !LBCPP_NETWORK_MESSAGE_H_
