
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

};

#endif // !LBCPP_NETWORK_CALLBACK_H_
