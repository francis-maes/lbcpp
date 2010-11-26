/*-----------------------------------------.---------------------------------.
| Filename: SandBoxUserInterface.cpp       | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 15:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

namespace lbcpp
{

class UserInterfaceThread;

class UserInterfaceManager
{
public:
  UserInterfaceManager() : userInterfaceThread(NULL) {}
  ~UserInterfaceManager()
    {shutdown();}

  void ensureIsInitialized();
  bool isRunning() const;
  void shutdown();

  typedef void* (MessageCallbackFunction) (void* userData);

  void* callFunctionOnMessageThread(MessageCallbackFunction* callback, void* userData);

private:
  UserInterfaceThread* userInterfaceThread;
};

////////////////

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

void UserInterfaceManager::ensureIsInitialized()
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

}; /* namespace lbcpp */

//////////////////////////////////

UserInterfaceManager userInterfaceManager;

using juce::Component;
using juce::DocumentWindow;

using juce::Colours;
using juce::Justification;


class UserInterfaceExecutionCallbackMainComponent : public Component, public ExecutionCallback
{
public:
  UserInterfaceExecutionCallbackMainComponent() : progressionValue(0.0) {}

  virtual void paint(juce::Graphics& g)
  {
    String progressionString;
    double progressionValue = getProgression(progressionString);
    g.fillRect(0, 0, (int)(getWidth() * progressionValue), getHeight());
    g.setColour(Colours::black);
    g.drawText(progressionString, 0, 0, getWidth(), getHeight(), Justification::centred, false);
  }

  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
  {
    {
      ScopedLock _(progressionLock);
      progressionString = String(progression);
      if (progressionTotal != 0.0)
        progressionString += T(" / ") + String(progressionTotal);
      if (progressionUnit.isNotEmpty())
        progressionString += T(" ") + progressionUnit;
      progressionValue = progressionTotal ? progression / progressionTotal : -1.0;
    }
    juce::MessageManagerLock _;
    repaint();
  }

  lbcpp_UseDebuggingNewOperator

private:
  CriticalSection progressionLock;
  String progressionString;
  double progressionValue;

  double getProgression(String& string) const
  {
    ScopedLock _(progressionLock);
    string = progressionString;
    return progressionValue;
  }
};

class LBCppMainWindow : public DocumentWindow
{
public:
  LBCppMainWindow(Component* content) 
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
    userInterfaceManager.ensureIsInitialized();
    userInterfaceManager.callFunctionOnMessageThread(createWindowFunction, this);
  }

  void shutdown()
    {if (mainWindow) userInterfaceManager.callFunctionOnMessageThread(destroyWindowFunction, this);}

private:
  LBCppMainWindow* mainWindow;
  UserInterfaceExecutionCallbackMainComponent* content;

  static void* createWindowFunction(void* userData)
  {
    UserInterfaceExecutionCallback* pthis = (UserInterfaceExecutionCallback* )userData;
    jassert(!pthis->content && !pthis->mainWindow);
    pthis->content = new UserInterfaceExecutionCallbackMainComponent();
    pthis->content->setStaticAllocationFlag();
    pthis->appendCallback(ExecutionCallbackPtr(pthis->content));

    pthis->mainWindow = new LBCppMainWindow(pthis->content);
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

//////////////////////////////////////
//////////////////////////////////////

class MyWorkUnit : public WorkUnit
{
public:
  virtual String toString() const
    {return T("My Work Unit !");}
 
  virtual bool run(ExecutionContext& context)
  {
    context.statusCallback(T("Working..."));

    //context.errorCallback(T("My Error"));
    context.warningCallback(T("My Warning"));
    context.informationCallback(T("My Information"));

    for (size_t i = 0; i < 10; ++i)
    {
      Thread::sleep(10);
      context.progressCallback(i + 1.0, 10.0, T("epochs"));
    }

    context.informationCallback(T("Finished."));
    return true;
  }
};

UserInterfaceExecutionCallbackPtr userInterfaceCallback;

ExecutionContextPtr createExecutionContext()
{
  ExecutionContextPtr res = defaultConsoleExecutionContext();
  res->appendCallback(userInterfaceCallback = UserInterfaceExecutionCallbackPtr(new UserInterfaceExecutionCallback()));
  return res;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = createExecutionContext();
  context->declareType(new DefaultClass(T("MyWorkUnit"), T("WorkUnit")));
  int res = WorkUnit::main(*context, new MyWorkUnit(), argc, argv);
  userInterfaceCallback->shutdown();
  userInterfaceManager.shutdown();
  context = ExecutionContextPtr();
  juce::shutdownJuce_NonGUI();
  lbcpp::deinitialize();
  return res;
}
