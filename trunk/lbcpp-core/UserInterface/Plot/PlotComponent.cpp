/*-----------------------------------------.---------------------------------.
| Filename: PlotComponent.cpp              | Plot Component                  |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "PlotComponent.h"
using namespace lbcpp;


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

/*
** PlotComponent
*/
PlotComponent::PlotComponent(PlotPtr plot, const String& name)
  : ObjectEditor(plot, plot, false, false), plot(plot)
{
  initialize();
}

Component* PlotComponent::createConfigurationComponent(const ObjectPtr& plot)
  {return new PlotConfigurationComponent(plot.staticCast<Plot>());}

Component* PlotComponent::createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration)
  {return new PlotContentComponent(configuration.staticCast<Plot>());}
