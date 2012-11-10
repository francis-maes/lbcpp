/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponent.cpp            | Base class for object editors   |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/UserInterface/ObjectComponent.h>
using namespace lbcpp;

/*
** ViewportComponent
*/
ViewportComponent::ViewportComponent(Component* component, bool showVerticalScrollbarIfNeeded, bool showHorizontalScrollbarIfNeeded)
{
  setScrollBarsShown(showVerticalScrollbarIfNeeded, showHorizontalScrollbarIfNeeded);
  if (component)
    setViewedComponent(component);
}

void ViewportComponent::resized()
{
  Component* content = getViewedComponent();
  if (content)
  {
    ComponentWithPreferedSize* c = dynamic_cast<ComponentWithPreferedSize* >(content);
    int w = getMaximumVisibleWidth();
    int h = getMaximumVisibleHeight();
    if (c)
      content->setSize(c->getPreferedWidth(w, h), c->getPreferedHeight(w, h));
    else
      content->setSize(w, h);
  }
  juce::Viewport::resized();
}  

/*
** TabbedVariableSelectorComponent
*/
TabbedVariableSelectorComponent::TabbedVariableSelectorComponent(const Variable& variable)
  : TabbedButtonBar(TabsAtLeft), variable(variable)
  {addChangeListener(this);}

Variable TabbedVariableSelectorComponent::getSubVariable(const Variable& variable, const String& tabName) const
  {return variable;}

void TabbedVariableSelectorComponent::changeListenerCallback(void* objectThatHasChanged)
{
  String tabName = getCurrentTabName();
  sendSelectionChanged(getSubVariable(variable, tabName), tabName);
}

int TabbedVariableSelectorComponent::getDefaultWidth() const
  {return 27;}

int TabbedVariableSelectorComponent::getPreferedWidth(int availableWidth, int availableHeight) const
  {return 27;}

/*
** BooleanButtonsComponent
*/
BooleanButtonsComponent::~BooleanButtonsComponent()
  {deleteAllChildren();}

void BooleanButtonsComponent::buttonClicked(juce::Button* button)
{
  ((ConfigurationButton* )button)->value = button->getToggleState();
  sendChangeMessage(this);
}

void BooleanButtonsComponent::paint(Graphics& g)
  {g.fillAll(juce::Colour(240, 245, 250));}

void BooleanButtonsComponent::initialize()
{
  for (size_t i = 0; i < buttons.size(); ++i)
    for (size_t j = 0; j < buttons[i].size(); ++j)
    {
      ConfigurationButton* button = buttons[i][j];
      button->addButtonListener(this);
      addAndMakeVisible(button);
    }
}

void BooleanButtonsComponent::resized()
{
  for (size_t i = 0; i < buttons.size(); ++i)
  {
    int x = (int)i * buttonWidth;
    for (size_t j = 0; j < buttons[i].size(); ++j)
      buttons[i][j]->setBounds(x, (int)j * buttonHeight, buttonWidth, buttonHeight);
  }
}

void BooleanButtonsComponent::addToggleButton(std::vector<ConfigurationButton* >& buttonsColumn, const String& name, bool& state, size_t columnsHeight, const juce::Colour& colour)
{
  buttonsColumn.push_back(new ConfigurationButton(name, state, colour));
  if (buttonsColumn.size() >= columnsHeight)
    flushButtons(buttonsColumn);
}

void BooleanButtonsComponent::flushButtons(std::vector<ConfigurationButton* >& buttonsColumn)
{
  if (buttonsColumn.size())
  {
    buttons.push_back(buttonsColumn);
    buttonsColumn.clear();
  }
}

/*
** ObjectEditor
*/
ObjectEditor::ObjectEditor(const ObjectPtr& object, const ObjectPtr& configuration, bool showVerticalScrollbarIfNeeded, bool showHorizontalScrollbarIfNeeded)
  : object(object), configuration(configuration), configurationComponent(NULL)
{
  addAndMakeVisible(contentViewport = new ViewportComponent(NULL, showVerticalScrollbarIfNeeded, showHorizontalScrollbarIfNeeded));
}

ObjectEditor::~ObjectEditor()
  {deleteAllChildren();}

void ObjectEditor::initialize()
{
  jassert(configuration);
  configurationComponent = createConfigurationComponent(configuration);
  addAndMakeVisible(configurationComponent);
  juce::ChangeBroadcaster* changeBroadcaster = dynamic_cast<juce::ChangeBroadcaster* >(configurationComponent);
  if (changeBroadcaster)
    changeBroadcaster->addChangeListener(this);
  changeListenerCallback(NULL);
}

void ObjectEditor::changeListenerCallback(void* objectThatHasChanged)
{
  Component* component = createContentComponent(object, configuration);
  contentViewport->setViewedComponent(component);
  contentViewport->resized();
}

void ObjectEditor::resized()
{
  size_t configurationHeight = getConfigurationComponentHeight();
  if (configurationComponent)
    configurationComponent->setBounds(0, 0, getWidth(), configurationHeight);
  contentViewport->setBounds(0, configurationHeight, getWidth(), getHeight() - configurationHeight);
}
