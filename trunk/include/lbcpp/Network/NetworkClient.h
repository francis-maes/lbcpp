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

namespace lbcpp
{

class NetworkClient : public InterprocessConnection, public Object
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
  std::deque<Variable> variables;
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

};

#endif //!LBCPP_NETWORK_CLIENT_H_
