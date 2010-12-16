/*-----------------------------------------.---------------------------------.
| Filename: NetworkCallback.h              | Network Callback                |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_CALLBACK_H_
# define LBCPP_NETWORK_CALLBACK_H_

# include <lbcpp/Core/Variable.h>
# include <deque>

namespace lbcpp
{

class NetworkCallback : public Object
{
public:
  virtual ~NetworkCallback() {}
  virtual void variableReceived(const Variable& variable) = 0;
  virtual void disconnected() = 0;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<NetworkCallback> NetworkCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NETWORK_CALLBACK_H_
