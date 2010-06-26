/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ObjectProxyComponent.h"
#include "ObjectContainerNameListComponent.h"
#include "StringComponent.h"
#include "TableComponent.h"
#include "TreeComponent.h"
#include "StringToObjectMapTabbedComponent.h"
#include "FileObjectComponent.h"
#include "ObjectBrowser.h"
#include "../Proteins/ProteinComponent.h"
using namespace lbcpp;

class FeatureGeneratorComponent : public ObjectSelectorAndContentComponent
{
public:
  FeatureGeneratorComponent(FeatureGeneratorPtr featureGenerator, const String& name = String::empty)
    : ObjectSelectorAndContentComponent(featureGenerator, new ObjectTreeComponent(featureGenerator, name))
    {}

  virtual void selectionChangedCallback(const std::vector<ObjectPtr>& selectedObjects)
  {
    std::vector<ObjectPtr> objects;
    objects.reserve(selectedObjects.size());
    for (size_t i = 0; i < selectedObjects.size(); ++i)
    {
      FeatureGeneratorPtr featureGenerator = selectedObjects[i].dynamicCast<FeatureGenerator>();
      if (featureGenerator)
        objects.push_back(featureGenerator->toTable());
      else
        objects.push_back(selectedObjects[i]);
    }
    ObjectSelectorAndContentComponent::selectionChangedCallback(objects);
  }
};

class InferenceComponent : public ObjectSelectorAndContentComponent
{
public:
  InferenceComponent(InferencePtr inference, const String& name = String::empty)
    : ObjectSelectorAndContentComponent(inference, new ObjectTreeComponent(inference, name))
    {}
};

Component* createComponentForObjectImpl(ObjectPtr object, const String& explicitName)
{
  String name = explicitName.isEmpty() ? object->getName() : explicitName;

  if (!object)
    return NULL;
  Component* res = object->createComponent();
  if (res)
    return res;

  if (object.dynamicCast<FileObject>())
  {
    FileObjectPtr fileObject = object.staticCast<FileObject>();
    FileObject::Type type = fileObject->getType();
    switch (type)
    {
    case FileObject::directory:
      return new ObjectSelectorAndContentComponent(object, new ObjectTreeComponent(object, name));

    case FileObject::classDirectory:
    case FileObject::classFile:
      {
        ObjectPtr object = Object::createFromFile(fileObject->getFile());
        if (object)
          return createComponentForObjectImpl(object, name);
        else if (type == FileObject::classDirectory)
          return new ObjectSelectorAndContentComponent(object, new ObjectTreeComponent(object, name));
        else
          return new HexadecimalFileObjectComponent(object, name);
      }

    case FileObject::textFile:
      return new StringComponent(object);
    case FileObject::binaryFile:
      return new HexadecimalFileObjectComponent(fileObject, name);
    default:
      jassert(false); return NULL;
    };
  }

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

  if (object.dynamicCast<ProteinObject>())
  {
    ProteinObjectPtr protein = object.staticCast<ProteinObject>();
    protein->computeMissingFields();
    return new ProteinComponent(protein, name);
  }

  if (object.dynamicCast<ObjectPair>())
  {
    ObjectPairPtr pair = object.dynamicCast<ObjectPair>();
    if (pair->getFirst().dynamicCast<ProteinObject>() && 
        pair->getSecond().dynamicCast<ProteinObject>())
    {
      std::vector<std::pair<String, ProteinObjectPtr> > proteins;
      proteins.push_back(std::make_pair(T("Prediction"), pair->getFirst().dynamicCast<ProteinObject>()));
      proteins.push_back(std::make_pair(T("Supervision"), pair->getSecond().dynamicCast<ProteinObject>()));
      return new MultiProteinComponent(proteins);
    }
  }

  if (object.dynamicCast<ObjectContainer>())
  {
    ObjectContainerPtr container = object.staticCast<ObjectContainer>();
    bool areClassFiles = true;
    size_t proteinSize = 0;
    std::vector< std::pair<String, ProteinObjectPtr> > proteins;
    for (size_t i = 0; i < container->size(); ++i)
    {
      ObjectPtr object = container->get(i);
      
      ProteinObjectPtr protein = object.dynamicCast<ProteinObject>();
      if (!protein || (proteinSize && protein->getLength() != proteinSize))
      {
        proteinSize = 0;
        break;
      }
      else
      {
        proteinSize = protein->getLength();
        proteins.push_back(std::make_pair(protein->getName(), protein));
      }
      
      FileObjectPtr fileObject = object.dynamicCast<FileObject>();
      areClassFiles &= fileObject && (fileObject->getType() == FileObject::classFile);
    }
    if (proteinSize)
      // container of proteins => multi protein component
      return new MultiProteinComponent(proteins);
    else if (areClassFiles)
    {
      // container of file objects => container of loaded objects
      VectorObjectContainerPtr objects = new VectorObjectContainer();
      objects->setName(container->getName());
      for (size_t i = 0; i < container->size(); ++i)
      {
        FileObjectPtr fileObject = container->getAndCast<FileObject>(i);
        ObjectPtr object = Object::createFromFile(fileObject->getFile());
        if (object)
        {
          NameableObjectPtr nameableObject = object.dynamicCast<NameableObject>();
          if (nameableObject)
            nameableObject->setName(fileObject->getFile().getFileNameWithoutExtension());
          objects->append(object);
        }
      }
      return createComponentForObjectImpl(objects, explicitName);
    }

    // default for ObjectContainer
    return new ObjectSelectorAndContentComponent(object, new ObjectContainerNameListComponent(object.dynamicCast<ObjectContainer>()));
  }

  if (object.dynamicCast<Table>())
    return new TableComponent(object.dynamicCast<Table>());

  if (object.dynamicCast<StringToObjectMap>())
    return new StringToObjectMapTabbedComponent(object.dynamicCast<StringToObjectMap>());

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

ObjectPtr lbcpp::createMultiSelectionObject(const std::vector<ObjectPtr>& objects)
{
  if (objects.empty())
    return ObjectPtr();
  if (objects.size() == 1)
    return objects[0];
  VectorObjectContainerPtr res = new VectorObjectContainer(objects);
  res->setName(T("MultiSelection"));
  return res;
}
