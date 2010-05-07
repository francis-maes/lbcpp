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
#include "ProcessManager/ProcessManager.h"
#include "ProcessManager/RecentProcesses.h"
#include "ExplorerConfiguration.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

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

    LBCPP_DECLARE_CLASS(ExplorerConfiguration);
    LBCPP_DECLARE_CLASS(RecentProcesses);

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
      ExplorerConfiguration::save();
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
