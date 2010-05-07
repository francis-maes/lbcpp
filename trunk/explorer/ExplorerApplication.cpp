/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

//#include "Components/ObjectGraphAndContentComponent.h"
#include "Components/StringComponent.h"
#include "Components/TableComponent.h"
#include "Components/StringToObjectMapTabbedComponent.h"
#include "Utilities/SplittedLayout.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

class Process : public NameableObject
{
public:
  Process(const File& executableFile, const String& arguments, const String& name = String::empty)
    : NameableObject(name.isEmpty() ? executableFile.getFileNameWithoutExtension() + T(" ") + arguments : name),
      executableFile(executableFile), arguments(arguments) {}

  virtual String toString() const
    {return executableFile.getFullPathName() + T(" ") + arguments;}

private:
  File executableFile;
  String arguments;
};

typedef ReferenceCountedObjectPtr<Process> ProcessPtr;

class ProcessList : public VectorObjectContainer
{
public:
  size_t getNumProcesses() const
    {return size();}

  ProcessPtr getProcess(size_t index) const
    {return getAndCast<Process>(index);}
};

typedef ReferenceCountedObjectPtr<ProcessList> ProcessListPtr;

class ProcessManager : public Object
{
public:
  ProcessManager() : runningProcesses(new ProcessList()), 
        waitingProcesses(new ProcessList()), 
        finishedProcesses(new ProcessList()), 
        killedProcesses(new ProcessList())
  {
  }

  virtual String getName() const
    {return T("Process Manager");}

  virtual juce::Component* createComponent() const;

  ProcessListPtr getRunningProcesses() const
    {return runningProcesses;}

  ProcessListPtr getWaitingProcesses() const
    {return waitingProcesses;}

  ProcessListPtr getFinishedProcesses() const
    {return finishedProcesses;}

  ProcessListPtr getKilledProcesses() const
    {return killedProcesses;}

protected:
  ProcessListPtr runningProcesses;
  ProcessListPtr waitingProcesses;
  ProcessListPtr finishedProcesses;
  ProcessListPtr killedProcesses;
};

typedef ReferenceCountedObjectPtr<ProcessManager> ProcessManagerPtr;

class LocalProcessManager : public ProcessManager
{
public:

};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class ObjectContainerNameListComponent : public juce::ListBox
{
public:
  ObjectContainerNameListComponent(ObjectContainerPtr container)
    : juce::ListBox(container->getName(), new Model(container)) {}
  
  virtual ~ObjectContainerNameListComponent()
    {delete getModel();}

  struct Model : public juce::ListBoxModel
  {
    Model(ObjectContainerPtr container) : container(container) {}

    virtual int getNumRows()
      {return (int)container->size();}

    virtual void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
      if (rowIsSelected)
        g.fillAll(Colours::lightblue);

      g.setColour(Colours::black);
      Font f(height * 0.7f);
      f.setHorizontalScale(0.9f);
      g.setFont(f);

      ObjectPtr object = container->get(rowNumber);
      String name = object ? object->getName() : T("<null>");
      g.drawText(name, 4, 0, width - 6, height, Justification::centredLeft, true);
    }

    juce_UseDebuggingNewOperator

  private:
    ObjectContainerPtr container;
  };

  juce_UseDebuggingNewOperator
};

class ProcessConsoleComponent : public Component
{
public:
  ProcessConsoleComponent(ProcessPtr process) : process(process) {}

  juce_UseDebuggingNewOperator

private:
  ProcessPtr process;
};

class ProcessManagerListTabs : public TabbedComponent
{
public:
  ProcessManagerListTabs(ProcessManagerPtr processManager) : TabbedComponent(TabbedButtonBar::TabsAtBottom)
  {
    addProcessList(T("Running"), processManager->getRunningProcesses());
    addProcessList(T("Waiting"), processManager->getWaitingProcesses());
    addProcessList(T("Finished"), processManager->getFinishedProcesses());
    addProcessList(T("Killed"), processManager->getKilledProcesses());
  }

