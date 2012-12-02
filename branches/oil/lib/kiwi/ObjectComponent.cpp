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
** ObjectSelectorTabbedButtonBar
*/
ObjectSelectorTabbedButtonBar::ObjectSelectorTabbedButtonBar(const ObjectPtr& object)
  : TabbedButtonBar(TabsAtLeft), object(object)
  {addChangeListener(this);}

ObjectPtr ObjectSelectorTabbedButtonBar::getTabSubObject(const ObjectPtr& object, const string& tabName) const
  {return object;}

void ObjectSelectorTabbedButtonBar::changeListenerCallback(void* objectThatHasChanged)
{
  string tabName = getCurrentTabName();
  sendSelectionChanged(getTabSubObject(object, tabName), tabName);
}

int ObjectSelectorTabbedButtonBar::getDefaultWidth() const
  {return 27;}

int ObjectSelectorTabbedButtonBar::getPreferedWidth(int availableWidth, int availableHeight) const
  {return 27;}
