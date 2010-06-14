/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

//#include "Components/ObjectGraphAndContentComponent.h"
#include "StringComponent.h"
#include "TableComponent.h"
#include "TreeComponent.h"
#include "StringToObjectMapTabbedComponent.h"
using namespace lbcpp;

Component* lbcpp::createComponentForObject(ObjectPtr object, bool topLevelComponent)
{
  if (!object)
    return NULL;
  Component* res = object->createComponent();
  if (res)
    return res;

  if (object.dynamicCast<StringToObjectMap>())
    return new StringToObjectMapTabbedComponent(object.dynamicCast<StringToObjectMap>());

  if (object.dynamicCast<FeatureGenerator>())
    return new ObjectTreeComponent(object);

  //ObjectGraphPtr graph = object->toGraph();
  //if (topLevelComponent && graph)
  //  return new ObjectGraphAndContentComponent(graph);
  TablePtr table = object->toTable();
  if (table)
    return new TableComponent(table);
  return new StringComponent(object);
}
