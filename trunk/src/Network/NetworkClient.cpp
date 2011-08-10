/*-----------------------------------------.---------------------------------.
| Filename: NetworkClient.cpp              | Network Client                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
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
#ifdef JUCE_DEBUG
  context.informationCallback(T("sendVariable"), text);
#endif // !JUCE_DEBUG
  juce::MemoryBlock block(text.toUTF8(), text.length());
  return sendMessage(block);
}

bool NetworkClient::receiveVariable(juce::int64 timeout, Variable& result)
{
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
#ifdef JUCE_DEBUG
    context.informationCallback(T("receiveVariable"), T("NetworkClient - time left: ") + String((timeout - elapsedTime) / 1000) + T(" s"));
#endif // !JUCE_DEBUG
    juce::int64 timeToSleep = juce::jlimit<juce::int64>((juce::int64)0, (juce::int64)1000, timeout - elapsedTime);
    if (!timeToSleep)
      return false;
    juce::Thread::sleep((int)timeToSleep);
  }
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

bool NetworkClient::receiveInteger(juce::int64 timeout, int& result)
{
  Variable v;
  if (!receiveVariable(timeout, v))
    return false;
  
  if (!v.isInteger())
    return false;
  
  result = v.getInteger();
  return true;
}

bool NetworkClient::hasVariableInQueue()
{
  ScopedLock _(lock);
  return variables.size() > 0;
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
  ScopedLock _(lock);
#ifdef JUCE_DEBUG
  context.informationCallback(T("messageReceived"), message.toString());
#endif // !JUCE_DEBUG
  juce::XmlDocument document(message.toString());
  XmlImporter importer(context, document);
  variableReceived(importer.isOpened() ? importer.load() : Variable());
}

void NetworkClient::variableReceived(const Variable& variable)
{
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
    ScopedLock _(lock);
    for (size_t i = 0; i < numAttempts || !numAttempts; ++i)
    {
      if (connectToSocket(host, port, 1000))
        return true;
    }
    return false;
  }

  virtual void stopClient()
  {
    ScopedLock _(lock);
    disconnect();
  }
  
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
    : Thread(T("Network ") + host + T(":") + String(port)),
      client(client), host(host), port(port),
      autoReconnect(autoReconnect), connected(false) {}
  
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
  bool autoReconnect;
  bool connected;
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

/** XxxNetworkClient **/
XxxNetworkClient::XxxNetworkClient(ExecutionContext& context)
  : InterprocessConnection(false, magicNumber), context(context), keepAlive(true) {}

bool XxxNetworkClient::startClient(const String& host, int port)
{
  lastHostName = host;
  lastPort = port;

  keepAlive = true;
  size_t timeToSleep = 1000;
  while (!connectToSocket(host, port, 1000))
  {
    context.warningCallback(T("XxxNetworkClient::startClient"), T("Connection to ")
                            + host + T(" at port ") + String((int)port) 
                            + (" failed ! Retry in ") + String((int)timeToSleep / 1000) + T("s"));
    juce::Thread::sleep(timeToSleep);
    timeToSleep *= 2;
  }
  return isConnected();
}

void XxxNetworkClient::stopClient()
{
  keepAlive = false;
  disconnect();
}

bool XxxNetworkClient::sendVariable(const Variable& variable)
{
  XmlExporter exporter(context);
  exporter.saveVariable(String::empty, variable, TypePtr());
  String text = exporter.toString();
  if (text == String::empty)
    return false;
#ifdef JUCE_DEBUG
  context.informationCallback(T("sendVariable"), text);
#endif // !JUCE_DEBUG
  juce::MemoryBlock block(text.toUTF8(), text.length());
  return sendMessage(block);
}

void XxxNetworkClient::messageReceived(const juce::MemoryBlock& message)
{
#ifdef JUCE_DEBUG
  context.informationCallback(T("messageReceived"), message.toString());
#endif // !JUCE_DEBUG
  juce::XmlDocument document(message.toString());
  XmlImporter importer(context, document);
  variableReceived(importer.isOpened() ? importer.load() : Variable());
}

void XxxNetworkClient::connectionLost()
{
  if (keepAlive)
  {
    context.warningCallback(T("XxxNetworkClient::connectionLost"), T("Connection lost ! Trying to reconnect."));
    startClient(lastHostName, lastPort);
  }
}
