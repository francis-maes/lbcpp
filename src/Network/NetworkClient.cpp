/*-----------------------------------------.---------------------------------.
| Filename: NetworkClient.cpp              | Network Client                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Network/NetworkClient.h>
#include <lbcpp/Core/XmlSerialisation.h>

using namespace lbcpp;

/** NetworkClient **/
bool NetworkClient::sendVariable(const Variable& variable)
{
  XmlExporter exporter(context);
  exporter.saveVariable(String::empty, variable, TypePtr());
  String text = exporter.toString();
  if (text == String::empty)
    return false;
  juce::MemoryBlock block(text.toUTF8(), text.length());
  return sendMessage(block);
}

void NetworkClient::variableReceived(const Variable& variable)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->variableReceived(variable);
}

void NetworkClient::appendCallback(NetworkCallbackPtr callback)
{
  ScopedLock _(lock);
  callbacks.push_back(callback);
}

void NetworkClient::removeCallback(NetworkCallbackPtr callback)
{
  ScopedLock _(lock);
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      return;
    }
}

void NetworkClient::connectionMade()
{
  // transmit to callbacks
}

void NetworkClient::connectionLost()
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->disconnected();
}

void NetworkClient::messageReceived(const juce::MemoryBlock& message)
{
  juce::XmlDocument document(message.toString());
  XmlImporter importer(context, document);
  variableReceived(importer.isOpened() ? importer.load() : Variable());
}

namespace lbcpp
{

/** BlockingNetworkClient **/
class BlockingNetworkClient : public NetworkClient
{
public:
  // 0 = infinite number of attempts
  BlockingNetworkClient(ExecutionContext& context, size_t numAttempts)
    : NetworkClient(context), numAttempts(numAttempts) {}
  
  virtual bool startClient(const String& host, int port)
  {
    for (size_t i = 0; i < numAttempts || !numAttempts; ++i)
    {
      if (connectToSocket(host, port, 10000))
        return true;
    }
    return false;
  }

  virtual void stopClient()
    {disconnect();}
  
protected:
  size_t numAttempts;
};

NetworkClientPtr blockingNetworkClient(ExecutionContext& context, size_t numAttempts)
  {return new BlockingNetworkClient(context, numAttempts);}

/** NonBlockingNetworkClient **/
class NetworkConnectionThread : public Thread
{
public:
  NetworkConnectionThread(NetworkClientPtr client, const String& host, int port, bool autoReconnect)
    : Thread(T("Network ") + host + T(":") + String(port)), client(client), host(host), port(port), connected(false), autoReconnect(autoReconnect) {}

  void disconnected()
    {connected = false;}

  virtual void run()
  {
    disconnected();
    while (!threadShouldExit())
    {
      if (connected)
      {
        if (!autoReconnect)
          return;
        sleep(1000);
      }
      else if (client->connectToSocket(host, port, 1000))
        connected = true;
    }
  }

protected:
  NetworkClientPtr client;
  String host;
  int port;
  bool connected;
  bool autoReconnect;
};

typedef ReferenceCountedObjectPtr<NetworkConnectionThread> NetworkConnectionThreadPtr;

class NonBlockingNetworkClient : public NetworkClient
{
public:
  NonBlockingNetworkClient(ExecutionContext& context, bool autoReconnect)
    : NetworkClient(context), autoReconnect(autoReconnect) {}

  virtual ~NonBlockingNetworkClient()
    {stopClient();}
  
  virtual bool startClient(const String& host, int port)
  {
    stopClient();
    connectionThread = new NetworkConnectionThread(this, host, port, autoReconnect);
    connectionThread->startThread();
    return true;
  }
  
  virtual void stopClient()
  {
    if (connectionThread->isThreadRunning())
      connectionThread->stopThread(2000);
    if (connectionThread)
      connectionThread = NetworkConnectionThreadPtr();
    disconnect();
  }

  virtual void connectionLost()
  {
    NetworkClient::connectionLost();
    if (!autoReconnect)
      stopClient();
    else
      connectionThread->disconnected();
  }

protected:
  bool autoReconnect;
  NetworkConnectionThreadPtr connectionThread;
};

NetworkClientPtr nonBlockingNetworkClient(ExecutionContext& context, bool autoReconnect)
  {return new NonBlockingNetworkClient(context, autoReconnect);}

}; /* namespace lbcpp */
