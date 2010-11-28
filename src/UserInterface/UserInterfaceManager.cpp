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
using juce::Desktop;

namespace lbcpp
{

class UserInterfaceThread : public Thread
{
public:
  UserInterfaceThread(NotificationQueuePtr notifications)
    : Thread(T("Juce Message Thread")), notifications(notifications), commandManager(NULL), initialized(false) {}
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
    {
      if (!messageManager->runDispatchLoopUntil(100) && juce::Desktop::getInstance().getNumComponents() == 0)
        break;
      notifications->flush();
    }
    notifications->flush();

    Desktop& desktop = Desktop::getInstance();
    jassert(!desktop.getNumComponents());

    deleteAndZero(commandManager);
#if JUCE_MAC
    const juce::ScopedAutoReleasePool pool;
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
  NotificationQueuePtr notifications;
  juce::ApplicationCommandManager* commandManager;
  bool mutable initialized;
};

}; /* namespace lbcpp */

UserInterfaceManager::UserInterfaceManager() : userInterfaceThread(NULL), notifications(new NotificationQueue())
{
}

UserInterfaceManager::~UserInterfaceManager()
  {shutdown();}

void UserInterfaceManager::ensureIsInitialized(ExecutionContext& context)
{
  if (!userInterfaceThread)
    userInterfaceThread = new UserInterfaceThread(notifications);

  if (!userInterfaceThread->isThreadRunning())
  {
    userInterfaceThread->startThread();
    while (!userInterfaceThread->isInitialized())
      Thread::sleep(1);
  }
}

bool UserInterfaceManager::isRunning() const
  {return userInterfaceThread && userInterfaceThread->isThreadRunning();}

bool UserInterfaceManager::hasAtLeastOneVisibleWindowOnDesktop() const
{
  if (!isRunning())
    return false;

  Desktop& desktop = Desktop::getInstance();
  for (int i = 0; i < desktop.getNumComponents(); ++i)
    if (desktop.getComponent(i)->isVisible())
      return true;

  return false;
}

void UserInterfaceManager::waitUntilAllWindowsAreClosed()
{
  while (hasAtLeastOneVisibleWindowOnDesktop())
    Thread::sleep(100);
}

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
