/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "VariableProxyComponent.h"
#include "ContainerSelectorComponent.h"
#include "StringComponent.h"
#include "VariableBrowser.h"
#include "HexadecimalFileComponent.h"
#include "../Utilities/FileType.h"

using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

Component* createComponentForObject(ExecutionContext& context, ObjectPtr object, const String& explicitName)
{
  String name = explicitName.isEmpty() ? object->getName() : explicitName;

  if (!object)
    return NULL;

  if (object->getClass()->inheritsFrom(xmlElementClass))
    object = object.staticCast<XmlElement>()->createObject(context);

  // old
  Component* res = object->createComponent();
  if (res)
    return res;

  // new
  res = userInterfaceManager().createComponentIfExists(context, object, explicitName);
  if (res)
    return res;

  ContainerPtr container = object.dynamicCast<Container>();
  if (container)
  {
    TypePtr elementsType = container->computeElementsCommonBaseType();
    if (elementsType->inheritsFrom(fileType))
    {
      ContainerPtr loadedContainer = container->apply(context, loadFromFileFunction(objectClass), Container::parallelApply);
      TypePtr loadedElementsType = loadedContainer->computeElementsCommonBaseType();
      if (loadedElementsType != objectClass)
        loadedContainer->setThisClass(containerClass(loadedElementsType));
      return createComponentForObject(context, loadedContainer, explicitName);
    }
  
    if (elementsType->inheritsFrom(sumType(doubleType, integerType)))
      return userInterfaceManager().createVariableTreeView(context, container);
    if (!elementsType->isNamedType() && elementsType->getNumMemberVariables() > 1)
      return userInterfaceManager().createContainerTableListBox(context, container);
    else
      return new ContainerSelectorComponent(container);
  }
/*
  if (object.dynamicCast<Protein>())
  {
    ProteinPtr protein = object.dynamicCast<Protein>();
    return new ProteinComponent(protein, name);
  }*/

  return NULL;
}

Component* createComponentForVariableImpl(ExecutionContext& context, const Variable& variable, const String& explicitName)
{
  if (variable.isNil())
    return NULL;
  if (variable.isMissingValue())
    return NULL;

  Component* res = NULL;

  if (variable.isFile())
  {
    File file = variable.getFile();
    switch (getFileType(file))
    {
    case binaryFile: return new HexadecimalFileComponent(variable, explicitName);
    case xmlFile: 
    case textFile: return new StringComponent(variable); 
    case lbcppXmlFile:
      {
        Variable v = Variable::createFromFile(context, file);
        return v.exists() ? createComponentForVariableImpl(context, v, file.getFileName()) : NULL;
      }
    case directory: return userInterfaceManager().createVariableTreeView(context, variable, explicitName, false, false);
    default: return NULL;
    };
  }

  if (variable.isObject())
  {
    res = createComponentForObject(context, variable.getObject(), explicitName);
    if (res) return res;
  }

  if (!res)
    res = userInterfaceManager().createVariableTreeView(context, variable, explicitName, true, true, false);
  return res;
}

Component* lbcpp::createComponentForVariable(ExecutionContext& context, const Variable& variable, const String& explicitName, bool topLevelComponent)
{
  if (!variable.exists())
    return NULL;
  Component* res = createComponentForVariableImpl(context, variable, explicitName);
  if (topLevelComponent && dynamic_cast<VariableSelector* >(res))
  {
    if (variable.getType()->inheritsFrom(xmlElementClass))
      return createComponentForObject(context, variable.getObjectAndCast<XmlElement>()->createObject(context), explicitName);
    res = new VariableBrowser(variable, res);
  }
  String title = T("Create Component");
  if (explicitName.isNotEmpty())
    title += T(" for ") + explicitName;
  flushErrorAndWarningMessages(title);
  return res;
}

Variable lbcpp::createMultiSelectionVariable(const std::vector<Variable>& variables)
{
  if (variables.empty())
    return Variable();
  if (variables.size() == 1)
    return variables[0];
  TypePtr baseType = variables[0].getType();
  for (size_t i = 0; i < variables.size(); ++i)
    baseType = Type::findCommonBaseType(baseType, variables[i].getType());
  VectorPtr res = vector(baseType);
  res->reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
    res->append(variables[i]);
  return res;
}
