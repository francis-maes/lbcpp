/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

#include "Utilities/SplittedLayout.h"
#include "Utilities/FileType.h"
#include "WorkUnitManager/NewWorkUnitDialogWindow.h"
#include "ExplorerConfiguration.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

class ExplorerExecutionCallback : public ExecutionCallback
{
public:
  virtual void errorCallback(const String& where, const String& what)
    {addMessage(T("Error in ") + where + T(": ") + what);}
  
  virtual void warningCallback(const String& where, const String& what)
    {addMessage(T("Warning in ") + where + T(": ") + what);}

  virtual void informationCallback(const String& where, const String& what)
    {}

  void flushMessages(const String& title)
  {
    if (text.isNotEmpty())
    {
      AlertWindow::showMessageBox(AlertWindow::WarningIcon, title, text);
      text = String::empty;
    }
  }

  juce_UseDebuggingNewOperator

private:
  String text;

  void addMessage(const String& str)
  {
    if (text.isNotEmpty())
      text += T("\n");
    text += str;
  }
};

static ExplorerExecutionCallback explorerExecutionCallback;

void flushErrorAndWarningMessages(const String& title)
  {explorerExecutionCallback.flushMessages(title);}

class ExplorerContentTabs : public TabbedComponent
{
public:
  ExplorerContentTabs(DocumentWindow* mainWindow)
    : TabbedComponent(TabbedButtonBar::TabsAtTop), mainWindow(mainWindow) {}

  void addVariable(ExecutionContext& context, const Variable& variable, const String& name)
  {
    Component* component = createComponentForVariable(context, variable, name, true);
    addTab(name, Colours::lightblue, component, true);
    variables.push_back(variable);
    setCurrentTabIndex(getNumTabs() - 1);
  }

  void closeCurrentTab()
  {
    int current = getCurrentTabIndex();
    variables.erase(variables.begin() + current);
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

  Variable getCurrentVariable() const
  {
    int current = getCurrentTabIndex();
    return current >= 0 ? variables[current] : Variable();
  }

private:
  DocumentWindow* mainWindow;
  std::vector<Variable> variables;
};

class ExplorerMainWindow : public DocumentWindow, public MenuBarModel
{
public:
  ExplorerMainWindow(ExecutionContext& context) 
    : DocumentWindow("LBC++ Explorer", Colours::whitesmoke, allButtons), context(context)
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
      menu.addItem(2, "Open Directory");
      
      ExplorerRecentFilesPtr configuration = ExplorerRecentFiles::getInstance(*silentExecutionContext);
      if (configuration->getNumRecentFiles())
      {
        PopupMenu openRecentFilesMenu;
        for (size_t i = 0; i < configuration->getNumRecentFiles(); ++i)
          openRecentFilesMenu.addItem(101 + (int)i, configuration->getRecentFile(i).getFileName());
        openRecentFilesMenu.addSeparator();
        openRecentFilesMenu.addItem(100, T("Clear Menu"));
        menu.addSubMenu(T("Open Recent File"), openRecentFilesMenu);
      }
      menu.addItem(3, "Start Work Unit");
      menu.addItem(4, "Close", contentTabs->getCurrentTabIndex() >= 0);
      menu.addSeparator();
      menu.addItem(5, "Quit");
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
      ExplorerRecentFilesPtr configuration = ExplorerRecentFiles::getInstance(*silentExecutionContext);

      if (menuItemID == 1 || menuItemID == 2)
      {
        File directory = configuration->getRecentDirectory();
        if (!directory.exists())
          directory = File::getSpecialLocation(File::userHomeDirectory);

        FileChooser chooser("Please select the file you want to load...",
                                 directory,
                                 "*.*");

        if ((menuItemID == 1 && chooser.browseForFileToOpen()) ||
            (menuItemID == 2 && chooser.browseForDirectory()))
        {
          File result = chooser.getResult();
          configuration->setRecentDirectory(result.getParentDirectory());
          loadObjectFromFile(result);
        }
      }
      else if (menuItemID >= 101)
      {
        size_t recentFileId = menuItemID - 101;
        jassert(recentFileId < configuration->getNumRecentFiles());
        loadObjectFromFile(configuration->getRecentFile(recentFileId));
      }
      else if (menuItemID == 100)
        configuration->clearRecentFiles();
      else if (menuItemID == 3)
      {
        String workUnitName;
        String arguments;
        File workingDirectory;
        if (NewWorkUnitDialogWindow::run(workUnitName, arguments, workingDirectory))
        {
          RecentWorkUnitsConfiguration::getInstance()->addRecent(workUnitName, arguments, workingDirectory);
          ExecutionContextPtr workUnitContext = defaultExecutionContext();

          TypePtr workUnitType = context.getType(workUnitName);
          if (workUnitType)
          {
            WorkUnitPtr workUnit = context.createObject(workUnitType);
            if (!workUnit)
              context.errorCallback(T("Create Work Unit"), T("Could not create ") + workUnitName);
            else if (workUnit->parseArguments(context, arguments))
            {
              ExecutionTracePtr trace(new ExecutionTrace(*workUnitContext));
              contentTabs->addVariable(context, trace, workUnitName);
              workUnitContext->pushWorkUnit(workUnit);
            }
          }
        }
        flushErrorAndWarningMessages(T("Start Work Unit"));
      }
      else if (menuItemID == 4)
        contentTabs->closeCurrentTab();
      else if (menuItemID == 5)
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

  void loadObjectFromFile(const File& file)
  {
    ExplorerRecentFiles::getInstance(context)->addRecentFile(file);
    ExplorerConfiguration::save(context);
    Variable variable = createVariableFromFile(file);
    if (variable.exists())
      contentTabs->addVariable(context, variable, file.getFileNameWithoutExtension());
  }

  Variable createVariableFromFile(const File& file)
  {
    Variable res = getFileType(file) == lbcppXmlFile ? Variable::createFromFile(context, file) : Variable(file);
    flushErrorAndWarningMessages(T("Load ") + file.getFileName());
    return res;
  }

  juce_UseDebuggingNewOperator

private:
  ExecutionContext& context;
  ExplorerContentTabs* contentTabs;
};

extern void declareProteinClasses(ExecutionContext& context);
extern void declareExplorerClasses(ExecutionContext& context);

class ExplorerApplication : public JUCEApplication
{
public:
  ExplorerApplication() : mainWindow(0) {/*_crtBreakAlloc = 729;*/}

  virtual void initialise(const String& commandLine)
  {    
    lbcpp::initialize(NULL);
    context = defaultConsoleExecutionContext(); // FIXME
    context->appendCallback(&explorerExecutionCallback);
    declareProteinClasses(*context);
    declareExplorerClasses(*context);

    theCommandManager = new ApplicationCommandManager();

    mainWindow = new ExplorerMainWindow(*context);
    mainWindow->setVisible(true);

    flushErrorAndWarningMessages(T("Explorer Start-up"));
  }
  
  virtual void shutdown()
  {
    delete mainWindow;
    mainWindow = 0;
    deleteAndZero(theCommandManager); 
    ExplorerConfiguration::getInstancePtr() = VariableVectorPtr();
    lbcpp::deinitialize();
  }

  virtual void systemRequestedQuit()
  {
    if (mainWindow)
    {
      deleteAndZero(mainWindow);
      ExplorerConfiguration::save(*context);
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
  ExecutionContextPtr context;
};

// START_JUCE_APPLICATION(ExplorerApplication)
// pb de link ...
int main(int argc, char* argv[])
  {return JUCEApplication::main (argc, argv, new ExplorerApplication());}
