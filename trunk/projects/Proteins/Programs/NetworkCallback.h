
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

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<NetworkCallback> NetworkCallbackPtr;

  class BufferedNetworkCallback : public NetworkCallback
{
public:
  Variable receiveVariable(bool blocking = true);
  
  /** NetworkConnect **/
  virtual void variableReceived(const Variable& variable);

protected:
  std::deque<Variable> variables;
  CriticalSection lock;
  
  Variable lockedPop();
};

typedef ReferenceCountedObjectPtr<BufferedNetworkCallback> BufferedNetworkCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NETWORK_CALLBACK_H_
