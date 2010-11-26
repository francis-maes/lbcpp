/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.cpp       | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/UserInterface/UserInterfaceManager.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

namespace lbcpp
{

class UserInterfaceThread : public Thread
{
public:
  UserInterfaceThread()
    : Thread(T("Juce Message Thread")), commandManager(NULL), initialized(false) {}
  virtual ~UserInterfaceThread()
    {}

  virtual void run()
  {
    juce::initialiseJuce_GUI();

    juce::MessageManager* messageManager = juce::MessageManager::getInstance();
    messageManager->setCurrentMessageThread(getThreadId());
    commandManager = new juce::ApplicationCommandManager();

    initialized = true;
    while (!threadShouldExit())
      if (!messageManager->runDispatchLoopUntil(100) && juce::Desktop::getInstance().getNumComponents() == 0)
        break;

    juce::Desktop& desktop = juce::Desktop::getInstance();
    jassert(!desktop.getNumComponents());

    deleteAndZero(commandManager);
#if JUCE_MAC
    const ScopedAutoReleasePool pool;
#endif
    {
      juce::DeletedAtShutdown::deleteAll();
      juce::LookAndFeel::clearDefaultLookAndFeel();
    }
    delete juce::MessageManager::getInstance();
  }

  juce::ApplicationCommandManager& getCommandManager()
    {jassert(commandManager); return *commandManager;}

  bool isInitialized() const
    {return initialized;}

protected:
  juce::ApplicationCommandManager* commandManager;
  bool mutable initialized;
};

}; /* namespace lbcpp */

void UserInterfaceManager::ensureIsInitialized(ExecutionContext& context)
{
  if (!userInterfaceThread)
    userInterfaceThread = new UserInterfaceThread();

  if (!userInterfaceThread->isThreadRunning())
  {
    userInterfaceThread->startThread();
    while (!userInterfaceThread->isInitialized())
      Thread::sleep(1);
  }
}

bool UserInterfaceManager::isRunning() const
  {return userInterfaceThread && userInterfaceThread->isThreadRunning();}

void UserInterfaceManager::shutdown()
{
  if (isRunning())
  {
    userInterfaceThread->stopThread(2000);
    deleteAndZero(userInterfaceThread);
  }
}

void* UserInterfaceManager::callFunctionOnMessageThread(MessageCallbackFunction* callback, void* userData)
{
  jassert(isRunning());
  return juce::MessageManager::getInstance()->callFunctionOnMessageThread(callback, userData);
}
