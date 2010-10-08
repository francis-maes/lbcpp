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
#include "Object/DenseDoubleObject.h"
using namespace lbcpp;

VariableValue DynamicClass::create() const
  {return createDenseObject();}

ObjectPtr DynamicClass::createDenseObject() const
{
  bool hasOnlyDoubles = true;
  for (size_t i = 0; i < variables.size(); ++i)
    if (!variables[i].first->inheritsFrom(doubleType()))
    {
      hasOnlyDoubles = false;
      break;
    }
  if (hasOnlyDoubles)
    return new DenseDoubleObject(refCountedPointerFromThis(this));
  else
    return new DenseGenericObject(refCountedPointerFromThis(this));
}

ObjectPtr DynamicClass::createSparseObject() const
  {return new SparseGenericObject(refCountedPointerFromThis(this));}

Variable DynamicClass::getObjectVariable(const VariableValue& value, size_t index) const
{
  ObjectPtr object = value.getObject();
  jassert(object);
  return object->getVariable(index);
}

void DynamicClass::setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const
{
  if (checkInheritance(subValue.getType(), getObjectVariableType(index)))
  {
    ObjectPtr object = value.getObject();
    jassert(object);
    object->setVariable(index, subValue);
  }
}

void DynamicClass::saveToXml(XmlExporter& exporter) const
{
  exporter.setAttribute(T("className"), getName());
}

bool DynamicClass::loadFromXml(XmlImporter& importer)
{ 
  setName(importer.getStringAttribute(T("className")));
  return true;
}
