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
** WorkUnitInformation
*/
juce::int64 WorkUnitInformation::lastIdentifier = Time::currentTimeMillis();

WorkUnitInformation::WorkUnitInformation(const String& projectName, const String& source, const String& destination,
                                         size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : projectName(projectName), source(source), destination(destination),
    requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime),
    status(communicationError)
{
}

String WorkUnitInformation::generateIdentifier()
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
