/*-----------------------------------------.---------------------------------.
| Filename: NetworkRequest.cpp             | Network Request                 |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NetworkRequest.h"

using namespace lbcpp;

/*
** NetworkRequest
*/

juce::int64 NetworkRequest::lastIdentifier = Time::currentTimeMillis();

NetworkRequest::NetworkRequest(const String& identifier, const String& projectName, const String& source, const String& destination)
  : identifier(identifier), projectName(projectName), source(source), destination(destination), status(unknown) {}
NetworkRequest::NetworkRequest(NetworkRequestPtr request)
  : identifier(request->identifier), projectName(request->projectName), source(request->source),
    destination(request->destination), status(request->status) {}

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

WorkUnitNetworkRequest::WorkUnitNetworkRequest(WorkUnitPtr workUnit, const String& projectName, const String& source, const String& destination)
  : NetworkRequest(generateIdentifier(), projectName, source, destination), workUnit(workUnit) {}
WorkUnitNetworkRequest::WorkUnitNetworkRequest(NetworkRequestPtr request, WorkUnitPtr workUnit)
  : NetworkRequest(request), workUnit(workUnit) {}
  
NetworkRequestPtr WorkUnitNetworkRequest::getNetworkRequest() const
  {return new NetworkRequest(getIdentifier(), getProjectName(), getSource(), getDestination());}
