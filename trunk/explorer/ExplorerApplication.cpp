/*-----------------------------.---------------------------------------------.
| Filename: application.cpp    | Explorer Application Main file              |
| Author  : Francis Maes       |                                             |
| Started : 10/11/2006 15:10   |                                             |
`------------------------------|                                             |
                               |                                             |
                               `--------------------------------------------*/

#include "Components/ObjectGraphAndContentComponent.h"
using namespace lbcpp;

ApplicationCommandManager* theCommandManager = NULL;

class ExplorerMainWindow : public DocumentWindow
{
public:
  ExplorerMainWindow() : DocumentWindow("LBC++ Explorer", Colours::white, allButtons)
  {
    //setMenuBar(this);
    setResizable(true, true);
    centreWithSize(700, 600);
#ifdef JUCE_MAC
//    setUsingNativeTitleBar(true);
#endif // JUCE_MAC

    FeatureDictionaryPtr dictionary = loadFeatureDictionary("/Users/francis/Projets/LBC++/trunk/examples/LearningMachine/features.dic");
    assert(dictionary);
    dictionary->addScope("coucou", new FeatureDictionary());
    dictionary->addScope("pouet", new FeatureDictionary());
   // setContentComponent(new ObjectGraphAndContent(dictionary->toGraph()));      
    setContentComponent(new ObjectGraphComponent(dictionary->toGraph()));
  }
    
  virtual void closeButtonPressed()
  {
    JUCEApplication::quit();
  }
  
private:
  Component* content;
};

class ExplorerApplication : public JUCEApplication
{
public:
  ExplorerApplication() : mainWindow(0) {/*_crtBreakAlloc = 3905;*/}

  virtual void initialise(const String& commandLine)
  {    
    theCommandManager = new ApplicationCommandManager();
    
    mainWindow = new ExplorerMainWindow();
    mainWindow->setVisible(true);

//    ImageCache::setCacheTimeout(30 * 1000);
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
//      resources::icons.clear();
//      StoredSettings::deleteInstance();
//      nieme::uninitialize();
      quit();
    }
  }

  virtual const String getApplicationName()
    {return T("LBC++ Explorer");}

  const String getApplicationVersion()
    {return T("1.0");}

  virtual bool moreThanOneInstanceAllowed()
  {
#ifndef JUCE_LINUX
    return false;
#else
    return true; //xxx should be false but doesn't work on linux..
#endif
  }

  virtual void anotherInstanceStarted(const String& commandLine)
  {
//    if (mainWindow != 0)
//      mainWindow->openFile(File::getCurrentWorkingDirectory().getChildFile(commandLine.unquoted()));
  }

private:
  ExplorerMainWindow* mainWindow;
};

// START_JUCE_APPLICATION(ExplorerApplication)
// pb de link ...

int main (int argc, char* argv[])
{
  return JUCEApplication::main (argc, argv, new ExplorerApplication());
}
