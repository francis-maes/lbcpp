/*-----------------------------------------.---------------------------------.
| Filename: NetworkMessage.h               | Network Message                 |
| Author  : Julien Becker                  |                                 |
| Started : 10/08/2011 14:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_MESSAGE_H_
# define LBCPP_NETWORK_MESSAGE_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Core/XmlSerialisation.h>

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

class WorkUnitNetworkMessage : public NetworkMessage
{
public:
  WorkUnitNetworkMessage(ExecutionContext& context, size_t sourceIdentifier, const WorkUnitPtr& workUnit)
    : sourceIdentifier(sourceIdentifier), workUnit(new XmlElement())
    {this->workUnit->saveObject(context, workUnit, T("workUnit"));}

  size_t getSourceIdentifier() const
    {return sourceIdentifier;}

  WorkUnitPtr getWorkUnit(ExecutionContext& context) const
    {return workUnit->createObjectAndCast<WorkUnit>(context);}

  XmlElementPtr getXmlElementWorkUnit() const
    {return workUnit;}

protected:
  friend class WorkUnitNetworkMessageClass;
  
  WorkUnitNetworkMessage() {}

  size_t sourceIdentifier;
  XmlElementPtr workUnit;
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkMessage> WorkUnitNetworkMessagePtr;

extern ClassPtr workUnitNetworkMessageClass;

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

  WorkUnitAcknowledgementNetworkMessage() {}

  size_t sourceIdentifier;
  String uniqueIdentifier;
};

typedef ReferenceCountedObjectPtr<WorkUnitAcknowledgementNetworkMessage> WorkUnitAcknowledgementNetworkMessagePtr;

extern ClassPtr workUnitAcknowledgementNetworkMessageClass;

class WorkUnitResultNetworkMessage : public NetworkMessage
{
public:
  WorkUnitResultNetworkMessage(ExecutionContext& context, const String& uniqueIdentifier, const Variable& result)
    : uniqueIdentifier(uniqueIdentifier), result(new XmlElement())
    {this->result->saveVariable(context, result, T("result"));}

  String getUniqueIdentifier() const
    {return uniqueIdentifier;}

  Variable getResult(ExecutionContext& context) const
    {return result->createVariable(context);}

  XmlElementPtr getXmlElementResult() const
    {return result;}

protected:
  friend class WorkUnitResultNetworkMessageClass;

  WorkUnitResultNetworkMessage() {}

  String uniqueIdentifier;
  XmlElementPtr result;
};

typedef ReferenceCountedObjectPtr<WorkUnitResultNetworkMessage> WorkUnitResultNetworkMessagePtr;

extern ClassPtr workUnitResultNetworkMessageClass;

}; /* namespace lbcpp */

#endif // !LBCPP_NETWORK_MESSAGE_H_
