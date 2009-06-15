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
  
};

class ObjectGraphAndContentComponent : public SplittedLayout
{
public:
  ObjectGraphAndContentComponent(ObjectGraphPtr graph)
    : SplittedLayout(new ObjectGraphComponent(graph), new ObjectComponentContainer(), 0.7, typicalHorizontal) {}
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_OBJECT_GRAPH_AND_CONTENT_H_

