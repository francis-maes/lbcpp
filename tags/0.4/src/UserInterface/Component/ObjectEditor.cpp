/*-----------------------------------------.---------------------------------.
| Filename: ObjectEditor.cpp               | Base class for object editors   |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/UserInterface/ObjectEditor.h>
using namespace lbcpp;

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
  enum {configurationHeight = 100};
  if (configurationComponent)
    configurationComponent->setBounds(0, 0, getWidth(), configurationHeight);
  contentViewport->setBounds(0, configurationHeight, getWidth(), getHeight() - configurationHeight);
}
