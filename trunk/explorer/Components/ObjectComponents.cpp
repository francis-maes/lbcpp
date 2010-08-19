/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "VariableProxyComponent.h"
#include "ObjectContainerNameListComponent.h"
#include "StringComponent.h"
#include "TableComponent.h"
#include "TreeComponent.h"
#include "StringToObjectMapTabbedComponent.h"
#include "FileObjectComponent.h"
#include "VariableBrowser.h"
#include "../Proteins/ProteinComponent.h"
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

class FeatureGeneratorComponent : public VariableSelectorAndContentComponent
{
public:
  FeatureGeneratorComponent(FeatureGeneratorPtr featureGenerator, const String& name = String::empty)
    : VariableSelectorAndContentComponent(featureGenerator, new ObjectTreeComponent(featureGenerator, name))
    {}

  virtual void selectionChangedCallback(const std::vector<Variable>& selectedVariables)
  {
    std::vector<Variable> variables;
    variables.reserve(selectedVariables.size());
    for (size_t i = 0; i < selectedVariables.size(); ++i)
    {
      FeatureGeneratorPtr featureGenerator = selectedVariables[i].getObjectAndCast<FeatureGenerator>();
      if (featureGenerator)
        variables.push_back(featureGenerator->toTable());
      else
        variables.push_back(selectedVariables[i]);
    }
    VariableSelectorAndContentComponent::selectionChangedCallback(variables);
  }
};

class InferenceComponent : public VariableSelectorAndContentComponent
{
public:
  InferenceComponent(InferencePtr inference, const String& name = String::empty)
    : VariableSelectorAndContentComponent(inference, new ObjectTreeComponent(inference, name))
    {}
};

Component* createComponentForObject(ObjectPtr object, const String& explicitName)
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
      return new VariableSelectorAndContentComponent(object, new ObjectTreeComponent(fileObject, name));

    case FileObject::classDirectory:
    case FileObject::classFile:
      {
        ObjectPtr object = Object::createFromFile(fileObject->getFile());
        if (object)
          return createComponentForVariable(object, name);
        else if (type == FileObject::classDirectory)
          return new VariableSelectorAndContentComponent(object, new ObjectTreeComponent(fileObject, name));
        else
          return new HexadecimalFileObjectComponent(object, name);
      }

    case FileObject::textFile:
        return new StringComponent(object);

    case FileObject::xmlFile:
      {
        Variable variable = Variable::createFromFile(fileObject->getFile());
        flushErrorAndWarningMessages(T("Load file ") + fileObject->getFile().getFileName());
        return createComponentForVariable(variable, name);
      }

    case FileObject::binaryFile:
      return new HexadecimalFileObjectComponent(fileObject, name);
    default:
      jassert(false); return NULL;
    };
  }
  
  if (object.dynamicCast<Protein>())
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    protein->computeMissingVariables();
    return new ProteinComponent(protein, name);
  }

  return NULL;
}

Component* createComponentForVariableImpl(const Variable& variable, const String& explicitName)
{
  if (variable.isNil())
    return NULL;
  if (variable.isMissingValue())
    return NULL;

  Component* res = NULL;
  if (variable.isObject())
    res = createComponentForObject(variable.getObject(), explicitName);

  if (!res)
    res = new VariableTreeComponent(variable, explicitName); 
  return res;
}

/*
  if (object.dynamicCast<FeatureGenerator>())
    return new FeatureGeneratorComponent(object.dynamicCast<FeatureGenerator>(), name);

  if (object.dynamicCast<Inference>())
  {
    if (object.dynamicCast<NumericalInference>())
    {
      NumericalInferencePtr inference = object.staticCast<NumericalInference>();
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
      return createComponentForVariableImpl(objects, explicitName);
    }

    // default for ObjectContainer
    return new VariableSelectorAndContentComponent(object, new ObjectContainerNameListComponent(object.dynamicCast<ObjectContainer>()));
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
 */

Component* lbcpp::createComponentForVariable(const Variable& variable, const String& explicitName, bool topLevelComponent)
{
  if (!variable)
    return NULL;
  Component* res = createComponentForVariableImpl(variable, explicitName);
  if (topLevelComponent)
  {
    VariableSelectorAndContentComponent* selector = dynamic_cast<VariableSelectorAndContentComponent* >(res);
    if (selector)
      res = new VariableBrowser(selector);
  }
  return res;
}

Variable lbcpp::createMultiSelectionVariable(const std::vector<Variable>& variables)
{
  if (variables.empty())
    return Variable();
  if (variables.size() == 1)
    return variables[0];
  VectorPtr res = new Vector(objectClass());
  res->reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
    res->append(variables[i]);
  //res->setName(T("MultiSelection"));
  return res;
}
