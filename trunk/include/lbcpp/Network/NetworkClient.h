/*-----------------------------------------.---------------------------------.
| Filename: NetworkClient.h                | Network Client                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_CLIENT_H_
# define LBCPP_NETWORK_CLIENT_H_

# include <lbcpp/Network/NetworkCallback.h>
# include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

class NetworkClient : public Object, public InterprocessConnection
{
public:
  NetworkClient(ExecutionContext& context)
    : InterprocessConnection(false, magicNumber), context(context), disconnected(true) {}
  virtual ~NetworkClient() {}

  virtual bool startClient(const String& host, int port) = 0;
  virtual void stopClient() = 0;

  bool sendVariable(const Variable& variable);
  bool receiveVariable(juce::int64 timeout, Variable& result);
  template<class O>
  bool receiveObject(juce::int64 timeout, ReferenceCountedObjectPtr<O>& result);
  bool receiveBoolean(juce::int64 timeout, bool& result);
  bool receiveString(juce::int64 timeout, String& result);
  bool receiveInteger(juce::int64 timeout, int& result);

  bool hasVariableInQueue();

  void appendCallback(NetworkCallbackPtr callback);
  void removeCallback(NetworkCallbackPtr callback);

  /* InterprocessConnection */
  virtual void connectionMade();
  virtual void connectionLost();
  virtual void messageReceived(const juce::MemoryBlock& message);

  lbcpp_UseDebuggingNewOperator

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
  std::deque<Variable> variables; // receivedVariables
  std::vector<NetworkCallbackPtr> callbacks;
  bool disconnected;
  CriticalSection lock;
  
  void variableReceived(const Variable& variable);

  bool popVariable(Variable& result);
  void pushVariable(const Variable& variable);
};

typedef ReferenceCountedObjectPtr<NetworkClient> NetworkClientPtr;

extern NetworkClientPtr nonBlockingNetworkClient(ExecutionContext& context, bool autoReconnect = false);
extern NetworkClientPtr blockingNetworkClient(ExecutionContext& context, size_t numAttempts = 3);

template<class O>
inline bool NetworkClient::receiveObject(juce::int64 timeout, ReferenceCountedObjectPtr<O>& result)
{
  Variable v;
  if (!receiveVariable(timeout, v))
    return false;
  
  if (v.isNil())
  {
    result = ReferenceCountedObjectPtr<O>();
    return true;
  }
  
  if (!v.isObject())
    return false;

  result = v.getObjectAndCast<O>();
  return true;
}

/** XxxNetworkClient **/
class XxxNetworkClient : public Object, public InterprocessConnection
{
public:
  XxxNetworkClient(ExecutionContext& context);

  bool startClient(const String& host, int port);
  void stopClient();

  bool sendVariable(const Variable& variable);
  virtual void variableReceived(const Variable& variable) = 0;

  ExecutionContext& getContext()
    {return context;}

  /* InterprocessConnection */
  virtual void connectionMade() {}
  virtual void connectionLost() {}

  lbcpp_UseDebuggingNewOperator
  
protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;

  virtual void messageReceived(const juce::MemoryBlock& message);
};

typedef ReferenceCountedObjectPtr<XxxNetworkClient> XxxNetworkClientPtr;
class XxxManagerNetworkClientCallback
{
public:
  virtual ~XxxManagerNetworkClientCallback() {}

  virtual void workUnitAcknowledgementReceived(size_t sourceIdentifier, const String& uniqueIdentifier) = 0;
  virtual void workUnitResultReceived(const String& uniqueIdentifier, const Variable& result) = 0;
};

typedef XxxManagerNetworkClientCallback* XxxManagerNetworkClientCallbackPtr;

class XxxManagerNetworkClient : public XxxNetworkClient
{
public:
  XxxManagerNetworkClient(ExecutionContext& context, const XxxManagerNetworkClientCallbackPtr& callback)
    : XxxNetworkClient(context), callback(callback)
    {jassert(callback);}

  virtual bool sendWorkUnit(const WorkUnitPtr& workUnit, size_t sourceIdentifier) = 0;

protected:
  XxxManagerNetworkClientCallbackPtr callback;

  virtual void variableReceived(const Variable& variable);
};

typedef ReferenceCountedObjectPtr<XxxManagerNetworkClient> XxxManagerNetworkClientPtr;

class XxxGridNetworkClientCallback
{
public:
  virtual ~XxxGridNetworkClientCallback() {}

  virtual void workUnitRequestReceived(size_t sourceIdentifier, const XmlElementPtr& xmlWorkUnit) = 0;
};

typedef XxxGridNetworkClientCallback* XxxGridNetworkClientCallbackPtr;

class XxxGridNetworkClient : public XxxNetworkClient
{
public:
  XxxGridNetworkClient(ExecutionContext& context, const XxxGridNetworkClientCallbackPtr& callback)
    : XxxNetworkClient(context), callback(callback)
    {jassert(callback);}

  virtual bool sendWorkUnitAcknowledgement(size_t sourceIdentifier, const String& uniqueIdentifier) = 0;
  virtual bool sendWorkUnitResult(const String& uniqueIdentifier, const Variable& result) = 0;

protected:
  XxxGridNetworkClientCallbackPtr callback;

  virtual void variableReceived(const Variable& variable);
};

typedef ReferenceCountedObjectPtr<XxxGridNetworkClient> XxxGridNetworkClientPtr;

};

#endif //!LBCPP_NETWORK_CLIENT_H_
