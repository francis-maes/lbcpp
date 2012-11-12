/*-----------------------------------------.---------------------------------.
| Filename: ObjectBrowser.cpp              | The Object Browser              |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 15:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ObjectBrowser.h"
#include "../ExplorerProject.h"
#include <lbcpp/Data/Table.h>
#include <lbcpp/UserInterface/UserInterfaceManager.h>
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

class ObjectBrowserResizerBar : public Component
{
public:
  ObjectBrowserResizerBar()
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

struct ObjectRelatedCommand
{
  ObjectRelatedCommand(const String& name, const String& iconToUse)
    : name(name), iconToUse(iconToUse) {}

  String name;
  String iconToUse;

  static std::vector<ObjectRelatedCommand> getObjectCommands(const ObjectPtr& object)
  {
    std::vector<ObjectRelatedCommand> res;
    res.push_back(ObjectRelatedCommand(T("Save"), T("Save-32.png")));
    if (object.isInstanceOf<Table>())
      res.push_back(ObjectRelatedCommand(T("Save as gnuplot"), T("Save-32.png")));
    return res;
  }

  static void execute(const ObjectPtr& object, const ObjectRelatedCommand& command)
  {
    if (command.name == T("Save"))
    {
      File outputFile = selectFileToSave(T("*.*"));
      if (outputFile != File::nonexistent)
        object->saveToFile(defaultExecutionContext(), outputFile);
    }
    else if (command.name == T("Save as gnuplot"))
    {
      File outputFile = selectFileToSave(T("*.data"));
      if (outputFile != File::nonexistent)
        saveContainerAsGnuplotData(defaultExecutionContext(), object.staticCast<Container>(), outputFile);
    }
  }

  static bool saveContainerAsGnuplotData(ExecutionContext& context, const ContainerPtr& container, const File& outputFile)
  {
    size_t numRows = container->getNumElements();
    ClassPtr rowType = container->getElementsType();

    if (outputFile.existsAsFile())
      outputFile.deleteFile();

    OutputStream* ostr = outputFile.createOutputStream();
    if (!ostr)
    {
      context.errorCallback(T("Could not create file ") + outputFile.getFullPathName());
      return false;
    }

    // make columns
    std::vector<size_t> columns;
    columns.reserve(rowType->getNumMemberVariables());
    for (size_t i = 0; i < rowType->getNumMemberVariables(); ++i)
      if (rowType->getMemberVariableType(i)->isConvertibleToDouble())
        columns.push_back(i);

    // write header
    *ostr << "# lbcpp-explorer gnu plot file\n";
    *ostr << "#";
    for (size_t i = 0; i < columns.size(); ++i)
      *ostr << " " << rowType->getMemberVariableName(columns[i]);
    *ostr << "\n\n";

    // write data
    for (size_t i = 0; i < numRows; ++i)
    {
      ObjectPtr object = container->getElement(i);
      for (size_t j = 0; j < columns.size(); ++j)
        *ostr << String(object->getVariable(columns[j])->toDouble()) << " ";
      *ostr << "\n";
    }
    ostr->flush();
    delete ostr;
    return true;
  }

  static File selectFileToSave(const String& extension)
  {
    File defaultDirectory = File::getSpecialLocation(File::userHomeDirectory);
    ExplorerProjectPtr project = ExplorerProject::getCurrentProject();
    if (project)
      defaultDirectory = project->getRootDirectory();

    FileChooser chooser("Select the file to save...", defaultDirectory, extension);
    if (chooser.browseForFileToSave(true))
      return chooser.getResult();
    else
      return File::nonexistent;
  }
};


class ObjectBrowserRowHeaderComponent : public Component, public juce::ButtonListener
{
public:
  ObjectBrowserRowHeaderComponent(const ObjectPtr& object, Component* selector)
    : object(object)
  {
    addAndMakeVisible(properties = new PropertyListDisplayComponent(40));
    bool isTabbedSelector = (dynamic_cast<ObjectSelectorTabbedButtonBar* >(selector) != NULL);
    if (isTabbedSelector)
      return;

    // properties
    properties->addProperty(T("Type"), object->getClassName());
    String str = object->toShortString();
    if (str.isNotEmpty())
      properties->addProperty(T("Desc"), str);
    ContainerPtr container = object.dynamicCast<Container>();
    if (container)
      properties->addProperty(T("Size"), String((int)container->getNumElements()));
    
    // command buttons
    commands = ObjectRelatedCommand::getObjectCommands(object);
    buttons.resize(commands.size());
    for (size_t i = 0; i < buttons.size(); ++i)
    {
      juce::ImageButton* button = new juce::ImageButton(commands[i].name);
      juce::Image* image = userInterfaceManager().getImage(commands[i].iconToUse);
      button->setImages(true, false, true, image, 1.0f, Colours::transparentBlack, NULL, 1.0f, Colours::white.withAlpha(0.2f), NULL, 1.0f, Colours::black.withAlpha(0.2f));
      button->addButtonListener(this);
      addAndMakeVisible(buttons[i] = button);
    }
  }

  virtual ~ObjectBrowserRowHeaderComponent()
    {deleteAllChildren();}

  virtual void resized()
  {
    int x = getWidth();
    for (int i = (int)buttons.size() - 1; i >= 0; --i)
    {
      int w = buttons[i]->getWidth();
      x -= (w + 5);
      buttons[i]->setBounds(x, 0, w, getHeight());
    }
    properties->setBounds(0, 0, x, getHeight());
  }

  virtual void buttonClicked(juce::Button* button)
  {
    for (size_t i = 0; i < buttons.size(); ++i)
      if (buttons[i] == button)
      {
        ObjectRelatedCommand::execute(object, commands[i]);
        break;
      }
  }

  virtual void paint(Graphics& g)
    {g.fillAll(Colour(240, 245, 250));}

private:
  ObjectPtr object;
  PropertyListDisplayComponent* properties;
  std::vector<juce::Button* > buttons;
  std::vector<ObjectRelatedCommand> commands;
};

class ObjectBrowserRowComponent : public Component, public ComponentWithPreferedSize
{
public:
  ObjectBrowserRowComponent(const ObjectPtr& object, Component* selector)
    : object(object), selector(selector), resizer(NULL)
  {
    bool isResizeable = (dynamic_cast<ObjectSelectorTabbedButtonBar* >(selector) == NULL);

    addAndMakeVisible(header = new ObjectBrowserRowHeaderComponent(object, selector));
    addAndMakeVisible(selector);
    if (isResizeable)
      addAndMakeVisible(resizer = new ObjectBrowserResizerBar());
  }

  virtual ~ObjectBrowserRowComponent()
    {deleteAllChildren();}

  enum
  {
    headerHeight = 36,
    resizerWidth = 4
  };

  virtual void resized()
  {
    header->setBounds(0, 0, getWidth() - (resizer ? resizerWidth : 0), headerHeight);
    selector->setBounds(0, headerHeight, getWidth() - (resizer ? resizerWidth : 2), getHeight() - headerHeight);
    if (resizer)
      resizer->setBounds(getWidth() - resizerWidth, 0, resizerWidth, getHeight());
  }

  virtual void paint(Graphics& g)
    {g.fillAll(Colours::white);}

  virtual void paintOverChildren(Graphics& g)
  {
    g.setColour(Colours::lightgrey);
    g.drawLine(0.f, (float)headerHeight, (float)getWidth(), (float)headerHeight);
  }

  Component* getSelectorComponent() const
    {return selector;}

  ObjectSelector* getSelector() const
    {return dynamic_cast<ObjectSelector* >(selector);}

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
      res = getPreferedWidth(availableWidth - resizerWidth, availableHeight - headerHeight);
      res += resizerWidth;
    }
    return res;
  }

  ObjectPtr getObject() const
    {return object;}

private:
  ObjectPtr object;
  ObjectBrowserRowHeaderComponent* header;
  Component* selector;
  ObjectBrowserResizerBar* resizer;
};

class ObjectBrowserContent : public Component, public ComponentWithPreferedSize, public ObjectSelectorCallback
{
public:
  ObjectBrowserContent()
  {
  }

  virtual ~ObjectBrowserContent()
    {clear();}

  void clear()
  {
    rows.clear();
    deleteAllChildren();
    repaint();
  }

  void setRootObject(const ObjectPtr& object, Component* selector)
    {clear(); appendObject(object, selector);}

  void appendObject(const ObjectPtr& object, Component* selector)
  {
    ObjectSelector* s = dynamic_cast<ObjectSelector* >(selector);
    if (s)
      s->addCallback(*this);
    ObjectBrowserRowComponent* rowComponent = new ObjectBrowserRowComponent(object, selector);
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
    getParentComponent()->resized();
    //setSize(getPreferedWidth(), getHeight());
  }

  virtual int getPreferedWidth(int availableWidth, int availableHeight) const
    {int w = getPreferedWidth(); return w ? w : availableWidth;}

  virtual void selectionChangedCallback(ObjectSelector* selector, const std::vector<ObjectPtr>& selectedObjects, const String& selectionName)
  {
    int rowNumber = findRowNumber(selector);
    jassert(rowNumber >= 0);
    for (int i = (int)rows.size() - 1; i > rowNumber; --i)
    {
      removeChildComponent(rows[i].first);
      delete rows[i].first;
      rows.erase(rows.begin() + i);
    }
    if (selectedObjects.size())
    {
      ObjectPtr object = lbcpp::createSelectionObject(selectedObjects);
      Component* component = selector->createComponentForObject(defaultExecutionContext(), object, selectionName);
      if (!component)
        component = lbcpp::createComponentForObject(defaultExecutionContext(), object, selectionName);
      if (component)
        appendObject(object, component);

      //setSize(getPreferedWidth(), getHeight());
      Viewport* viewport = findParentComponentOfClass<Viewport>();
      if (viewport)
      {
        viewport->setViewPositionProportionately(1.0, 0.0);
        viewport->resized();
      }

      flushErrorAndWarningMessages(T("Changed Selection"));
    }
  }

private:
  std::vector<std::pair<ObjectBrowserRowComponent*, int> > rows;

  int findRowNumber(ObjectSelector* selector) const
  {
    for (size_t i = 0; i < rows.size(); ++i)
      if (rows[i].first->getSelector() == selector)
        return (int)i;
    return -1;
  }
};

/*
** ObjectBrowser
*/
ObjectBrowser::ObjectBrowser(const ObjectPtr& object, Component* selector)
  : ViewportComponent(new ObjectBrowserContent(), false, true)
{
  getContent()->setRootObject(object, selector);
}

void ObjectBrowser::resized()
{
  ObjectBrowserContent* content = getContent();

  if (content->getWidth() > getWidth())
    content->setSize(content->getWidth(), getHeight() - getScrollBarThickness());
  else
    content->setSize(getWidth(), getHeight());
}

ObjectBrowserContent* ObjectBrowser::getContent() const
  {return (ObjectBrowserContent* )getViewedComponent();}
