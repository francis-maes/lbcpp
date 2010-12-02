/*-----------------------------------------.---------------------------------.
| Filename: NewWorkUnitDialogWindow.h      | The dialog window to start a    |
| Author  : Francis Maes                   |  new work unit                  |
| Started : 02/12/2010 03:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
# define LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_

# include "WorkUnitManagerConfiguration.h"
# include "../Components/common.h"

namespace lbcpp
{

class NewWorkUnitDialogWindow : public juce::AlertWindow
{
public:
  struct WorkUnitSelector : public juce::ComboBox, public juce::ComboBoxListener
  {
    WorkUnitSelector(String& workUnitName) : juce::ComboBox(T("Toto")), workUnitName(workUnitName)
    {
      setEditableText(true);
      setSize(600, 22);
      addListener(this);

      RecentWorkUnitsConfigurationPtr recent = RecentWorkUnitsConfiguration::getInstance();
      size_t numRecents = recent->getNumRecentWorkUnits();
      for (size_t i = 0; i < numRecents; ++i)
      {
        String str = recent->getRecentWorkUnit(i)->getWorkUnitName();
        if (str.isNotEmpty())
          addItem(str, (int)i + 1);
      }
      setName(T("WorkUnit"));
    }

    virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
    {
      RecentWorkUnitsConfigurationPtr recent = RecentWorkUnitsConfiguration::getInstance();
      if (comboBoxThatHasChanged == this)
      {
        workUnitName = getText();
        recent->addRecentWorkUnit(workUnitName);
      }
    }

    String& workUnitName;

    juce_UseDebuggingNewOperator
  };

  struct ArgumentsSelector : public juce::ComboBox, public juce::ComboBoxListener
  {
    ArgumentsSelector(String& arguments)
      : juce::ComboBox(T("Toto")), arguments(arguments)
    {
      setEditableText(true);
      setSize(600, 22);
      addListener(this);
    }

    virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
    {
      if (comboBoxThatHasChanged == this)
        arguments = getText();
      if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
      {
        clear();
        RecentWorkUnitsConfigurationPtr recent = RecentWorkUnitsConfiguration::getInstance();
        RecentWorkUnitConfigurationPtr workUnit = recent->getWorkUnit(comboBoxThatHasChanged->getText());
        if (workUnit)
        {
          std::vector<String> arguments = workUnit->getArguments();
          for (size_t i = 0; i < arguments.size(); ++i)
            addItem(arguments[i].isEmpty() ? T(" ") : arguments[i], (int)i + 1);
          if (arguments.size())
            setSelectedItemIndex(0);
        }
      }
    }

    String& arguments;

    juce_UseDebuggingNewOperator
  };

  struct FileSelector : public Component, public juce::ButtonListener, public juce::ComboBoxListener
  {
    FileSelector(File& file, bool selectDirectories)
      : file(file), selectDirectories(selectDirectories)
    {
      startingDirectory = File::getSpecialLocation(File::userHomeDirectory);
      wildCardPattern = T("*");
      addAndMakeVisible(comboBox = new juce::ComboBox(T("TOTO")));
      comboBox->setEditableText(false);
      comboBox->addListener(this);
      addAndMakeVisible(browseButton = new juce::TextButton(T("Browse")));
      browseButton->addButtonListener(this);
      setSize(600, 22);
    }

    virtual ~FileSelector()
      {deleteAllChildren();}

    virtual void resized()
    {
      int buttonX = getWidth() - 2 * getHeight();
      comboBox->setBounds(0, 0, buttonX - 5, getHeight());
      browseButton->setBounds(buttonX, 0, getWidth() - buttonX, getHeight());
    }

    virtual void buttonClicked(juce::Button* button)
    {
      if (selectDirectories)
      {
        FileChooser chooser("Please select a directory...", startingDirectory);
        if (chooser.browseForDirectory())
          setFile(chooser.getResult());
      }
      else
      {
        FileChooser chooser("Please select a file...", startingDirectory, "*.exe");
        if (chooser.browseForFileToOpen())
          setFile(chooser.getResult());
      }
    }

    virtual void setFile(const File& file)
    {
      this->file = file;
      comboBox->setText(file.getFullPathName());
    }

    void addListener(juce::ComboBoxListener* listener)
      {comboBox->addListener(listener);}

    virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
      if (comboBoxThatHasChanged == comboBox)
        file = File(comboBox->getText());
    }

    juce::ComboBox* getComboBox() const
      {return comboBox;}

    juce_UseDebuggingNewOperator

  protected:
    File& file;
    bool selectDirectories;

    juce::ComboBox* comboBox;
    juce::Button* browseButton;

    File startingDirectory;
    String wildCardPattern;
  };

  struct WorkingDirectorySelector : public FileSelector
  {
    WorkingDirectorySelector(File& workingDirectory) : FileSelector(workingDirectory, true) {}

    virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
      if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
      {
        comboBox->clear();
        RecentWorkUnitsConfigurationPtr recent = RecentWorkUnitsConfiguration::getInstance();
        RecentWorkUnitConfigurationPtr workUnit = recent->getWorkUnit(comboBoxThatHasChanged->getText());
        if (workUnit)
        {
          std::vector<File> arguments = workUnit->getWorkingDirectories();
          for (size_t i = 0; i < arguments.size(); ++i)
            comboBox->addItem(arguments[i].getFullPathName(), (int)i + 1);
          if (arguments.size())
            comboBox->setSelectedItemIndex(0);
        }
      }
      else
        FileSelector::comboBoxChanged(comboBoxThatHasChanged);
    }

    juce_UseDebuggingNewOperator
  };

  WorkUnitSelector workUnitSelector;
  ArgumentsSelector argumentsSelector;
  WorkingDirectorySelector workingDirectorySelector;

  NewWorkUnitDialogWindow(String& workUnitName, String& arguments, File& workingDirectory)
    : AlertWindow(T("New Work Unit"), T("Select a work unit and its arguments"), QuestionIcon),
      workUnitSelector(workUnitName), argumentsSelector(arguments), workingDirectorySelector(workingDirectory)
  {
    addTextBlock(T("Work Unit:"));
    addCustomComponent(&workUnitSelector);
    addTextBlock(T("Arguments:"));
    addCustomComponent(&argumentsSelector);
    addTextBlock(T("Working directory:"));
    addCustomComponent(&workingDirectorySelector);
    addButton(T("OK"), 1, juce::KeyPress::returnKey);
    addButton(T("Cancel"), 0, juce::KeyPress::escapeKey);

    workUnitSelector.addListener(&argumentsSelector);
    workUnitSelector.addListener(&workingDirectorySelector);
    if (workUnitSelector.getNumItems())
      workUnitSelector.setSelectedItemIndex(0);
  }

  static bool run(String& workUnitName, String& arguments, File& workingDirectory)
  {
    NewWorkUnitDialogWindow* window = new NewWorkUnitDialogWindow(workUnitName, arguments, workingDirectory);
    const int result = window->runModalLoop();
    delete window;
    return result == 1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
