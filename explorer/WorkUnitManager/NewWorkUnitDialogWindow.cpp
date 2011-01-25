/*-----------------------------------------.---------------------------------.
| Filename: NewWorkUnitDialogWindow.cp     | The dialog window to start a    |
| Author  : Francis Maes                   |  new work unit                  |
| Started : 25/01/2011 18:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NewWorkUnitDialogWindow.h"
using namespace lbcpp;

namespace lbcpp {

using juce::ComboBox;
using juce::Label;
using juce::Font;
using juce::TextEditor;

////////////
class VariableEditorComponent : public Component, public juce::ChangeBroadcaster
{
public:
  VariableEditorComponent(TypePtr type)
    : type(type) {}

  virtual void setValue(const Variable& value) = 0;
  virtual Variable getValue() const = 0;

  static VariableEditorComponent* create(const Variable& value);

protected:
  TypePtr type;
};

class VariableTextEditor : public VariableEditorComponent, public juce::TextEditorListener
{
public:
  VariableTextEditor(const Variable& variable)
    : VariableEditorComponent(variable.getType())
  {
    addAndMakeVisible(editor = new TextEditor());
    editor->addListener(this);
    setValue(variable);
  }

  virtual ~VariableTextEditor()
    {deleteAllChildren();}

  virtual void setValue(const Variable& value)
  {
    if (value.isMissingValue())
      editor->setText(String::empty);
    else
      editor->setText(value.toString());
  }

  virtual Variable getValue() const
    {return Variable::createFromString(defaultExecutionContext(), type, editor->getText());}

  virtual void resized()
    {editor->setBoundsRelative(0, 0, 1, 1);}

  virtual void textEditorTextChanged(TextEditor& editor) {}
  virtual void textEditorEscapeKeyPressed(TextEditor& editor) {}
  virtual void textEditorFocusLost(TextEditor& editor) {}
  virtual void textEditorReturnKeyPressed(TextEditor& editor)
    {sendChangeMessage(&editor);}

protected:
  TextEditor* editor;
};

VariableEditorComponent* VariableEditorComponent::create(const Variable& value)
{
  return new VariableTextEditor(value);
}

////////////

class WorkUnitSelectorComboxBox : public ComboBox, public juce::ComboBoxListener
{
public:
  WorkUnitSelectorComboxBox(RecentWorkUnitsConfigurationPtr recent, String& workUnitName)
    : juce::ComboBox(T("Toto")), recent(recent), workUnitName(workUnitName)
  {
    //setEditableText(true);
    setSize(600, 22);
    setName(T("WorkUnit"));
    addListener(this);

    int id = 1;

    size_t numRecents = recent->getNumRecentWorkUnits();
    if (numRecents)
    {
      addSectionHeading(T("Recent"));
      if (numRecents > 8) numRecents = 8;
      for (size_t i = 0; i < numRecents; ++i)
      {
        String str = recent->getRecentWorkUnit(i)->getWorkUnitName();
        addItem(str, id++);
      }
    }

    size_t numLibraries = lbcpp::getNumLibraries();
    for (size_t i = 0; i < numLibraries; ++i)
    {
      LibraryPtr library = lbcpp::getLibrary(i);
      if (library->getName() == T("LBCpp"))
        continue;
      std::vector<TypePtr> workUnits = library->getTypesInheritingFrom(workUnitClass);
      if (workUnits.size())
      {
        addSectionHeading(library->getName());
        for (size_t j = 0; j < workUnits.size(); ++j)
          if (workUnits[j] != workUnitClass)
            addItem(workUnits[j]->getName(), id++);
      }
    }
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == this)
    {
      workUnitName = getText();
      recent->addRecentWorkUnit(workUnitName);
    }
  }

  RecentWorkUnitsConfigurationPtr recent;
  String& workUnitName;

  juce_UseDebuggingNewOperator
};

class WorkUnitArgumentComponent : public Component, public juce::ChangeListener
{
public:
  WorkUnitArgumentComponent(ClassPtr workUnitClass, size_t variableIndex)
    : name(NULL), shortName(NULL), description(NULL)
  {
    TypePtr typeValue = workUnitClass->getObjectVariableType(variableIndex);
    String nameValue = workUnitClass->getObjectVariableName(variableIndex);
    String shortNameValue = workUnitClass->getObjectVariableShortName(variableIndex);
    String descriptionValue = workUnitClass->getObjectVariableDescription(variableIndex);

    int desiredHeight = namesHeight;
    addAndMakeVisible(name = new Label(T("name"), nameValue));
    name->setFont(Font(16, Font::bold));
    if (shortNameValue != nameValue)
    {
      addAndMakeVisible(shortName = new Label(T("shortName"), T("-") + shortNameValue));
      shortName->setJustificationType(Justification::centredRight);
      shortName->setFont(Font(14, Font::bold));
    }
    
    if (descriptionValue.isNotEmpty())
    {
      addAndMakeVisible(description = new Label(T("desc"), descriptionValue));
      description->setFont(Font(12, Font::italic));
      description->setSize(600, descriptionHeightPerLine);
      desiredHeight += descriptionHeightPerLine; // todo: * numLines
    }

    addAndMakeVisible(value = VariableEditorComponent::create(Variable::missingValue(typeValue)));
    value->addChangeListener(this);
    desiredHeight += 2 + (value->getHeight() ? value->getHeight() : defaultValueHeight);

    setSize(600, desiredHeight);
  }

  virtual ~WorkUnitArgumentComponent()
    {deleteAllChildren();}

  enum
  {
    namesHeight = 18,
    descriptionHeightPerLine = 15,
    defaultValueHeight = 20, 
  };

  virtual void resized()
  {
    name->setBounds(0, 0, getWidth(), namesHeight);
    if (shortName)
      shortName->setBounds(0, 0, getWidth(), namesHeight);
    if (description)
      description->setBounds(0, namesHeight, getWidth(), description->getHeight());
    int y = namesHeight + (description ? description->getHeight() : 0) + 2;
    value->setBounds(0, y, getWidth(), value->getHeight() ? value->getHeight() : defaultValueHeight);
  }

  void setValue(const Variable& v)
    {value->setValue(v);}

  virtual void changeListenerCallback(void* objectThatHasChanged)
  {

  }

private:
  Label* name;
  Label* shortName;
  Label* description;
  VariableEditorComponent* value; 
};

class WorkUnitArgumentListComponent : public Component
{
public:
  WorkUnitArgumentListComponent(RecentWorkUnitConfigurationPtr workUnit, String& resultString)
    : workUnit(workUnit), resultString(resultString)
  {
    ClassPtr workUnitClass = getType(workUnit->getWorkUnitName());
    jassert(workUnitClass);
    arguments.resize(workUnitClass->getObjectNumVariables());
    int desiredHeight = 0;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      addAndMakeVisible(arguments[i] = new WorkUnitArgumentComponent(workUnitClass, i));
      desiredHeight += arguments[i]->getHeight() + 4;
    }
    setSize(600, desiredHeight);
    setValuesFromCommandLine(resultString);
  }

  virtual ~WorkUnitArgumentListComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    int y = 0;
    for (size_t i = 0; i < arguments.size(); ++i)
    {
      int h = arguments[i]->getHeight();
      arguments[i]->setBounds(0, y, getWidth(), h);
      y += h + 4;
    }
  }

  void setValuesFromCommandLine(const String& commandLine)
  {
    ExecutionContext& context = defaultExecutionContext();
    
    WorkUnitPtr workUnit = WorkUnit::create(getType(this->workUnit->getWorkUnitName()));
    if (!workUnit)
    {
      context.errorCallback(T("Could not create work unit of class ") + this->workUnit->getWorkUnitName());
      return;
    }
    workUnit->parseArguments(context, commandLine);
    jassert(workUnit->getNumVariables() == arguments.size());
    for (size_t i = 0; i < arguments.size(); ++i)
      arguments[i]->setValue(workUnit->getVariable(i));
  }

protected:
  RecentWorkUnitConfigurationPtr workUnit;
  std::vector<WorkUnitArgumentComponent* > arguments;
  String& resultString;
};

class WorkUnitArgumentsViewport : public Viewport
{
public:
  WorkUnitArgumentsViewport()
  {
    setSize(600, 400);
    setScrollBarsShown(true, false);
  }

  virtual void paint(Graphics& g)
    {g.fillAll(Colours::white);}

  juce_UseDebuggingNewOperator
};

class WorkUnitArgumentsComponent : public Component, public juce::ComboBoxListener
{
public:
  WorkUnitArgumentsComponent(RecentWorkUnitConfigurationPtr workUnit, String& resultString)
    : argumentList(NULL), workUnit(workUnit), resultString(resultString)
  {
    // argument list
    addAndMakeVisible(viewport = new WorkUnitArgumentsViewport());
    viewport->setViewedComponent(argumentList = new WorkUnitArgumentListComponent(workUnit, resultString));
    argumentList->setSize(viewport->getMaximumVisibleWidth(), argumentList->getHeight());
    
    // command line label
    addAndMakeVisible(commandLineLabel = new Label(T("cmd"), T("Command Line:")));

    // command line
    addAndMakeVisible(commandLine = new ComboBox(T("recentArgs")));
    commandLine->setEditableText(true);
    std::vector<String> recents = workUnit->getArguments();
    commandLine->clear();
    for (size_t i = 0; i < recents.size(); ++i)
      if (recents[i].trim().isNotEmpty())
        commandLine->addItem(recents[i], (int)i + 1);
    if (commandLine->getNumItems())
      commandLine->setSelectedItemIndex(0);
    commandLine->addListener(this);

    setSize(600, 400);
  }

  virtual ~WorkUnitArgumentsComponent()
    {deleteAllChildren();}
  
  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged == commandLine)
    {
      resultString = commandLine->getText();
      argumentList->setValuesFromCommandLine(resultString);
    }
  }

  enum {commandLineHeight = 20};

  virtual void resized()
  {
    int h = getHeight() - 2 * commandLineHeight;
    viewport->setBounds(0, 0, getWidth(), h);
    if (argumentList)
      argumentList->setSize(viewport->getMaximumVisibleWidth(), argumentList->getHeight());
    commandLineLabel->setBounds(0, h, getWidth(), commandLineHeight);
    h += commandLineHeight;
    commandLine->setBounds(0, h, getWidth(), commandLineHeight);
  }

  juce_UseDebuggingNewOperator

private:
  WorkUnitArgumentsViewport* viewport;
  WorkUnitArgumentListComponent* argumentList;
  Label* commandLineLabel;
  ComboBox* commandLine;

  RecentWorkUnitConfigurationPtr workUnit;
  String& resultString;
};

class NewWorkUnitContentComponent : public Component, public juce::ComboBoxListener
{
public:
  NewWorkUnitContentComponent(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& workUnitParameters)
    : argumentsSelector(NULL), recent(recent), workUnitName(workUnitName), workUnitParameters(workUnitParameters)
  {
    addAndMakeVisible(workUnitSelectorLabel = new Label(T("selector"), T("Work Unit")));
    workUnitSelectorLabel->setFont(Font(18, Font::italic | Font::bold));
    addAndMakeVisible(workUnitSelector = new WorkUnitSelectorComboxBox(recent, workUnitName));
    addAndMakeVisible(argumentsSelectorLabel = new Label(T("selector"), T("Arguments")));
    argumentsSelectorLabel->setFont(Font(18, Font::italic | Font::bold));

    workUnitSelector->addListener(this);
    if (workUnitSelector->getNumItems())
      workUnitSelector->setSelectedItemIndex(0);
  }

  virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged)
  {
    if (comboBoxThatHasChanged->getName() == T("WorkUnit"))
    {
      RecentWorkUnitConfigurationPtr workUnit = recent->getWorkUnit(comboBoxThatHasChanged->getText());
      if (argumentsSelector)
      {
        removeChildComponent(argumentsSelector);
        deleteAndZero(argumentsSelector);
      }
      TypePtr type = getType(workUnit->getWorkUnitName());
      if (type && type->getObjectNumVariables() > 0)
      {
        addAndMakeVisible(argumentsSelector = new WorkUnitArgumentsComponent(workUnit, workUnitParameters));
        argumentsSelectorLabel->setText(T("Arguments"), false);
      }
      else
        argumentsSelectorLabel->setText(T("No arguments"), false);
      resized();
    }
  }

  virtual void resized()
  {
    int x = 2 * margin;
    int w = getWidth() - 4 * margin;
    int h = 0;
    workUnitSelectorLabel->setBounds(x, 0, w, labelsHeight);
    h += labelsHeight + margin;
    workUnitSelector->setBounds(x, h, w, workUnitSelectorHeight);
    h += workUnitSelectorHeight + 2 * margin;
    argumentsSelectorLabel->setBounds(x, h, w, labelsHeight);
    h += labelsHeight + 2 * margin;
    if (argumentsSelector)
      argumentsSelector->setBounds(x, h, w, getHeight() - h - 60);
  }

  virtual void paint(Graphics& g)
  {
  //  float y = 2 * labelsHeight + workUnitSelectorHeight + 4 * margin - 2;
  //  g.setColour(Colours::black.withAlpha(0.4f));
  //  g.drawLine(0.f, y, (float)getWidth(), y);
  }

  enum {workUnitSelectorHeight = 20, labelsHeight = 25, margin = 5};

private:
  Label* workUnitSelectorLabel;
  WorkUnitSelectorComboxBox* workUnitSelector;
  Label* argumentsSelectorLabel;
  WorkUnitArgumentsComponent* argumentsSelector;

  RecentWorkUnitsConfigurationPtr recent;
  String& workUnitName;
  String& workUnitParameters;
};


}; /* namespace lbcpp */

