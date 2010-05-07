/*-----------------------------------------.---------------------------------.
| Filename: NewProcessDialogWindow.h       | The dialog window to create a   |
| Author  : Francis Maes                   |  new process                    |
| Started : 07/05/2010 13:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_PROCESS_MANAGER_NEW_DIALOG_WINDOW_H_
# define LBCPP_EXPLORER_PROCESS_MANAGER_NEW_DIALOG_WINDOW_H_

# include "ProcessManager.h"
# include "RecentProcesses.h"

namespace lbcpp
{

class NewProcessDialogWindow : public AlertWindow
{
public:
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

  struct ExecutableSelector : public FileSelector
  {
    ExecutableSelector(File& executable) : FileSelector(executable, false)
    {
      RecentProcessesPtr recent = RecentProcesses::getInstance();
      size_t numRecents = recent->getNumRecentExecutables();
      for (size_t i = 0; i < numRecents; ++i)
        comboBox->addItem(recent->getRecentExecutable(i).getFullPathName(), i + 1);
      if (numRecents)
        startingDirectory = recent->getRecentExecutable(0).getParentDirectory();
#ifdef JUCE_WIN32
      wildCardPattern = T("*.exe");
#endif //JUCE_WIN32
      comboBox->setName(T("Executable"));
    }

    virtual void setFile(const File& file)
    {
      FileSelector::setFile(file);
      comboBox->addItem(file.getFullPathName(), comboBox->getNumItems() + 1);
      RecentProcesses::getInstance()->addRecentExecutable(file);
    }

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
      if (comboBoxThatHasChanged->getName() == T("Executable"))
      {
        clear();
        RecentProcessesPtr recent = RecentProcesses::getInstance();
        std::vector<String> arguments = recent->getRecentArguments(comboBoxThatHasChanged->getText());
        for (size_t i = 0; i < arguments.size(); ++i)
          addItem(arguments[i].isEmpty() ? T("<empty>") : arguments[i], i + 1);
        setSelectedItemIndex(0);
      }
    }

    String& arguments;

    juce_UseDebuggingNewOperator
  };

  struct WorkingDirectorySelector : public FileSelector
  {
    WorkingDirectorySelector(File& workingDirectory) : FileSelector(workingDirectory, true) {}

    virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
      if (comboBoxThatHasChanged->getName() == T("Executable"))
      {
        comboBox->clear();
        RecentProcessesPtr recent = RecentProcesses::getInstance();
        std::vector<File> arguments = recent->getRecentWorkingDirectories(comboBoxThatHasChanged->getText());
        for (size_t i = 0; i < arguments.size(); ++i)
          comboBox->addItem(arguments[i].getFullPathName(), i + 1);
        comboBox->setSelectedItemIndex(0);
      }
      else
        FileSelector::comboBoxChanged(comboBoxThatHasChanged);
    }

    juce_UseDebuggingNewOperator
  };

  ExecutableSelector executableSelector;
  ArgumentsSelector argumentsSelector;
  WorkingDirectorySelector workingDirectorySelector;

  NewProcessDialogWindow(File& executable, String& arguments, File& workingDirectory)
    : AlertWindow(T("New Process"), T("Select a program and its arguments"), QuestionIcon),
      executableSelector(executable), argumentsSelector(arguments), workingDirectorySelector(workingDirectory)
  {
    addTextBlock(T("Executable:"));
    addCustomComponent(&executableSelector);
    addTextBlock(T("Arguments:"));
    addCustomComponent(&argumentsSelector);
    addTextBlock(T("Working directory:"));
    addCustomComponent(&workingDirectorySelector);
    addButton(T("OK"), 1, juce::KeyPress::returnKey);
    addButton(T("Cancel"), 0, juce::KeyPress::escapeKey);

    executableSelector.addListener(&argumentsSelector);
    executableSelector.addListener(&workingDirectorySelector);
    if (executableSelector.getComboBox()->getNumItems())
      executableSelector.getComboBox()->setSelectedItemIndex(0);
  }

  static bool run(File& executable, String& arguments, File& workingDirectory)
  {
    NewProcessDialogWindow* window = new NewProcessDialogWindow(executable, arguments, workingDirectory);
    const int result = window->runModalLoop();
    delete window;
    return result == 1;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_PROCESS_MANAGER_NEW_DIALOG_WINDOW_H_
