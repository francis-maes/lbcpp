
#ifndef LBCPP_NETWORK_SERVER_H_
# define LBCPP_NETWORK_SERVER_H_

# include "NetworkClient.h"

namespace lbcpp
{

class NetworkServer : public InterprocessConnectionServer
{
public:
  NetworkServer(ExecutionContext& context) : context(context) {}
  
  bool startServer(int port);
  void stopServer();

  NetworkClientPtr acceptClient(bool blocking);

  juce_UseDebuggingNewOperator

protected:
  /* InterprocessConnectionServer */
  virtual InterprocessConnection* createConnectionObject();

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
  std::deque<NetworkClientPtr> acceptedClients;
  CriticalSection lock;
  
  NetworkClientPtr lockedPop();
};

typedef ReferenceCountedObjectPtr<NetworkServer> NetworkServerPtr;
  
class ServerWorkUnit : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    NetworkServerPtr server = new NetworkServer(context);
    server->startServer(1664);

    NetworkClientPtr client = server->acceptClient(true);
    std::cout << "* New client: " << client->getConnectedHostName() << std::endl;

    BufferedNetworkCallbackPtr callback = new BufferedNetworkCallback();
    client->appendCallback(callback);
    
    client->sendVariable(String(T("Welcome :-)")));
    
    std::cout << "> " << callback->receiveVariable(true).toString() << std::endl;
    
    client->stopClient();
    server->stopServer();

    return true;
  }
};
  
  
}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_SERVER_H_
