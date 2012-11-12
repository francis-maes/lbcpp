/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponent.h              | Base classes for ui components  |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_
# define LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_

# include "../common.h"
# include "../Core.h"

namespace lbcpp
{

using juce::Graphics;
using juce::Component;
using juce::Colours;

class ComponentWithPreferedSize
{
public:
  virtual ~ComponentWithPreferedSize() {}

  virtual int getDefaultWidth() const
    {return getPreferedWidth(0, 0);}

  virtual int getDefaultHeight() const
    {return getPreferedHeight(0, 0);}

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return availableWidth;}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return availableHeight;}
};

class ViewportComponent : public juce::Viewport
{
public:
  ViewportComponent(Component* component = NULL, bool showVerticalScrollbarIfNeeded = true, bool showHorizontalScrollbarIfNeeded = true);

  virtual void resized();
};

class ObjectSelector;
class ObjectSelectorCallback
{
public:
  virtual ~ObjectSelectorCallback() {}

  virtual void selectionChangedCallback(ObjectSelector* selector, const std::vector<ObjectPtr>& selectedObjects, const String& selectionName) = 0;
};

class ObjectSelector
{
public:
  virtual ~ObjectSelector() {}
  
  void addCallback(ObjectSelectorCallback& callback)
    {callbacks.push_back(&callback);}

  void sendSelectionChanged(const ObjectPtr& selectedObject, const String& selectionName)
    {sendSelectionChanged(std::vector<ObjectPtr>(1, selectedObject), selectionName);}

  void sendSelectionChanged(const std::vector<ObjectPtr>& selectedObjects, const String& selectionName)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->selectionChangedCallback(this, selectedObjects, selectionName);
  }

  virtual juce::Component* createComponentForObject(ExecutionContext& context, const ObjectPtr& object, const String& name)
    {return NULL;}

protected:
  std::vector<ObjectSelectorCallback* > callbacks;
};

class ObjectSelectorTabbedButtonBar : public juce::TabbedButtonBar, public ObjectSelector, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  ObjectSelectorTabbedButtonBar(const ObjectPtr& object);

  virtual ObjectPtr getTabSubObject(const ObjectPtr& object, const String& tabName) const;
  virtual void changeListenerCallback(void* objectThatHasChanged);
  virtual int getDefaultWidth() const;
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const;

protected:
  ObjectPtr object;
};

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_OBJECT_COMPONENT_H_
