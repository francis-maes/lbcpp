/*-----------------------------------------.---------------------------------.
| Filename: PlotComponent.h                | Plot Component                  |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_
# define LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_

# include "PlotContentComponent.h"
# include "PlotConfigurationComponent.h"

namespace lbcpp
{
  
class ObjectEditor : public Component, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  ObjectEditor(const ObjectPtr& object, const ObjectPtr& configuration, bool showVerticalScrollbarIfNeeded = true, bool showHorizontalScrollbarIfNeeded = true);
  virtual ~ObjectEditor();
  
  void initialize();

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() * 4 / 10;}

  ViewportComponent* getContentViewport() const
    {return contentViewport;}

  Component* getContentComponent() const
    {return contentViewport->getViewedComponent();}

  virtual void changeListenerCallback(void* objectThatHasChanged);
  virtual void resized();

protected:
  ObjectPtr object;
  ObjectPtr configuration;

  Component* configurationComponent;
  ViewportComponent* contentViewport;

  virtual Component* createConfigurationComponent(const ObjectPtr& configuration) = 0;
  virtual Component* createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration) = 0;
  
  virtual size_t getConfigurationComponentHeight() const
    {return 100;}
};

class PlotComponent : public ObjectEditor
{
public:
  PlotComponent(PlotPtr plot, const String& name);

protected:
  PlotPtr plot;

  virtual Component* createConfigurationComponent(const ObjectPtr& configuration);
  virtual Component* createContentComponent(const ObjectPtr& object, const ObjectPtr& configuration);
};

typedef ReferenceCountedObjectPtr<PlotComponent> PlotComponentPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_