  juce_UseDebuggingNewOperator

private:
  void addProcessList(const String& name, ProcessListPtr processes)
    {addTab(name, Colours::lightblue, new ObjectContainerNameListComponent(processes), true);}
};

class RecentProcesses : public Object
{
public:

};


class ProcessManagerComponent : public SplittedLayout, public MenuBarModel
{
public:
  ProcessManagerComponent(ProcessManagerPtr processManager)
    : SplittedLayout(new ProcessManagerListTabs(processManager), new Viewport(), 0.33, SplittedLayout::typicalHorizontal), processManager(processManager)
    {}

  virtual const StringArray getMenuBarNames()
  {
    StringArray res;
    res.add(T("Process"));
    return res;
  }

  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName)
  {
    jassert(topLevelMenuIndex == 0);
    PopupMenu menu;
    menu.addItem(1, T("New Process"));
    menu.addItem(2, T("Kill all Running Processes"));
    menu.addItem(3, T("Clear Process Lists"));
    return menu;
  }

  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex)
  {
    jassert(topLevelMenuIndex == 0);
    
    switch (menuItemID)
    {
    case 1:
      break;

    case 2:
      break;

    case 3:
      break;
    };
  }

  juce_UseDebuggingNewOperator

protected:
  ProcessManagerPtr processManager;
};

juce::Component* ProcessManager::createComponent() const
  {return new ProcessManagerComponent(ProcessManagerPtr(const_cast<ProcessManager* >(this)));}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

Component* lbcpp::createComponentForObject(ObjectPtr object, bool topLevelComponent)
{
  if (!object)
    return NULL;
  Component* res = object->createComponent();
  if (res)
    return res;

  if (object.dynamicCast<StringToObjectMap>())
    return new StringToObjectMapTabbedComponent(object.dynamicCast<StringToObjectMap>());

  //ObjectGraphPtr graph = object->toGraph();
  //if (topLevelComponent && graph)
  //  return new ObjectGraphAndContentComponent(graph);
  TablePtr table = object->toTable();
  if (table)
    return new TableComponent(table);
  return new StringComponent(object);
}

class ExplorerErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const String& where, const String& what)
    {AlertWindow::showMessageBox(AlertWindow::WarningIcon, T("Error in '") + where + T("'"), what);}
  
  virtual void warningMessage(const String& where, const String& what)
    {AlertWindow::showMessageBox(AlertWindow::WarningIcon, T("Warning in '") + where + T("'"), what);}

  juce_UseDebuggingNewOperator
};

static ExplorerErrorHandler explorerErrorHandler;

class ExplorerContentTabs : public TabbedComponent
{
public:
  ExplorerContentTabs(DocumentWindow* mainWindow)
    : TabbedComponent(TabbedButtonBar::TabsAtTop), mainWindow(mainWindow) {}

  void addObject(ObjectPtr object)
  {
    Component* component = createComponentForObject(object, true);
    addTab(object->getName(), Colours::lightblue, component, true);
    objects.push_back(object);
    setCurrentTabIndex(getNumTabs() - 1);
  }

  void closeCurrentTab()
  {
    int current = getCurrentTabIndex();
    objects.erase(objects.begin() + current);
    if (current >= 0)
      removeTab(current);
  }

  virtual void currentTabChanged(const int newCurrentTabIndex, const String& newCurrentTabName)
  {
    String windowName = T("LBC++ Explorer");
    if (newCurrentTabIndex >= 0)
      windowName += T(" - ") + getCurrentTabName();
    mainWindow->setName(windowName);
    dynamic_cast<MenuBarModel* >(mainWindow)->menuItemsChanged();
  }

  ObjectPtr getCurrentObject() const
  {
    int current = getCurrentTabIndex();
    return current >= 0 ? objects[current] : ObjectPtr();
  }

private:
  DocumentWindow* mainWindow;
  std::vector<ObjectPtr> objects;
};