/*
** NewWorkUnitDialogWindow
*/
NewWorkUnitDialogWindow::NewWorkUnitDialogWindow(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory)
  : juce::DocumentWindow(T("New Work Unit"), Colour(250, 252, 255), DocumentWindow::maximiseButton | DocumentWindow::closeButton, true)
    //workUnitSelector(new WorkUnitSelectorComboxBox(recent, workUnitName)),
    //argumentsSelector(new WorkUnitArgumentsComponent(recent, arguments))
{
  setResizable(true, true);
  centreWithSize(800, 600);
  setContentComponent(new NewWorkUnitContentComponent(recent, workUnitName, arguments));
/*  
  addTextBlock(T("Work Unit:"));
  addCustomComponent(workUnitSelector);
  addTextBlock(T("Arguments:"));
  addCustomComponent(argumentsSelector);
  //addTextBlock(T("Working directory:"));
  //addCustomComponent(&workingDirectorySelector);
  addButton(T("OK"), 1, juce::KeyPress::returnKey);
  addButton(T("Cancel"), 0, juce::KeyPress::escapeKey);

  workUnitSelector->addListener(argumentsSelector);
  workUnitSelector->addListener(&workingDirectorySelector);
  if (workUnitSelector->getNumItems())
    workUnitSelector->setSelectedItemIndex(0);*/
}
/*
NewWorkUnitDialogWindow::~NewWorkUnitDialogWindow()
{
  deleteAndZero(workUnitSelector);
  deleteAndZero(argumentsSelector);
}
*/
bool NewWorkUnitDialogWindow::run(RecentWorkUnitsConfigurationPtr recent, String& workUnitName, String& arguments, File& workingDirectory)
{
  NewWorkUnitDialogWindow* window = new NewWorkUnitDialogWindow(recent, workUnitName, arguments, workingDirectory);
  const int result = window->runModalLoop();
  delete window;
  return result == 1;
}
/*
void NewWorkUnitDialogWindow::resized()
{
  //argumentsSelector->setSize(argumentsSelector->getWidth(), getHeight() - 100);
  AlertWindow::resized();
}
*/