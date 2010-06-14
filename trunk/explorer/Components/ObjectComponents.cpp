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
#include "ObjectBrowser.h"
using namespace lbcpp;

class FeatureGeneratorComponent : public ObjectSelectorAndContentComponent
{
public:
  FeatureGeneratorComponent(FeatureGeneratorPtr featureGenerator, const String& name = String::empty)
    : ObjectSelectorAndContentComponent(featureGenerator, new ObjectTreeComponent(featureGenerator, name))
    {}

  virtual void objectSelectedCallback(ObjectPtr selected)
  {
    FeatureGeneratorPtr featureGenerator = selected.dynamicCast<FeatureGenerator>();
    jassert(featureGenerator);
    ObjectSelectorAndContentComponent::objectSelectedCallback(featureGenerator->toTable());
  }
};

class InferenceComponent : public ObjectSelectorAndContentComponent
{
public:
  InferenceComponent(InferencePtr inference, const String& name = String::empty)
    : ObjectSelectorAndContentComponent(inference, new ObjectTreeComponent(inference, name))
    {}
/*
  virtual void objectSelectedCallback(ObjectPtr selected)
  {
    InferencePtr subInference = selected.dynamicCast<Inference>();
    jassert(subInference);
    ObjectSelectorAndContentComponent::objectSelectedCallback(subInference->toString());
  }*/
};

Component* lbcpp::createComponentForObject(ObjectPtr object, const String& explicitName, bool topLevelComponent)
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

  if (object.dynamicCast<Inference>())
  {
    if (topLevelComponent)
      return new ObjectBrowser(new InferenceComponent(object.dynamicCast<Inference>(), name));

    if (object.dynamicCast<ParameterizedInference>())
    {
      ParameterizedInferencePtr inference = object.staticCast<ParameterizedInference>();
      return createComponentForObject(inference->getParameters(), inference->getName());
    }
    else
      return new InferenceComponent(object.dynamicCast<Inference>(), name);
  }

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
