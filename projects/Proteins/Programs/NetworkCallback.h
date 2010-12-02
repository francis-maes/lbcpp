/*-----------------------------------------.---------------------------------.
| Filename: NetworkCallback.h              | Network Callback                |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_CALLBACK_H_
# define LBCPP_NETWORK_CALLBACK_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class NetworkCallback
{
public:
  virtual ~NetworkCallback() {}
  virtual void variableReceived(const Variable& variable) = 0;
  virtual void disconnected() = 0;

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<NetworkCallback> NetworkCallbackPtr;

  class BufferedNetworkCallback : public NetworkCallback
{
public:
  BufferedNetworkCallback() : connected(true) {}
  
  Variable receiveVariable(bool blocking = true);
  
  /** NetworkConnect **/
  virtual void variableReceived(const Variable& variable);
  
  virtual void disconnected();

protected:
  std::deque<Variable> variables;
  bool connected;
  CriticalSection lock;
  
  Variable lockedPop();
};

typedef ReferenceCountedObjectPtr<BufferedNetworkCallback> BufferedNetworkCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NETWORK_CALLBACK_H_
