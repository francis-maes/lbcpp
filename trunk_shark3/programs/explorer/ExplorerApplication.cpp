/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Components/ObjectBrowser.h"
#include "ExplorerConfiguration.h"
#include "ExplorerProject.h"
#include <oil/Execution/ExecutionTrace.h>
#include <oil/UserInterface/UserInterfaceManager.h>
#include <oil/library.h>
using namespace lbcpp;
using juce::TooltipWindow;

ApplicationCommandManager* theCommandManager = NULL;

class ExplorerExecutionCallback : public ExecutionCallback
{
public:
  ExplorerExecutionCallback() : numErrors(0), numWarnings(0) {thisClass = executionCallbackClass;}

  virtual void errorCallback(const string& where, const string& what)
  {
    ++numErrors;
    lastError = JUCE_T("Error in ") + where + JUCE_T(": ") + what;
  }
  
  virtual void warningCallback(const string& where, const string& what)
  {
    ++numWarnings;
    lastWarning = JUCE_T("Warning in ") + where + JUCE_T(": ") + what;
  }

  virtual void informationCallback(const string& where, const string& what)
    {}

  void flushMessages(const string& title)
  {
    if (numErrors || numWarnings)
    {
      string text = string(numErrors) + JUCE_T(" error(s), ") + string(numWarnings) + JUCE_T(" warning(s)\n");
      if (lastError.isNotEmpty())
        text += JUCE_T("Last Error: ") + lastError + JUCE_T("\n");
      if (lastWarning.isNotEmpty())
        text += JUCE_T("Last Warning: ") + lastWarning + JUCE_T("\n");

      numErrors = numWarnings = 0;
      lastError = lastWarning = string::empty;
      AlertWindow::showMessageBox(AlertWindow::WarningIcon, title, text);
    }
  }

  juce_UseDebuggingNewOperator

private:
  string text;
  int numErrors, numWarnings;
  string lastError, lastWarning;

  void addMessage(const string& str)
  {
    if (text.isNotEmpty())
      text += JUCE_T("\n");
    text += str;
  }
};

typedef ReferenceCountedObjectPtr<ExplorerExecutionCallback> ExplorerExecutionCallbackPtr;

static ExplorerExecutionCallbackPtr explorerExecutionCallback;

void flushErrorAndWarningMessages(const string& title)
  {explorerExecutionCallback->flushMessages(title);}

class ExplorerContentTabs : public TabbedComponent
{
public:
  ExplorerContentTabs(DocumentWindow* mainWindow)
    : TabbedComponent(TabbedButtonBar::TabsAtTop), mainWindow(mainWindow) {}

