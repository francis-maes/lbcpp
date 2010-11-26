/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.h         | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_MANAGER_H_
# define LBCPP_USER_INTERFACE_MANAGER_H_

# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class UserInterfaceThread;
class UserInterfaceManager
{
public:
  UserInterfaceManager() : userInterfaceThread(NULL) {}
  ~UserInterfaceManager()
    {shutdown();}

  void ensureIsInitialized(ExecutionContext& context);
  bool isRunning() const;
  void shutdown();

  typedef void* (MessageCallbackFunction) (void* userData);

  void* callFunctionOnMessageThread(MessageCallbackFunction* callback, void* userData);

private:
  UserInterfaceThread* userInterfaceThread;
};

}; /* namespace smode */

#endif // !LBCPP_USER_INTERFACE_MANAGER_H_
