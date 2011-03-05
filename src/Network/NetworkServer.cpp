/*-----------------------------------------.---------------------------------.
| Filename: NetworkServer.cpp              | Network Server                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkServer.h>
using namespace lbcpp;

namespace lbcpp
{

class AcceptedNetworkClient : public NetworkClient
{
public:
  AcceptedNetworkClient(ExecutionContext& context)
    : NetworkClient(context) {}
  
  virtual bool startClient(const String& host, int port)
    {jassertfalse; return false;}
  
  virtual void stopClient()
    {disconnect();}
};
  
}; /* namespace */

/** NetworkServer **/
bool NetworkServer::startServer(int port)
{
  stopServer();
  return beginWaitingForSocket(port);
}

void NetworkServer::stopServer()
  {stop();}

InterprocessConnection* NetworkServer::createConnectionObject()
{
  ScopedLock _(lock);
  NetworkClientPtr res = new AcceptedNetworkClient(context);
  acceptedClients.push_back(res);
  return res.get();
}

static void visualStudioWorkAroundSleep(int milliseconds)
  {juce::Thread::sleep(milliseconds);}

NetworkClientPtr NetworkServer::acceptClient(juce::int64 timeout)
{
  juce::int64 startTime = Time::getMillisecondCounter();
  while (true)
  {
    NetworkClientPtr res = popClient();
    if (res)
      return res;

    juce::int64 elapsedTime = Time::getMillisecondCounter() - startTime;
    if (elapsedTime > timeout)
      return NetworkClientPtr();

    int timeToSleep = juce::jlimit(0, 1000, (int)(timeout - elapsedTime));
    visualStudioWorkAroundSleep(timeToSleep);
  }
}

NetworkClientPtr NetworkServer::popClient()
{
  ScopedLock _(lock);
  NetworkClientPtr res;
  if (acceptedClients.size())
  {
    res = acceptedClients.front();
    acceptedClients.pop_front();
  }
  return res;
}