  void addObject(ExecutionContext& context, const ObjectPtr& object, const string& name, Component* component = NULL)
  {
    if (!component)
      component = createComponentForObject(context, object, name, true);
    addTab(name, Colours::lightblue, component, true);
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

  virtual void currentTabChanged(const int newCurrentTabIndex, const string& newCurrentTabName)
  {
    string windowName = JUCE_T("LBC++ Explorer");
    if (newCurrentTabIndex >= 0)
      windowName += JUCE_T(" - ") + getCurrentTabName();
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
    res.add(JUCE_T("File"));
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

  virtual const PopupMenu getMenuForIndex(int topLevelMenuIndex, const string& menuName)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();

    if (topLevelMenuIndex == 0)
    {
      PopupMenu menu;
      menu.addItem(newProjectMenu, JUCE_T("New Project"));
      menu.addItem(openProjectMenu, JUCE_T("Open Project"));
      
      RecentFileVectorPtr recentProjects = configuration->getRecentProjects();
      if (recentProjects->getNumRecentFiles())
      {
        PopupMenu subMenu;
        for (size_t i = 0; i < recentProjects->getNumRecentFiles(); ++i)
          subMenu.addItem(openRecentProjectMenu + (int)i, recentProjects->getRecentFile(i).getFileName());
        subMenu.addSeparator();
        subMenu.addItem(openClearRecentProjectMenu, JUCE_T("Clear Menu"));
        menu.addSubMenu(JUCE_T("Open Recent Project"), subMenu);
      }
      menu.addItem(closeProjectMenu, JUCE_T("Close Project"));
      menu.addSeparator();

      bool hasCurrentProject = ExplorerProject::hasCurrentProject();

      menu.addItem(openFileMenu, JUCE_T("Open File"), hasCurrentProject);
      menu.addItem(openDirectoryMenu, JUCE_T("Open Directory"), hasCurrentProject);
      menu.addItem(startWorkUnitMenu, JUCE_T("Start Work Unit"), hasCurrentProject);
      menu.addItem(closeMenu, JUCE_T("Close"), hasCurrentProject && contentTabs->getCurrentTabIndex() >= 0);
      menu.addSeparator();
    
      menu.addItem(quitMenu, JUCE_T("Quit"));
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
    setName(JUCE_T("LBC++ Explorer"));
  }

  void setCurrentProject(ExplorerProjectPtr project)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();
    RecentFileVectorPtr recentProjects = configuration->getRecentProjects();

    closeCurrentProject();
    jassert(project);

    juce::File directory = project->getRootDirectory();
    defaultExecutionContext().setProjectDirectory(directory);
    jassert(project->workUnitContext->getProjectDirectory() == directory);

    recentProjects->addRecentFile(directory);
    recentProjects->setRecentDirectory(directory.getParentDirectory());
    configuration->save(context);
    setName(JUCE_T("LBC++ Explorer - ") + directory.getFileName());
    ExplorerProject::currentProject = project;
    loadObjectFromFile(directory);
  }

  virtual void menuItemSelected(int menuItemID, int topLevelMenuIndex)
  {
    ExplorerConfigurationPtr configuration = ExplorerConfiguration::getInstance();
    RecentFileVectorPtr recentProjects = configuration->getRecentProjects();
    ExplorerProjectPtr currentProject = ExplorerProject::getCurrentProject();

    if (topLevelMenuIndex == 0)
    {
      switch (menuItemID)
      {
      case newProjectMenu:
        {
          juce::File directory = configuration->getRecentProjects()->getRecentDirectory();
          if (!directory.exists())
            directory = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
          FileChooser chooser("Please select the root directory of your project", directory, "*.*");
          if (chooser.browseForFileToSave(true))
          {
            juce::File directory = chooser.getResult();
            ExplorerProjectPtr project = ExplorerProject::createProject(context, directory);
            if (project)
              setCurrentProject(project);
          }
        }
        break;

      case openProjectMenu:
        {
          juce::File directory = configuration->getRecentProjects()->getRecentDirectory();
          if (!directory.exists())
            directory = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
          FileChooser chooser("Please choose the project to open", directory, "*.*");
          if (chooser.browseForDirectory())
          {
            juce::File directory = chooser.getResult();
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
          juce::File directory = ExplorerProject::currentProject->getRecentDirectory();
          if (!directory.exists())
            directory = juce::File::getSpecialLocation(juce::File::userHomeDirectory);

          FileChooser chooser("Please select the file you want to load...",
                                   directory,
                                   "*.*");

          if ((menuItemID == openFileMenu && chooser.browseForFileToOpen()) ||
              (menuItemID == openDirectoryMenu && chooser.browseForDirectory()))
          {
            juce::File result = chooser.getResult();
            currentProject->setRecentDirectory(result.getParentDirectory());
            loadObjectFromFile(result);
          }
        }

        break;

      case startWorkUnitMenu:
        {
          WorkUnitPtr workUnit;
          if (currentProject->startWorkUnit(defaultExecutionContext(), workUnit))
          {
            ExecutionTracePtr trace(new ExecutionTrace(ExplorerProject::currentProject->workUnitContext->toString()));
            Component* component = userInterfaceManager().createExecutionTraceInteractiveTreeView(context, trace, currentProject->workUnitContext);
            contentTabs->addObject(context, trace, workUnit->getClassName(), new ObjectBrowser(trace, component));
            currentProject->workUnitContext->pushWorkUnit(workUnit);
          }
          flushErrorAndWarningMessages(JUCE_T("Start Work Unit"));
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

  void loadObjectFromFile(const juce::File& file)
  {
    //RecentFileVector::getInstance(context)->addRecentFile(file);
    // FIXME: save project ?
    ExplorerConfiguration::save(context);
    ObjectPtr object;
    if (file.isDirectory())
      object = File::create(file);
    else
    {
      object = Object::createFromFile(context, file);
      flushErrorAndWarningMessages(JUCE_T("Load ") + file.getFileName());
    }

    if (object)
      contentTabs->addObject(context, object, file.getFileNameWithoutExtension());
  }

  juce_UseDebuggingNewOperator

private:
  ExecutionContext& context;
  ExplorerContentTabs* contentTabs;
  TooltipWindow* tooltipWindow;
};

namespace lbcpp
{
  extern LibraryPtr lbCppMLLibrary();
  extern LibraryPtr explorerLibrary();
};

class ExplorerJUCEApplication : public JUCEApplication
{
public:
  ExplorerJUCEApplication() : mainWindow(0) {/*_crtBreakAlloc = 729;*/}

  virtual void initialise(const string& commandLine)
  {    
    lbcpp::initialize(NULL);
    setDefaultExecutionContext(singleThreadedExecutionContext());
    defaultExecutionContext().appendCallback(consoleExecutionCallback());
    defaultExecutionContext().appendCallback(explorerExecutionCallback = new ExplorerExecutionCallback());

    juce::File currentExecutableDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
#ifdef JUCE_MAC
    currentExecutableDirectory = currentExecutableDirectory.getChildFile("../../..");
#endif // JUCE_MAC
    lbcpp::importLibrary(lbCppMLLibrary());
    lbcpp::importLibrariesFromDirectory(currentExecutableDirectory);
    lbcpp::importLibrary(explorerLibrary());
    defaultExecutionContext().setProjectDirectory(currentExecutableDirectory);

    theCommandManager = new ApplicationCommandManager();

    mainWindow = new ExplorerMainWindow(defaultExecutionContext());
    mainWindow->setVisible(true);

    flushErrorAndWarningMessages(JUCE_T("Explorer Start-up"));
  }
  
  virtual void shutdown()
  {
    delete mainWindow;
    mainWindow = 0;
    deleteAndZero(theCommandManager); 
    ExplorerConfiguration::getInstancePtr() = ExplorerConfigurationPtr();
    ExplorerProject::currentProject = ExplorerProjectPtr();
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

  virtual const string getApplicationName()
    {return JUCE_T("LBC++ Explorer");}

  const string getApplicationVersion()
    {return JUCE_T("1.0");}

  virtual bool moreThanOneInstanceAllowed()
    {return true;}

  virtual void anotherInstanceStarted(const string& commandLine)
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
