/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.cpp       | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/Library.h>
#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/UserInterface/UserInterfaceManager.h>
#include <lbcpp/library.h>
using namespace lbcpp;

using juce::Desktop;
using juce::Image;
using juce::ImageCache;

namespace UserInterfaceData {extern const char* get(const String& fileName, int& size);};

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
      notifications->flush(ObjectPtr());
    }
    notifications->flush(ObjectPtr());

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

Image* UserInterfaceManager::getImage(const String& fileName)
{
  int size;
  const char* data = UserInterfaceData::get(fileName, size);
  return data ? ImageCache::getFromMemory(data, size) : NULL;
}

Image* UserInterfaceManager::getImage(const String& fileName, int width, int height)
{
  juce::int64 hashCode = (fileName.hashCode64() * 101 + (juce::int64)width) * 101 + (juce::int64)height;
  Image* res = ImageCache::getFromHashCode(hashCode);
  if (!res)
  {
    res = getImage(fileName);
    if (!res)
      return NULL;
    res = res->createCopy(width, height);
    jassert(res);
    ImageCache::addImageToCache(res, hashCode);
  }
  return res;
}

juce::Component* UserInterfaceManager::createComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name) const
{
  size_t n = lbcpp::getNumLibraries();
  for (size_t i = 0; i < n; ++i)
  {
    juce::Component* res = lbcpp::getLibrary(i)->createUIComponentIfExists(context, object, name);
    if (res)
      return res;
  }
  return NULL;
}

#include "Component/VariableTreeView.h"
#include "Component/ContainerTableListBox.h"
#include "Component/ExecutionTraceTreeView.h"

juce::TreeView* UserInterfaceManager::createVariableTreeView(ExecutionContext& context, const Variable& variable, const String& name,
                                                              bool showTypes, bool showShortSummaries, bool showMissingVariables, bool makeRootNodeVisible) const
{
  return new VariableTreeView(variable, name, VariableTreeOptions(showTypes, showShortSummaries, showMissingVariables, makeRootNodeVisible));
}

juce::TableListBox* UserInterfaceManager::createContainerTableListBox(ExecutionContext& context, const ContainerPtr& container) const
{
  return new ContainerTableListBox(container);
}

juce::TreeView* UserInterfaceManager::createExecutionTraceInteractiveTreeView(ExecutionContext& context, ExecutionTracePtr trace, ExecutionContextPtr traceContext) const
{
  return new ExecutionTraceTreeView(trace, traceContext);
}