class ExplorerMainWindow : public DocumentWindow, public MenuBarModel
{
public:
  ExplorerMainWindow() : DocumentWindow("LBC++ Explorer", Colours::whitesmoke, allButtons)
  {
    setContentComponent(contentTabs = new ExplorerContentTabs(this));
    setMenuBar(this);
    setResizable(true, true);
    centreWithSize(700, 600);
#ifdef JUCE_MAC
    setUsingNativeTitleBar(true);
#endif // JUCE_MAC
  }
  
  virtual ~ExplorerMainWindow()
    {setMenuBar(NULL);}
    
  virtual void closeButtonPressed()
    {JUCEApplication::quit();}

  MenuBarModel* getAdditionalMenus() const
    {return dynamic_cast<MenuBarModel* >(contentTabs->getCurrentContentComponent());}
  
  virtual const StringArray getMenuBarNames()
  {
    StringArray res;
    res.add(T("File"));
    MenuBarModel* additionalMenus = getAdditionalMenus();
    if (additionalMenus)
      res.addArray(additionalMenus->getMenuBarNames());
    return res;
  }

  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName)
  {
    if (topLevelMenuIndex == 0)
    {
      PopupMenu menu;
      menu.addItem(1, "Open");
      menu.addItem(2, "Close", contentTabs->getCurrentTabIndex() >= 0);
      menu.addSeparator();
      menu.addItem(3, "Process Manager");
      menu.addSeparator();
      menu.addItem(4, "Quit");
      return menu;
    }
    else
    {
      jassert(topLevelMenuIndex > 0);
      MenuBarModel* additionalMenus = getAdditionalMenus();
      jassert(additionalMenus);
      return additionalMenus->getMenuForIndex(topLevelMenuIndex - 1, menuName);
    }
  }

  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex)
  {
    if (topLevelMenuIndex == 0)
    {
      if (menuItemID == 1)
      {
        FileChooser chooser("Please select the file you want to load...",
                                 File::getSpecialLocation (File::userHomeDirectory),
                                 "*.*");

        if (chooser.browseForFileToOpen())
        {
          ObjectPtr object = loadObject(chooser.getResult());
          if (object)
            contentTabs->addObject(object);
        }
      }
      else if (menuItemID == 2)
        contentTabs->closeCurrentTab();
      else if (menuItemID == 3)
        contentTabs->addObject(new LocalProcessManager());
      else if (menuItemID == 4)
        JUCEApplication::quit();
    }
    else
    {
      jassert(topLevelMenuIndex > 0);
      MenuBarModel* additionalMenus = getAdditionalMenus();
      jassert(additionalMenus);
      return additionalMenus->menuItemSelected(menuItemID, topLevelMenuIndex - 1);
    }
  }
  
  juce_UseDebuggingNewOperator

private:
  ExplorerContentTabs* contentTabs;
};

extern void declareLBCppCoreClasses();
extern void declareProteinClasses();

class ExplorerApplication : public JUCEApplication
{
public:
  ExplorerApplication() : mainWindow(0) {/*_crtBreakAlloc = 3905;*/}

  virtual void initialise(const String& commandLine)
  {    
    ErrorHandler::setInstance(explorerErrorHandler);
    declareLBCppCoreClasses();
    declareProteinClasses();

    theCommandManager = new ApplicationCommandManager();

    mainWindow = new ExplorerMainWindow();
    mainWindow->setVisible(true);
  }
  
  virtual void shutdown()
  {
    delete mainWindow;
    mainWindow = 0;
    deleteAndZero(theCommandManager); 
  }

  virtual void systemRequestedQuit()
  {
    if (mainWindow)
    {
      deleteAndZero(mainWindow);
      quit();
    }
  }

  virtual const String getApplicationName()
    {return T("LBC++ Explorer");}

  const String getApplicationVersion()
    {return T("1.0");}

  virtual bool moreThanOneInstanceAllowed()
    {return true;}

  virtual void anotherInstanceStarted(const String& commandLine)
    {}

  juce_UseDebuggingNewOperator

private:
  ExplorerMainWindow* mainWindow;
};

// START_JUCE_APPLICATION(ExplorerApplication)
// pb de link ...

int main(int argc, char* argv[])
{
  return JUCEApplication::main (argc, argv, new ExplorerApplication());
}
