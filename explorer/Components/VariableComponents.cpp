/*-----------------------------------------.---------------------------------.
| Filename: ObjectComponents.cpp           | Object Components               |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "VariableProxyComponent.h"
#include "ContainerSelectorComponent.h"
#include "StringComponent.h"
#include "VariableTreeComponent.h"
#include "VariableBrowser.h"
#include "HexadecimalFileComponent.h"
#include "../Proteins/ProteinComponent.h"
#include "../Utilities/FileType.h"
using namespace lbcpp;

extern void flushErrorAndWarningMessages(const String& title);

Component* createComponentForObject(ObjectPtr object, const String& explicitName)
{
  String name = explicitName.isEmpty() ? object->getName() : explicitName;

  if (!object)
    return NULL;
  Component* res = object->createComponent();
  if (res)
    return res;

  ContainerPtr container = object.dynamicCast<Container>();
  if (container)
  {
    TypePtr elementsType = container->computeElementsCommonBaseType();
    if (elementsType->inheritsFrom(fileType))
    {
      ContainerPtr loadedContainer = container->apply(loadFromFileFunction(objectClass));
      return createComponentForObject(loadedContainer, explicitName);
    }

    if (elementsType->inheritsFrom(proteinClass))
    {
      std::vector<ProteinPtr> proteins;
      std::vector<String> names;

      size_t n = container->getNumElements();
      proteins.reserve(n);
      names.reserve(n);
      for (size_t i = 0; i < n; ++i)
      {
        ProteinPtr protein = container->getElement(i).getObjectAndCast<Protein>();
        if (protein)
        {
          protein->computeMissingVariables();
          proteins.push_back(protein);
          names.push_back(String((int)i) + T(" - ") + protein->getName());
        }
      }
      return new MultiProteinComponent(proteins, names);      
    }

    return new ContainerSelectorComponent(container);
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
        Variable v = Variable::createFromFile(file);
        return v.exists() ? createComponentForVariableImpl(v, file.getFileName()) : NULL;
      }
    case directory: return new VariableTreeComponent(variable, explicitName, VariableTreeOptions(false, false));
    default: return NULL;
    };
  }

  if (variable.getType() == pairClass(proteinClass, positiveIntegerType))
    return new ResiduePerceptionComponent(variable);

  if (variable.getType() == pairClass(proteinClass, pairClass(positiveIntegerType, positiveIntegerType)))
    return new ResiduePairPerceptionComponent(variable); 

  if (variable.isObject())
  {
    res = createComponentForObject(variable.getObject(), explicitName);
    if (res) return res;
  }

  if (!res)
    res = new VariableTreeComponent(variable, explicitName, VariableTreeOptions(true, true, false)); 
  return res;
}

Component* lbcpp::createComponentForVariable(const Variable& variable, const String& explicitName, bool topLevelComponent)
{
  if (!variable.exists())
    return NULL;
  Component* res = createComponentForVariableImpl(variable, explicitName);
  if (topLevelComponent && dynamic_cast<VariableSelector* >(res))
    res = new VariableBrowser(variable, res);
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
