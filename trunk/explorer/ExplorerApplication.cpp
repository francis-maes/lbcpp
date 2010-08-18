/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

#include "Utilities/SplittedLayout.h"
#include "ProcessManager/ProcessManager.h"
#include "ProcessManager/RecentProcesses.h"
#include "Components/FileObjectComponent.h" // for FileObject
#include "ExplorerConfiguration.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

class ExplorerErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const String& where, const String& what)
    {addMessage(T("Error in ") + where + T(": ") + what);}
  
  virtual void warningMessage(const String& where, const String& what)
    {addMessage(T("Warning in ") + where + T(": ") + what);}

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

static ExplorerErrorHandler explorerErrorHandler;

void flushErrorAndWarningMessages(const String& title)
  {explorerErrorHandler.flushMessages(title);}

class ExplorerContentTabs : public TabbedComponent
{
public:
  ExplorerContentTabs(DocumentWindow* mainWindow)
    : TabbedComponent(TabbedButtonBar::TabsAtTop), mainWindow(mainWindow) {}

  void addObject(ObjectPtr object, const String& name)
  {
    Component* component = createComponentForObject(object, name, true);
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
      menu.addItem(2, "Open Directory");
      
      ExplorerRecentFilesPtr configuration = ExplorerRecentFiles::getInstance();
      if (configuration->getNumRecentFiles())
      {
        PopupMenu openRecentFilesMenu;
        for (size_t i = 0; i < configuration->getNumRecentFiles(); ++i)
          openRecentFilesMenu.addItem(101 + i, configuration->getRecentFile(i).getFileName());
        openRecentFilesMenu.addSeparator();
        openRecentFilesMenu.addItem(100, T("Clear Menu"));
        menu.addSubMenu(T("Open Recent File"), openRecentFilesMenu);
      }

      menu.addItem(3, "Close", contentTabs->getCurrentTabIndex() >= 0);
      menu.addSeparator();
      menu.addItem(4, "Process Manager");
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
      ExplorerRecentFilesPtr configuration = ExplorerRecentFiles::getInstance();

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
        contentTabs->closeCurrentTab();
      else if (menuItemID == 4)
        contentTabs->addObject(localProcessManager(), T("Process Manager"));
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
    ExplorerRecentFiles::getInstance()->addRecentFile(file);
    ExplorerConfiguration::save();
    ObjectPtr object = loadObject(file);
    if (object)
      contentTabs->addObject(object, file.getFileNameWithoutExtension());
  }

  ObjectPtr loadObject(const File& file)
  {
    FileObjectPtr fileObject = new FileObject(file);
    ObjectPtr res;
    switch (fileObject->getType())
    {
    case FileObject::textFile:
      res = fileObject;
      break;

    case FileObject::xmlFile:
      res = Variable::createFromFile(file).getObject();
      break;

    case FileObject::classFile:
    case FileObject::classDirectory:
      res = Object::createFromFile(file);
      break;

    default:
      res = fileObject;
    }
    flushErrorAndWarningMessages(T("Load ") + file.getFileName());
    return res;
  }

  juce_UseDebuggingNewOperator

private:
  ExplorerContentTabs* contentTabs;
};

extern void declareLBCppCoreClasses();
extern void declareProteinLibrary();

class ExplorerApplication : public JUCEApplication
{
public:
  ExplorerApplication() : mainWindow(0) {/*_crtBreakAlloc = 3905;*/}

  virtual void initialise(const String& commandLine)
  {    
    ErrorHandler::setInstance(explorerErrorHandler);
    lbcpp::initialize();
    declareProteinLibrary();

    LBCPP_DECLARE_CLASS(FileObject, Object);

    LBCPP_DECLARE_CLASS_LEGACY(ExplorerConfiguration);

    LBCPP_DECLARE_CLASS_LEGACY(ExplorerRecentFiles);
    LBCPP_DECLARE_CLASS_LEGACY(RecentProcesses);
    LBCPP_DECLARE_CLASS_LEGACY(ProcessConsoleSettings);
    LBCPP_DECLARE_CLASS_LEGACY(ProcessConsoleFilter);

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
  {return JUCEApplication::main (argc, argv, new ExplorerApplication());}
