/*-----------------------------------------.---------------------------------.
| Filename: NewWorkUnitDialogWindow.h      | The dialog window to start a    |
| Author  : Francis Maes                   |  new work unit                  |
| Started : 02/12/2010 03:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
# define LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_

# include "../ExplorerProject.h"
# include "../Components/common.h"

namespace lbcpp
{

class NewWorkUnitContentComponent;

class NewWorkUnitDialogWindow : public juce::DocumentWindow
{
public:
//  virtual ~NewWorkUnitDialogWindow();
/*
  struct FileSelector : public Component, public juce::ButtonListener, public juce::ComboBoxListener
  {
    FileSelector(RecentWorkUnitsConfigurationPtr recent, File& file, bool selectDirectories)
      : recent(recent), file(file), selectDirectories(selectDirectories)
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
    RecentWorkUnitsConfigurationPtr recent;
    File& file;
    bool selectDirectories;

    juce::ComboBox* comboBox;
    juce::Button* browseButton;

    File startingDirectory;
    String wildCardPattern;
  };

  struct WorkingDirectorySelector : public FileSelector
  {
    WorkingDirectorySelector(RecentWorkUnitsConfigurationPtr recent, File& workingDirectory) : FileSelector(recent, workingDirectory, true) {}

    virtual void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
      if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
      {
        comboBox->clear();
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
*/
  static bool run(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory);
  
  virtual void closeButtonPressed();

  //virtual void resized();

  juce_UseDebuggingNewOperator

private:
  NewWorkUnitDialogWindow(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory);
  
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXPLORER_WORK_UNIT_MANAGER_NEW_DIALOG_WINDOW_H_
