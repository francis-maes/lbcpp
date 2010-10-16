/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.cpp              | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/DynamicObject.h>
#include <lbcpp/Data/XmlSerialisation.h>
#include "Object/SparseGenericObject.h"
#include "Object/DenseGenericObject.h"
#include "Object/DenseObjectObject.h"
#include "Object/DenseDoubleObject.h"
using namespace lbcpp;

DynamicClass::~DynamicClass()
{
}

bool DynamicClass::initialize(MessageCallback& callback)
{
  if (!getObjectNumVariables())
    createObjectVariables();
  return DefaultClass::initialize(callback);
}

VariableValue DynamicClass::create() const
  {return createDenseObject();}

ObjectPtr DynamicClass::createDenseObject() const
{
  bool hasOnlyDoubles = true;
  bool hasOnlyObjects = true;
  for (size_t i = 0; i < variables.size(); ++i)
  {
    if (!variables[i].first->inheritsFrom(doubleType))
      hasOnlyDoubles = false;
    if (!variables[i].first->inheritsFrom(objectClass))
      hasOnlyObjects = false;
  }
  if (hasOnlyDoubles)
    return new DenseDoubleObject(nativePointerFromThis(this));
  else if (hasOnlyObjects)
    return new DenseObjectObject(nativePointerFromThis(this));
  else
    return new DenseGenericObject(nativePointerFromThis(this));
}

ObjectPtr DynamicClass::createSparseObject() const
  {return new SparseGenericObject(nativePointerFromThis(this));}

Variable DynamicClass::getObjectVariable(const Object* pthis, size_t index) const
  {jassert(pthis); return pthis->getVariable(index);}

void DynamicClass::setObjectVariable(Object* pthis, size_t index, const Variable& subValue) const
{
  if (checkInheritance(subValue.getType(), getObjectVariableType(index)))
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
