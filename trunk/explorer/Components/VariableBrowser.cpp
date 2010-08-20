/*-----------------------------------------.---------------------------------.
| Filename: VariableBrowser.cpp            | The Variable Browser            |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 15:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "VariableBrowser.h"
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

class VariableBrowserResizerBar : public Component
{
public:
  VariableBrowserResizerBar()
  {
    setRepaintsOnMouseActivity(true);
    setMouseCursor(juce::MouseCursor(juce::MouseCursor::LeftRightResizeCursor));
  }

  virtual void paint(Graphics& g)
  {
    g.fillAll(Colour(240, 245, 250));
    float x = getWidth() / 2.f;
    float thickness = isMouseOver() ? 2.f : 1.f;
    g.setColour(isMouseButtonDown() ? Colours::grey : Colours::lightgrey);
    g.drawLine(x, 0.f, x, (float)getHeight(), thickness);
  }

  virtual void mouseDown(const MouseEvent& )
    {mouseDownWidth = getParentComponent()->getWidth();}

  virtual void mouseDrag(const MouseEvent& e)
  {
    int desiredWidth = juce::jmax(10, mouseDownWidth + e.getDistanceFromDragStartX());
    Component* parent = getParentComponent();
    jassert(parent);
    parent->setSize(desiredWidth, parent->getHeight());
  }

  juce_UseDebuggingNewOperator

private:
  int mouseDownWidth;
};

class VariableBrowserRowComponent : public Component, public ComponentWithPreferedSize
{
public:
  VariableBrowserRowComponent(const Variable& variable, Component* selector)
    : variable(variable), selector(selector)
  {
    properties = new PropertyListDisplayComponent(40);
    if (!dynamic_cast<TabbedVariableSelectorComponent* >(selector))
    {
      properties->addProperty(T("Type"), variable.getTypeName());
      properties->addProperty(T("Desc"), variable.getShortSummary());
      if (variable.size())
        properties->addProperty(T("Size"), String((int)variable.size()));
    }
    addAndMakeVisible(properties);
    addAndMakeVisible(selector);
    addAndMakeVisible(resizer = new VariableBrowserResizerBar());
  }

  virtual ~VariableBrowserRowComponent()
    {deleteAllChildren();}

  enum
  {
    propertiesHeight = 36,
    resizerWidth = 4
  };

  virtual void resized()
  {
    int w = getWidth() - resizerWidth;
    properties->setBounds(0, 0, w, propertiesHeight);
    selector->setBounds(0, propertiesHeight, w, getHeight() - propertiesHeight);
    resizer->setBounds(w, 0, resizerWidth, getHeight());
  }

  virtual void paint(Graphics& g)
    {g.fillAll(Colours::white);}

  virtual void paintOverChildren(Graphics& g)
  {
    g.setColour(Colours::lightgrey);
    g.drawLine(0.f, (float)propertiesHeight, (float)getWidth(), (float)propertiesHeight);
  }

  Component* getSelectorComponent() const
    {return selector;}

  VariableSelector* getSelector() const
    {return dynamic_cast<VariableSelector* >(selector);}

  virtual int getDefaultWidth() const
  {
    ComponentWithPreferedSize* pf = dynamic_cast<ComponentWithPreferedSize* >(selector);
    int w = pf ? pf->getDefaultWidth() : 200;
    return w + resizerWidth;
  }

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
  {
    int res = availableWidth;
    ComponentWithPreferedSize* pf = dynamic_cast<ComponentWithPreferedSize* >(selector);
    if (pf)
    {
      res = getPreferedWidth(availableWidth - resizerWidth, availableHeight - propertiesHeight);
      res += resizerWidth;
    }
    return res;
  }

  Variable getVariable() const
    {return variable;}

private:
  Variable variable;
  PropertyListDisplayComponent* properties;
  Component* selector;
  VariableBrowserResizerBar* resizer;
};

class VariableBrowserContent : public Component, public ComponentWithPreferedSize, public VariableSelectorCallback
{
public:
  VariableBrowserContent()
  {
  }

  virtual ~VariableBrowserContent()
    {clear();}

  void clear()
  {
    rows.clear();
    deleteAllChildren();
    repaint();
  }

  void setRootVariable(const Variable& variable, Component* selector)
    {clear(); appendVariable(variable, selector);}

  void appendVariable(const Variable& variable, Component* selector)
  {
    VariableSelector* s = dynamic_cast<VariableSelector* >(selector);
    if (s)
      s->addCallback(*this);
    VariableBrowserRowComponent* rowComponent = new VariableBrowserRowComponent(variable, selector);
    rowComponent->setBounds(getPreferedWidth(), 0, rowComponent->getDefaultWidth(), getHeight());
    addAndMakeVisible(rowComponent);
    rows.push_back(std::make_pair(rowComponent, getPreferedWidth()));
    setSize(getPreferedWidth(), getHeight());
  }

  int getPreferedWidth() const
    {return rows.size() ? rows.back().first->getRight() : 0;}

  virtual void resized()
  {
    for (size_t i = 0; i < rows.size(); ++i)
    {
      Component* c = rows[i].first;
      c->setBounds(rows[i].second, 0, c->getWidth(), getHeight());
    }
  }

  virtual void childBoundsChanged(Component* child)
  {
    int x = 0;
    for (size_t i = 0; i < rows.size(); ++i)
    {
      rows[i].second = x;
      Component* c = rows[i].first;
      int w = c->getWidth();
      c->setBounds(x, 0, w, getHeight());
      x += w;
    }
    setSize(getPreferedWidth(), getHeight());
  }

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {int w = getPreferedWidth(); return w ? w : availableWidth;}

  virtual void selectionChangedCallback(VariableSelector* selector, const std::vector<Variable>& selectedVariables)
  {
    int rowNumber = findRowNumber(selector);
    jassert(rowNumber >= 0);
    for (int i = (int)rows.size() - 1; i > rowNumber; --i)
    {
      removeChildComponent(rows[i].first);
      delete rows[i].first;
      rows.erase(rows.begin() + i);
    }
    Variable variable = selector->createMultiSelectionVariable(selectedVariables);
    Component* component = selector->createComponentForVariable(variable, variable.getShortSummary());
    if (component)
      appendVariable(variable, component);

    setSize(getPreferedWidth(), getHeight());
    Viewport* viewport = findParentComponentOfClass<Viewport>();
    if (viewport)
      viewport->setViewPositionProportionately(1.0, 0.0);

    flushErrorAndWarningMessages(T("Changed Selection"));
  }

private:
  std::vector<std::pair<VariableBrowserRowComponent*, int> > rows;

  int findRowNumber(VariableSelector* selector) const
  {
    for (size_t i = 0; i < rows.size(); ++i)
      if (rows[i].first->getSelector() == selector)
        return (int)i;
    return -1;
  }
};

/*
** VariableBrowser
*/
VariableBrowser::VariableBrowser(const Variable& variable, Component* selector)
  : ViewportComponent(new VariableBrowserContent(), false, true)
{
  getContent()->setRootVariable(variable, selector);
}

void VariableBrowser::resized()
{
  VariableBrowserContent* content = getContent();
  content->setSize(juce::jmax(content->getWidth(), getWidth()), getHeight());
}

VariableBrowserContent* VariableBrowser::getContent() const
  {return (VariableBrowserContent* )getViewedComponent();}
