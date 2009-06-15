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
# include "../Utilities/SplittedLayout.h"

namespace lbcpp
{

class ObjectComponentContainer : public Component
{
public:
  void setObject(ObjectPtr object)
  {
    deleteAllChildren();
    if (object)
    {
      Label* tmp = new Label("pouet", object->getName().c_str());
      tmp->setBoundsRelative(0, 0, 1, 1);
      addAndMakeVisible(tmp);
    }
  }
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
    : SplittedLayout(new GraphComponent(graph), new ObjectComponentContainer(), 0.7, typicalHorizontal), graph(graph) {}
  
  void selectionChanged(ObjectPtr selected)
  {
    ((ObjectComponentContainer* )second)->setObject(selected);
  }
  
private:
  ObjectGraphPtr graph;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_GRAPH_AND_CONTENT_H_

