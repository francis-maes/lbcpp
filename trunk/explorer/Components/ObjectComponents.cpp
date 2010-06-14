/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ObjectComponentContainer.h"
#include "StringComponent.h"
#include "TableComponent.h"
#include "TreeComponent.h"
#include "StringToObjectMapTabbedComponent.h"
using namespace lbcpp;


class FeatureGeneratorComponent : public ObjectSelectorAndContentComponent
{
public:
  FeatureGeneratorComponent(FeatureGeneratorPtr featureGenerator, const String& name = String::empty)
    : ObjectSelectorAndContentComponent(new ObjectTreeComponent(featureGenerator, name))
  {
  }

  virtual void objectSelectedCallback(ObjectPtr selected)
  {
    FeatureGeneratorPtr featureGenerator = selected.dynamicCast<FeatureGenerator>();
    jassert(featureGenerator);
    ObjectSelectorAndContentComponent::objectSelectedCallback(featureGenerator->toTable());
  }
};

Component* lbcpp::createComponentForObject(ObjectPtr object, const String& explicitName)
{
  String name = explicitName.isEmpty() ? object->getName() : explicitName;

  if (!object)
    return NULL;
  Component* res = object->createComponent();
  if (res)
    return res;

  if (object.dynamicCast<StringToObjectMap>())
    return new StringToObjectMapTabbedComponent(object.dynamicCast<StringToObjectMap>());

  if (object.dynamicCast<FeatureGenerator>())
    return new FeatureGeneratorComponent(object.dynamicCast<FeatureGenerator>(), name);

  if (object.dynamicCast<Table>())
    return new TableComponent(object.dynamicCast<Table>());

  //ObjectGraphPtr graph = object->toGraph();
  //if (topLevelComponent && graph)
  //  return new ObjectGraphAndContentComponent(graph);
  TablePtr table = object->toTable();
  if (table)
    return new TableComponent(table);
  return new StringComponent(object);
}
