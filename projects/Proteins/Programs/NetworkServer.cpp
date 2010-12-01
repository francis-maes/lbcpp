
#include "NetworkServer.h"

using namespace lbcpp;

namespace lbcpp
{

class AcceptedNetworkClient : public NetworkClient
{
public:
  AcceptedNetworkClient(ExecutionContext& context) : NetworkClient(context) {}
  
  virtual bool startClient(const String& host, int port)
    {jassertfalse; return false;}
  
  virtual void stopClient()
    {jassertfalse;}
};
  
}; /* namespace */

/** NetworkServer **/
bool NetworkServer::startServer(int port)
{
  stopServer();
  return beginWaitingForSocket(port);
}

void NetworkServer::stopServer() {}

InterprocessConnection* NetworkServer::createConnectionObject()
{
  ScopedLock _(lock);
  NetworkClientPtr res = new AcceptedNetworkClient(context);
  acceptedClients.push_back(res);
  return res.get();
}

static void visualStudioWorkAroundSleep(int milliseconds)
  {Thread::sleep(milliseconds);}

NetworkClientPtr NetworkServer::acceptClient(bool blocking)
{
  
  if (!blocking)
    return lockedPop();

  NetworkClientPtr res;
  do
  {
    res = lockedPop();
    if (res)
      return res;
    visualStudioWorkAroundSleep(1000);
  } while (true);
}

NetworkClientPtr NetworkServer::lockedPop()
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
