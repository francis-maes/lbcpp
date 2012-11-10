/*-----------------------------------------.---------------------------------.
| Filename: PlotComponent.h                | Plot Component                  |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_
# define LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_

# include "PlotConfigurationComponent.h"
# include "PlotContentComponent.h"

namespace lbcpp
{

class PlotComponent : public Component, public juce::ChangeListener, public ComponentWithPreferedSize
{
public:
  PlotComponent(PlotPtr plot, const String& name)
    : plot(plot), content(NULL)
  {
    addAndMakeVisible(configuration = new PlotConfigurationComponent(plot));
    configuration->
    addAndMakeVisible(content = new PlotContentComponent(plot));
    configuration->addChangeListener(this);
  }

  virtual ~PlotComponent()
    {deleteAllChildren();}

  virtual int getDefaultWidth() const
    {return juce::Desktop::getInstance().getMainMonitorArea().getWidth() * 4 / 10;}

  virtual void changeListenerCallback(void* objectThatHasChanged)
  {
    removeChildComponent(content);
    delete content;
    addAndMakeVisible(content = new PlotContentComponent(plot));
    resized();
  }

  virtual void resized()
  {
    const size_t h = 100;
    configuration->setBounds(0, 0, getWidth(), h);
    content->setBounds(0, h, getWidth(), getHeight() - h);
  }

protected:
  PlotPtr plot;
  PlotConfigurationComponent* configuration;
  PlotContentComponent* content;
};

typedef ReferenceCountedObjectPtr<PlotComponent> PlotComponentPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_PLOT_COMPONENT_H_
