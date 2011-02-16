/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.cpp              | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/DynamicObject.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include "Object/SparseGenericObject.h"
#include "Object/SparseDoubleObject.h"
#include "Object/DenseGenericObject.h"
#include "Object/DenseObjectObject.h"
#include "Object/DenseDoubleObject.h"
using namespace lbcpp;

DynamicClass::~DynamicClass()
{
}

void DynamicClass::ensureVariablesTypeIsComputed()
{
  if (variablesType == uncomputedVariableTypes)
    computeVariablesType();
}

void DynamicClass::computeVariablesType()
{
  size_t n = getNumMemberVariables();
  if (n)
  {
    bool hasOnlyDoubles = true;
    bool hasOnlyObjects = true;
    for (size_t i = 0; i < n; ++i)
    {
      TypePtr type = getMemberVariableType(i);
      if (!type->inheritsFrom(doubleType))
        hasOnlyDoubles = false;
      if (!type->inheritsFrom(objectClass))
        hasOnlyObjects = false;
    }
    if (hasOnlyDoubles)
      variablesType = onlyDoubleVariables;
    else if (hasOnlyObjects)
      variablesType = onlyObjectVariables;
    else
      variablesType = mixedVariableTypes;
  }
}

bool DynamicClass::initialize(ExecutionContext& context)
{
  if (!getNumMemberVariables())
    createObjectVariables();
  computeVariablesType();
  return DefaultClass::initialize(context);
}

ObjectPtr DynamicClass::createDenseObject() const
{
  jassert(getNumMemberVariables());
  const_cast<DynamicClass* >(this)->ensureVariablesTypeIsComputed();
  if (variablesType == onlyDoubleVariables)
    return new DenseDoubleObject(refCountedPointerFromThis(this));
  else if (variablesType == onlyObjectVariables)
    return new DenseObjectObject(refCountedPointerFromThis(this));
  else
    return new DenseGenericObject(refCountedPointerFromThis(this));
}

ObjectPtr DynamicClass::createSparseObject() const
{ 
  jassert(variables.size());
  const_cast<DynamicClass* >(this)->ensureVariablesTypeIsComputed();
  if (variablesType == onlyDoubleVariables)
    return new SparseDoubleObject(refCountedPointerFromThis(this));
  else
    return new SparseGenericObject(refCountedPointerFromThis(this));
}

Variable DynamicClass::getMemberVariableValue(const Object* pthis, size_t index) const
  {jassert(pthis); return pthis->getVariable(index);}

void DynamicClass::setMemberVariableValue(Object* pthis, size_t index, const Variable& subValue) const
{
  if (checkInheritance(subValue.getType(), getMemberVariableType(index)))
  {
    jassert(pthis);
    pthis->setVariable(index, subValue);
  }
}

bool DynamicClass::loadFromXml(XmlImporter& importer)
{ 
  if (!DefaultClass::loadFromXml(importer))
    return false;
  createObjectVariables();
  return true;
}
