/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Utilities/FileType.h"
#include "Components/VariableBrowser.h"
#include "ExplorerConfiguration.h"
#include "ExplorerProject.h"
using namespace lbcpp;
using juce::TooltipWindow;

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

typedef ReferenceCountedObjectPtr<ExplorerExecutionCallback> ExplorerExecutionCallbackPtr;

static ExplorerExecutionCallbackPtr explorerExecutionCallback;

void flushErrorAndWarningMessages(const String& title)
  {explorerExecutionCallback->flushMessages(title);}

class ExplorerContentTabs : public TabbedComponent
{
public:
  ExplorerContentTabs(DocumentWindow* mainWindow)
    : TabbedComponent(TabbedButtonBar::TabsAtTop), mainWindow(mainWindow) {}

  void addVariable(ExecutionContext& context, const Variable& variable, const String& name, Component* component = NULL)
  {
    if (!component)
      component = createComponentForVariable(context, variable, name, true);
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
/*
class ExplorerApplicationData
{
public:
  
private:
  ExplorerProjectPtr currentProject;
  ExecutionContextPtr guiContext;
  ExecutionContextPtr localContext;
};*/

class ExplorerMainWindow : public DocumentWindow, public MenuBarModel
{
public:
  ExplorerMainWindow(ExecutionContext& context) 
    : DocumentWindow("LBC++ Explorer", Colours::whitesmoke, allButtons), context(context)
  {
    tooltipWindow = new TooltipWindow(NULL, 15);
    //addAndMakeVisible();
    setContentComponent(contentTabs = new ExplorerContentTabs(this));
    setMenuBar(this);
    setResizable(true, true);
    centreWithSize(1024, 768);
#ifdef JUCE_MAC
    setUsingNativeTitleBar(true);
#endif // JUCE_MAC

    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();
    if (configuration->getRecentProjects()->getNumRecentFiles())
      menuItemSelected(openRecentProjectMenu, 0); // open most recent project
  }
  
  virtual ~ExplorerMainWindow()
  {
    //removeChildComponent(tooltipWindow);
    deleteAndZero(tooltipWindow);
    setMenuBar(NULL);
  }
    
  virtual void closeButtonPressed()
  {
    closeCurrentProject();
    JUCEApplication::quit();
  }

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

  enum
  {
    newProjectMenu = 1,
    openProjectMenu,
    closeProjectMenu,

    openFileMenu,
    openDirectoryMenu,
    startWorkUnitMenu,
    closeMenu,

    quitMenu,

    openClearRecentProjectMenu = 100,
    openRecentProjectMenu,
  };

  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();

    if (topLevelMenuIndex == 0)
    {
      PopupMenu menu;
      menu.addItem(newProjectMenu, T("New Project"));
      menu.addItem(openProjectMenu, T("Open Project"));
      
      RecentFileVectorPtr recentProjects = configuration->getRecentProjects();
      if (recentProjects->getNumRecentFiles())
      {
        PopupMenu subMenu;
        for (size_t i = 0; i < recentProjects->getNumRecentFiles(); ++i)
          subMenu.addItem(openRecentProjectMenu + (int)i, recentProjects->getRecentFile(i).getFileName());
        subMenu.addSeparator();
        subMenu.addItem(openClearRecentProjectMenu, T("Clear Menu"));
        menu.addSubMenu(T("Open Recent Project"), subMenu);
      }
      menu.addItem(closeProjectMenu, T("Close Project"));
      menu.addSeparator();

      menu.addItem(openFileMenu, T("Open File"), ExplorerProject::hasCurrentProject());
      menu.addItem(openDirectoryMenu, T("Open Directory"), ExplorerProject::hasCurrentProject());
      menu.addItem(startWorkUnitMenu, T("Start Work Unit"), ExplorerProject::hasCurrentProject());
      menu.addItem(closeMenu, T("Close"), ExplorerProject::hasCurrentProject() && contentTabs->getCurrentTabIndex() >= 0);
      menu.addSeparator();
      menu.addItem(quitMenu, T("Quit"));
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

  void closeCurrentProject()
  {
    if (ExplorerProject::hasCurrentProject())
    {
      ExplorerProject::currentProject->close(context);
      ExplorerProject::currentProject = ExplorerProjectPtr();
    }
    contentTabs->clearTabs();
    setName(T("LBC++ Explorer"));
  }

  void setCurrentProject(ExplorerProjectPtr project)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();
    RecentFileVectorPtr recentProjects = configuration->getRecentProjects();

    closeCurrentProject();
    jassert(project);

    File directory = project->getRootDirectory();
    defaultExecutionContext().setProjectDirectory(directory);

    recentProjects->addRecentFile(directory);
    recentProjects->setRecentDirectory(directory.getParentDirectory());
    configuration->save(context);
    setName(T("LBC++ Explorer - ") + directory.getFileName());
    ExplorerProject::currentProject = project;
    loadObjectFromFile(directory);
  }

  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();
    RecentFileVectorPtr recentProjects = configuration->getRecentProjects();

    if (topLevelMenuIndex == 0)
    {
      switch (menuItemID)
      {
      case newProjectMenu:
        {
          File directory = configuration->getRecentProjects()->getRecentDirectory();
          if (!directory.exists())
            File::getSpecialLocation(File::userHomeDirectory);
          FileChooser chooser("Please select the root directory of your project", directory, "*.*");
          if (chooser.browseForFileToSave(true))
          {
            File directory = chooser.getResult();
            ExplorerProjectPtr project = ExplorerProject::createProject(context, directory);
            if (project)
              setCurrentProject(project);
          }
        }
        break;

      case openProjectMenu:
        {
          File directory = configuration->getRecentProjects()->getRecentDirectory();
          if (!directory.exists())
            File::getSpecialLocation(File::userHomeDirectory);
          FileChooser chooser("Please choose the project to open", directory, "*.*");
          if (chooser.browseForDirectory())
          {
            File directory = chooser.getResult();
            ExplorerProjectPtr project = ExplorerProject::openProject(context, directory);
            if (project)
              setCurrentProject(project);
          }
        }
        break;

      case closeProjectMenu:
        closeCurrentProject();
        break;

      case openFileMenu:
      case openDirectoryMenu:
        {
          File directory = ExplorerProject::currentProject->getRecentDirectory();
          if (!directory.exists())
            directory = File::getSpecialLocation(File::userHomeDirectory);

          FileChooser chooser("Please select the file you want to load...",
                                   directory,
                                   "*.*");

          if ((menuItemID == openFileMenu && chooser.browseForFileToOpen()) ||
              (menuItemID == openDirectoryMenu && chooser.browseForDirectory()))
          {
            File result = chooser.getResult();
            ExplorerProject::currentProject->setRecentDirectory(result.getParentDirectory());
            loadObjectFromFile(result);
          }
        }

        break;

      case startWorkUnitMenu:
        {
          WorkUnitPtr workUnit;
          if (ExplorerProject::currentProject->startWorkUnit(defaultExecutionContext(), workUnit))
          {
            ExecutionTracePtr trace(new ExecutionTrace(ExplorerProject::currentProject->workUnitContext->toString()));
            Component* component = userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, ExplorerProject::currentProject->workUnitContext);
            contentTabs->addVariable(context, trace, workUnit->getClassName(), new VariableBrowser(trace, component));
            ExplorerProject::currentProject->workUnitContext->pushWorkUnit(workUnit);
          }
          flushErrorAndWarningMessages(T("Start Work Unit"));
        }
        break;

      case closeMenu:
        contentTabs->closeCurrentTab();
        break;

      case quitMenu:
        JUCEApplication::quit();
        break;

      default:
        if (menuItemID >= openRecentProjectMenu)
        {
          size_t recentFileId = (size_t)(menuItemID - openRecentProjectMenu);
          jassert(recentFileId < recentProjects->getNumRecentFiles());
          ExplorerProjectPtr project = ExplorerProject::openProject(context, recentProjects->getRecentFile(recentFileId));
          if (project)
            setCurrentProject(project);
        }
        else if (menuItemID == openClearRecentProjectMenu)
          recentProjects->clearRecentFiles();
        break;
      };
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
    //RecentFileVector::getInstance(context)->addRecentFile(file);
    // FIXME: save project ?
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
  TooltipWindow* tooltipWindow;
};

namespace lbcpp
{
  extern LibraryPtr explorerLibrary();
};

class ExplorerJUCEApplication : public JUCEApplication
{
public:
  ExplorerJUCEApplication() : mainWindow(0) {/*_crtBreakAlloc = 729;*/}

  virtual void initialise(const String& commandLine)
  {    
    lbcpp::initialize(NULL);
    setDefaultExecutionContext(singleThreadedExecutionContext());
    defaultExecutionContext().appendCallback(consoleExecutionCallback());
    defaultExecutionContext().appendCallback(explorerExecutionCallback = new ExplorerExecutionCallback());

    File currentExecutableDirectory = File::getSpecialLocation(File::currentExecutableFile).getParentDirectory();
#ifdef JUCE_MAC
    currentExecutableDirectory = currentExecutableDirectory.getChildFile("../../..");
#endif // JUCE_MAC
    lbcpp::importLibrariesFromDirectory(currentExecutableDirectory);
    lbcpp::importLibrary(explorerLibrary());
    defaultExecutionContext().setProjectDirectory(currentExecutableDirectory);

    theCommandManager = new ApplicationCommandManager();

    mainWindow = new ExplorerMainWindow(defaultExecutionContext());
    mainWindow->setVisible(true);

    flushErrorAndWarningMessages(T("Explorer Start-up"));
  }
  
  virtual void shutdown()
  {
    delete mainWindow;
    mainWindow = 0;
    deleteAndZero(theCommandManager); 
    ExplorerConfiguration::getInstancePtr() = ExplorerConfigurationPtr();
    explorerExecutionCallback = ExecutionCallbackPtr();
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
  {return JUCEApplication::main(argc, argv, new ExplorerJUCEApplication());}
