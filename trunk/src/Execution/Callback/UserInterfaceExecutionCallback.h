/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceExecutionCallback.h| A UI based Execution Callback  |
| Author  : Francis Maes                   |                                 |
| Started : 28/11/2010 23:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_USER_INTERFACE_H_
# define LBCPP_EXECUTION_CALLBACK_USER_INTERFACE_H_

# include "../../UserInterface/Component/ExecutionTraceTreeView.h"

namespace lbcpp
{

class UserInterfaceExecutionCallbackMainWindow : public DocumentWindow
{
public:
  UserInterfaceExecutionCallbackMainWindow(Component* content) 
    : DocumentWindow("LBC++", Colours::whitesmoke, allButtons)
  {
    setVisible(true);
    setContentComponent(content);
    //setMenuBar(this);
    setResizable(true, true);
    centreWithSize(700, 600);
  }
  
  virtual void closeButtonPressed()
    {setVisible(false);}
};

class UserInterfaceExecutionCallback : public CompositeExecutionCallback
{
public:
  UserInterfaceExecutionCallback() : mainWindow(NULL), content(NULL) {}
  virtual ~UserInterfaceExecutionCallback()
    {shutdown();}

  virtual void initialize(ExecutionContext& context)
  {
    ExecutionCallback::initialize(context);
    userInterfaceManager().ensureIsInitialized(context);
    userInterfaceManager().callFunctionOnMessageThread(createWindowFunction, this);
  }

  void shutdown()
    {if (mainWindow) userInterfaceManager().callFunctionOnMessageThread(destroyWindowFunction, this);}

private:
  Component* mainWindow;
  Component* content;

  static void* createWindowFunction(void* userData)
  {
    UserInterfaceExecutionCallback* pthis = (UserInterfaceExecutionCallback* )userData;
    jassert(!pthis->content && !pthis->mainWindow);
    pthis->content =  new ExecutionTraceTreeView(pthis);
    pthis->mainWindow = new UserInterfaceExecutionCallbackMainWindow(pthis->content);
    return NULL;
  }

  static void* destroyWindowFunction(void* userData)
  {
    UserInterfaceExecutionCallback* pthis = (UserInterfaceExecutionCallback* )userData;
    pthis->clearCallbacks();
    if (pthis->mainWindow)
      deleteAndZero(pthis->mainWindow);
    pthis->content = NULL;
    return NULL;
  }
};

typedef ReferenceCountedObjectPtr<UserInterfaceExecutionCallback> UserInterfaceExecutionCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_EXECUTION_CALLBACK_USER_INTERFACE_H_

