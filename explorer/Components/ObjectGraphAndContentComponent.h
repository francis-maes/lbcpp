/*-----------------------------------------.---------------------------------.
| Filename: ObjectGraphAndContentComponent.h| Object Graph And Content       |
| Author  : Francis Maes                   |                                 |
| Started : 15/06/2009 13:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_OBJECT_GRAPH_AND_CONTENT_H_
# define EXPLORER_COMPONENTS_OBJECT_GRAPH_AND_CONTENT_H_

# include "ObjectGraphComponent.h"
# include "StringComponent.h"
# include "TableComponent.h"
# include "../Utilities/SplittedLayout.h"

namespace lbcpp
{

class ObjectComponentContainer : public Component
{
public:
  ObjectComponentContainer() : component(NULL) {}
  virtual ~ObjectComponentContainer()
    {deleteAllChildren();}
  
  void setObject(ObjectPtr object, bool topLevelComponent = false)
  {
    deleteAllChildren();
    if (object)
    {
      component = createComponentForObject(object, topLevelComponent);
      addAndMakeVisible(component);
      component->setBoundsRelative(0, 0, 1, 1);
    }
    else
      component = NULL;
    this->object = object;
  }
  
  ObjectPtr getObject() const
    {return object;}
  
  virtual void resized()
  {
    if (component)
      component->setBoundsRelative(0, 0, 1, 1);
  }
  
private:
  Component* component;
  ObjectPtr object;
};

class ObjectGraphAndContentComponent : public SplittedLayout
{
public:
  struct GraphComponent : public ObjectGraphComponent
  {
    GraphComponent(ObjectGraphPtr graph)
      : ObjectGraphComponent(graph) {}
    
    virtual void selectionChanged(int selectedNode)
      {((ObjectGraphAndContentComponent* )getParentComponent())->selectionChanged(selectedNode >= 0 ? layouter.getNode(selectedNode) : ObjectPtr());}
  };
  
  ObjectGraphAndContentComponent(ObjectGraphPtr graph)
    : SplittedLayout(new GraphComponent(graph), new ObjectComponentContainer(), 0.5, typicalHorizontal), graph(graph) {}
  
  void selectionChanged(ObjectPtr selected)
    {((ObjectComponentContainer* )second)->setObject(selected);}
  
private:
  ObjectGraphPtr graph;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_GRAPH_AND_CONTENT_H_

