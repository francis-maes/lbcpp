/*-----------------------------------------.---------------------------------.
| Filename: ObjectEditor.h                | Base class for object editors   |
| Author  : Francis Maes                   |                                 |
| Started : 27/01/2011 20:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_OBJECT_EDITOR_H_
# define LBCPP_USER_INTERFACE_OBJECT_EDITOR_H_

# include "VariableSelector.h"
# include "ComponentWithPreferedSize.h"

namespace lbcpp
{

using juce::Graphics;
using juce::Component;
using juce::Colours;

class BooleanButtonsComponent : public Component, public juce::ButtonListener, public juce::ChangeBroadcaster
{
public:
  struct ConfigurationButton : public juce::ToggleButton
  {
    ConfigurationButton(const String& name, bool& value, const juce::Colour& colour = juce::Colours::black)
      : ToggleButton(name), value(value)
    {
      setColour(textColourId, colour);
      setToggleState(value, false);
    }
      
    bool& value;
  };

  virtual ~BooleanButtonsComponent()
    {deleteAllChildren();}

  virtual void buttonClicked(juce::Button* button)
  {
    ((ConfigurationButton* )button)->value = button->getToggleState();
    sendChangeMessage(this);
  }

  virtual void paint(Graphics& g)
    {g.fillAll(juce::Colour(240, 245, 250));}

  void initialize()
  {
    for (size_t i = 0; i < buttons.size(); ++i)
      for (size_t j = 0; j < buttons[i].size(); ++j)
      {
        ConfigurationButton* button = buttons[i][j];
        button->addButtonListener(this);
        addAndMakeVisible(button);
      }
  }


  virtual void resized()
  {
    for (size_t i = 0; i < buttons.size(); ++i)
    {
      int x = (int)i * buttonWidth;
      for (size_t j = 0; j < buttons[i].size(); ++j)
        buttons[i][j]->setBounds(x, (int)j * buttonHeight, buttonWidth, buttonHeight);
    }
  }

  enum {buttonWidth = 200, buttonHeight = 20};

protected:
  std::vector< std::vector<ConfigurationButton* > > buttons;

  void addToggleButton(std::vector<ConfigurationButton* >& buttonsColumn, const String& name, bool& state, size_t columnsHeight, const juce::Colour& colour = juce::Colours::black)
  {
    buttonsColumn.push_back(new ConfigurationButton(name, state, colour));
    if (buttonsColumn.size() >= columnsHeight)
      flushButtons(buttonsColumn);
  }

  void flushButtons(std::vector<ConfigurationButton* >& buttonsColumn)
  {
    if (buttonsColumn.size())
    {
      buttons.push_back(buttonsColumn);
      buttonsColumn.clear();
    }
  }
};

class SplittedLayout : public Component, public ComponentWithPreferedSize
{
public:
  SplittedLayout(Component* first, Component* second, double defaultRatio, int flags)
    : layoutflags(flags), first(first), second(second), defaultRatio(defaultRatio)
  {
    if (first)
      addAndMakeVisible(first);
    layout.setItemLayout(0, 10, -1, -defaultRatio);
    if ((layoutflags & noResizeBar) != 0)
      resizebar = new Component();
    else
      resizebar = new juce::StretchableLayoutResizerBar(&layout, 1, (layoutflags & vertical) == 0);
    addAndMakeVisible(resizebar);
    double s = (layoutflags & miniResizeBar) != 0 ? 2 : 8;
    layout.setItemLayout(1, s, s, s);
    if (second)
      addAndMakeVisible(second);
    layout.setItemLayout(2, 10, -1, defaultRatio - 1);
  }

  virtual ~SplittedLayout()
  {
    if ((layoutflags & deleteChildrens) != 0)
      deleteAllChildren();
    else
    {
      removeChildComponent(resizebar);
      delete resizebar;
    }
  }

  enum
  {
    horizontal = 0x00,
    vertical = 0x01,
    noResizeBar = 0x02,
    miniResizeBar = 0x04,
    deleteChildrens = 0x08,
    borderPaintOverChildren = 0x10,

    typicalVertical = vertical | deleteChildrens | miniResizeBar,
    typicalHorizontal = horizontal | deleteChildrens | miniResizeBar,
    defaultForToolbars = vertical | miniResizeBar | deleteChildrens,
  };

  virtual void resized()
  {
    Component* comps[] = {first, resizebar, second};
    layout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), (layoutflags & vertical) != 0, true);
  }

  virtual void paintOverChildren(Graphics& g)
  {
    if ((layoutflags & borderPaintOverChildren) != 0)
    {
      g.setColour(Colours::black);
      if ((layoutflags & vertical) != 0)
      {
        g.drawLine(0.f, (float)first->getBottom() - 1, (float)getWidth(), (float)first->getBottom() - 1);
        g.drawLine(0.f, (float)second->getY(), (float)getWidth(), (float)second->getY());
      }
      else
      {
        g.drawLine((float)first->getRight() - 1, 0.f, (float)first->getRight() - 1, (float)getHeight());
        g.drawLine((float)second->getX(), 0.f, (float)second->getX(), (float)getHeight());
      }
    }
  }
  
  void changeFirstComponent(Component* newComponent)
  {
    if (first)
      removeChildComponent(first), delete first;
    addAndMakeVisible(first = newComponent);
    resized();
  }

  void changeSecondComponent(Component* newComponent)
  {
    if (second)
      removeChildComponent(second), delete second;
    addAndMakeVisible(second = newComponent);
    resized();
  }
  
  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {return isHorizontal() ? computePreferedSize(availableWidth, availableHeight) : availableWidth;}

  virtual int getPreferedHeight(int availableWidth, int availableHeight) const
    {return isVertical() ? computePreferedSize(availableHeight, availableWidth) : availableHeight;}

  int computePreferedSize(int availableSize1, int availableSize2) const
  {
    int prefered1 = getSubPreferedSize(first, (int)(availableSize1 * defaultRatio), availableSize2);
    int prefered2 = getSubPreferedSize(second, (int)(availableSize1 * (1.0 - defaultRatio)), availableSize2);
    return prefered1 + 8 + prefered2;
  }

  int getSubPreferedSize(Component* c, int availableSize1, int availableSize2) const
  {
    ComponentWithPreferedSize* cs = dynamic_cast<ComponentWithPreferedSize* >(c);
    return cs 
      ? (isHorizontal() ? cs->getPreferedWidth(availableSize1, availableSize2) : cs->getPreferedHeight(availableSize2, availableSize1))
      : availableSize1;
  }

  bool isHorizontal() const
    {return (layoutflags & horizontal) == horizontal;}

  bool isVertical() const
    {return (layoutflags & vertical) == vertical;}

  juce::StretchableLayoutManager layout;

  juce_UseDebuggingNewOperator

protected:
  int        layoutflags;
  Component* first;
  Component* resizebar;
  Component* second;
  double defaultRatio;
};

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

}; /* namespace lbcpp */

#endif // !LBCPP_USER_INTERFACE_COMPONENTS_CONTAINER_CURVE_EDITOR_H_
