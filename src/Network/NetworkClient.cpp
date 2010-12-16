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

bool NetworkClient::receiveVariable(juce::int64 timeout, Variable& result)
{
  std::cout << "NetworkClient - receiveVariable" << std::endl;
  juce::int64 startTime = Time::getMillisecondCounter();
  while (true)
  {
    Variable res;
    if (popVariable(res))
    {
      result = res;
      return true;
    }
    
    juce::int64 elapsedTime = Time::getMillisecondCounter() - startTime;
    if (elapsedTime >= timeout || disconnected)
      return false;
    std::cout << "NetworkClient - time left: " << timeout - elapsedTime << std::endl;
    juce::int64 timeToSleep = juce::jlimit<juce::int64>((juce::int64)0, (juce::int64)1000, timeout - elapsedTime);
    if (!timeToSleep)
      return false;
    juce::Thread::sleep(timeToSleep);
  }
}

template<class O>
bool NetworkClient::receiveObject(juce::int64 timeout, ReferenceCountedObjectPtr<O>& result)
{
  Variable v;
  if (!receiveVariable(timeout, v))
    return false;
  
  if (!v.isObject())
    return false;
  
  result = v.getObjectAndCast<O>();
  return true;
}

bool NetworkClient::receiveBoolean(juce::int64 timeout, bool& result)
{
  Variable v;
  if (!receiveVariable(timeout, v))
    return false;
  
  if (!v.isBoolean())
    return false;
  
  result = v.getBoolean();
  return true;
}

bool NetworkClient::receiveString(juce::int64 timeout, String& result)
{
  Variable v;
  if (!receiveVariable(timeout, v))
    return false;
  
  if (!v.isString())
    return false;
  
  result = v.getString();
  return true;
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
  disconnected = false;
}

void NetworkClient::connectionLost()
{
  disconnected = true;
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->disconnected();
}

void NetworkClient::messageReceived(const juce::MemoryBlock& message)
{
  juce::XmlDocument document(message.toString());
  XmlImporter importer(context, document);
  variableReceived(importer.isOpened() ? importer.load() : Variable());
}

void NetworkClient::variableReceived(const Variable& variable)
{
  std::cout << "Variable received: " << variable.toString() << std::endl;
  pushVariable(variable);
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->variableReceived(variable);
}

bool NetworkClient::popVariable(Variable& result)
{
  ScopedLock _(lock);
  if (!variables.size())
    return false;
  result = variables.front();
  variables.pop_front();
  return true;
}

void NetworkClient::pushVariable(const Variable& variable)
{
  ScopedLock _(lock);
  variables.push_back(variable);
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
      if (connectToSocket(host, port, 5000))
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
