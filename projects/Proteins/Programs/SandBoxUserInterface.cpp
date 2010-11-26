/*-----------------------------------------.---------------------------------.
| Filename: SandBoxUserInterface.cpp       | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 15:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

using juce::Component;
using juce::DocumentWindow;

using juce::Colours;
using juce::Justification;

class UserInterfaceExecutionCallbackMainComponent : public Component
{
public:
  UserInterfaceExecutionCallbackMainComponent(CompositeExecutionCallback* parentCallback) : progressionValue(0.0)
  {
    parentCallback->appendCallback(notifierExecutionCallback(parentCallback->getContext(),
        userInterfaceManager().getNotificationQueue(), callback = new Callback(this)));
  }

  virtual ~UserInterfaceExecutionCallbackMainComponent()
  {
    callback->owner = NULL;
  }

  virtual void paint(juce::Graphics& g)
  {
    g.setColour(Colours::red);
    g.fillRect(0, 0, (int)(getWidth() * progressionValue), getHeight());
    g.setColour(Colours::black);
    g.drawText(progressionString, 0, 0, getWidth(), getHeight(), Justification::centred, false);
  }

  lbcpp_UseDebuggingNewOperator

private:
  String progressionString;
  double progressionValue;

  struct Callback : public ExecutionCallback
  {
    Callback(UserInterfaceExecutionCallbackMainComponent* owner)
      : owner(owner) {}

    UserInterfaceExecutionCallbackMainComponent* owner;
      
    virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit)
    {
      if (!owner)
        return;
      String str = String(progression);
      if (progressionTotal != 0.0)
        str += T(" / ") + String(progressionTotal);
      if (progressionUnit.isNotEmpty())
        str += T(" ") + progressionUnit;
      owner->progressionString = str;
      owner->progressionValue = progressionTotal ? progression / progressionTotal : -1.0;
      owner->repaint();
    }
  };

  typedef ReferenceCountedObjectPtr<Callback> CallbackPtr;
  CallbackPtr callback;
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
    userInterfaceManager().ensureIsInitialized(context);
    userInterfaceManager().callFunctionOnMessageThread(createWindowFunction, this);
  }

  void shutdown()
    {if (mainWindow) userInterfaceManager().callFunctionOnMessageThread(destroyWindowFunction, this);}

private:
  LBCppMainWindow* mainWindow;
  UserInterfaceExecutionCallbackMainComponent* content;

  static void* createWindowFunction(void* userData)
  {
    UserInterfaceExecutionCallback* pthis = (UserInterfaceExecutionCallback* )userData;
    jassert(!pthis->content && !pthis->mainWindow);
    pthis->content = new UserInterfaceExecutionCallbackMainComponent(pthis);
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
      Thread::sleep(100);
      context.progressCallback(i + 1.0, 10.0, T("epochs"));
    }

    context.informationCallback(T("Finished."));
    return true;
  }
};

ExecutionContextPtr createExecutionContext()
{
  ExecutionContextPtr res = defaultConsoleExecutionContext();
  res->appendCallback(new UserInterfaceExecutionCallback());
  return res;
}

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = createExecutionContext();
  context->declareType(new DefaultClass(T("MyWorkUnit"), T("WorkUnit")));
  int res = WorkUnit::main(*context, new MyWorkUnit(), argc, argv);
  context->clearCallbacks();
  lbcpp::deinitialize();
  return res;
}
