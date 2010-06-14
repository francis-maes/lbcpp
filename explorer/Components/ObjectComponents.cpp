/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ObjectComponentContainer.h"
#include "ObjectContainerNameListComponent.h"
#include "StringComponent.h"
#include "TableComponent.h"
#include "TreeComponent.h"
#include "StringToObjectMapTabbedComponent.h"
#include "ObjectBrowser.h"
#include "../Proteins/ProteinComponent.h"
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

Component* createComponentForObjectImpl(ObjectPtr object, const String& explicitName)
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
    if (object.dynamicCast<ParameterizedInference>())
    {
      ParameterizedInferencePtr inference = object.staticCast<ParameterizedInference>();
      return createComponentForObject(inference->getParameters(), inference->getName());
    }
    else
      return new InferenceComponent(object.dynamicCast<Inference>(), name);
  }

  if (object.dynamicCast<Protein>())
    return new ProteinComponent(object.dynamicCast<Protein>(), name);

  if (object.dynamicCast<ObjectPair>())
  {
    ObjectPairPtr pair = object.dynamicCast<ObjectPair>();
    if (pair->getFirst().dynamicCast<Protein>() && 
        pair->getSecond().dynamicCast<Protein>())
    {
      std::vector<std::pair<String, ProteinPtr> > proteins;
      proteins.push_back(std::make_pair(T("Input"), pair->getFirst().dynamicCast<Protein>()));
      proteins.push_back(std::make_pair(T("Supervision"), pair->getSecond().dynamicCast<Protein>()));
      return new MultiProteinComponent(proteins);
    }
  }

  if (object.dynamicCast<ObjectContainer>())
  {
    

    return new ObjectSelectorAndContentComponent(object, new ObjectContainerNameListComponent(object.dynamicCast<ObjectContainer>()));
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

Component* lbcpp::createComponentForObject(ObjectPtr object, const String& explicitName, bool topLevelComponent)
{
  Component* res = createComponentForObjectImpl(object, explicitName);
  if (topLevelComponent)
  {
    ObjectSelectorAndContentComponent* selector = dynamic_cast<ObjectSelectorAndContentComponent* >(res);
    if (selector)
      res = new ObjectBrowser(selector);
  }
  return res;
}
