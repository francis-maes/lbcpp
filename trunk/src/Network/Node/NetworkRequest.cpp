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
** WorkUnitInformation
*/
juce::int64 NetworkRequest::lastIdentifier = Time::currentTimeMillis();

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

NetworkRequest::NetworkRequest(ExecutionContext& context,
                               const String& projectName, const String& source, const String& destination, WorkUnitPtr workUnit,
                               size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : projectName(projectName), source(source), destination(destination), workUnit(new XmlElement()),
    requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime)
{
  this->workUnit->saveObject(context, workUnit);
}