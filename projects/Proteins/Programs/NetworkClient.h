
#ifndef LBCPP_NETWORK_CLIENT_H_
# define LBCPP_NETWORK_CLIENT_H_

# include "NetworkCallback.h"

namespace lbcpp
{

class NetworkClient : public InterprocessConnection
{
public:
  NetworkClient(ExecutionContext& context)
    : InterprocessConnection(false, magicNumber), context(context) {}
  virtual ~NetworkClient() {}

  virtual bool startClient(const String& host, int port) = 0;
  virtual void stopClient() = 0;

  bool sendVariable(const Variable& variable);
  void variableReceived(const Variable& variable);

  void appendCallback(NetworkCallbackPtr callback);
  void removeCallback(NetworkCallbackPtr callback);

  /* InterprocessConnection */
  virtual void connectionMade();
  virtual void connectionLost();
  virtual void messageReceived(const juce::MemoryBlock& message);

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
  std::vector<NetworkCallbackPtr> callbacks;
  CriticalSection lock;
};

typedef ReferenceCountedObjectPtr<NetworkClient> NetworkClientPtr;

extern NetworkClientPtr nonBlockingNetworkClient(ExecutionContext& context, bool autoReconnect = false);
extern NetworkClientPtr blockingNetworkClient(ExecutionContext& context, size_t numAttempts = 3);

  
class PrintNetworkCallback : public NetworkCallback
{
public:
  virtual void variableReceived(const Variable& variable)
  {std::cout << T("> ") + variable.toString() << std::endl;}
};
  
class ClientWorkUnit : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    NetworkClientPtr client = blockingNetworkClient(context, 3);
    BufferedNetworkCallbackPtr callback = new BufferedNetworkCallback();
    client->appendCallback(callback);
    if (client->startClient("192.168.1.3", 1664))
    {
      std::cout << "* Connected to " << client->getConnectedHostName() << std::endl;
      std::cout << "> " << callback->receiveVariable(true).toString() << std::endl;
      client->sendVariable(String(T("Hello")));
    }
    else
    {
      std::cout << "* Fail !" << std::endl;
    }
    client->stopClient();
    
    return true;
  }
};
  
  
  
  
}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_CLIENT_H_
