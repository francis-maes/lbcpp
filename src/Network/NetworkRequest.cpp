/*-----------------------------------------.---------------------------------.
| Filename: NetworkRequest.cpp             | Network Request                 |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Network/NetworkRequest.h>

using namespace lbcpp;

/*
** NetworkRequest
*/

juce::int64 NetworkRequest::lastIdentifier = Time::currentTimeMillis();

NetworkRequest::NetworkRequest(const String& identifier, const String& projectName, const String& source, const String& destination, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : identifier(identifier), projectName(projectName), source(source), destination(destination),
    requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime), status(unknown) {}

NetworkRequest::NetworkRequest(NetworkRequestPtr request)
  : identifier(request->identifier), projectName(request->projectName), source(request->source),
    destination(request->destination), requiredCpus(request->requiredCpus),
    requiredMemory(request->requiredMemory), requiredTime(request->requiredTime), status(request->status) {}

String NetworkRequest::generateIdentifier()
{
  juce::int64 res = Time::currentTimeMillis();
  if (res != lastIdentifier)
  {
    lastIdentifier = res;
    return String(res);
  }
  juce::Thread::sleep(1);
  return generateIdentifier();
}

/*
** WorkUnitNetworkRequest
*/

WorkUnitNetworkRequest::WorkUnitNetworkRequest(ExecutionContext& context, WorkUnitPtr workUnit, const String& projectName, const String& source, const String& destination, size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : NetworkRequest(generateIdentifier(), projectName, source, destination, requiredCpus, requiredMemory, requiredTime), context(context), workUnitXmlElement(new XmlElement())
{workUnitXmlElement->saveObject(context, workUnit);}

WorkUnitNetworkRequest::WorkUnitNetworkRequest(ExecutionContext& context, NetworkRequestPtr request, WorkUnitPtr workUnit)
  : NetworkRequest(request), context(context), workUnitXmlElement(new XmlElement())
{workUnitXmlElement->saveObject(context, workUnit);}
  
NetworkRequestPtr WorkUnitNetworkRequest::getNetworkRequest() const
  {return new NetworkRequest(getIdentifier(), getProjectName(), getSource(), getDestination(), requiredCpus, requiredMemory, requiredTime);}
