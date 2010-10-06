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
using namespace lbcpp;

VariableValue DynamicClass::create() const
  {return createDenseObject();}

ObjectPtr DynamicClass::createDenseObject() const
  {return new DenseGenericObject(refCountedPointerFromThis(this));}

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

  /*exporter.enter(T("class"));

  exporter.setAttribute(T("name"), getName());
  exporter.setAttribute(T("base"), getBaseType()->getName().replaceCharacters(T("<>"), T("[]")));
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr type = variables[i].first;
    exporter.enter(T("variable"));
    exporter.setAttribute(T("name"), variables[i].second);
    exporter.writeType(type);
    exporter.leave();
  }
  exporter.leave();*/
}

bool DynamicClass::loadFromXml(XmlImporter& importer)
{ 
  setName(importer.getStringAttribute(T("className")));
  return true;
 
  /*variables.clear();
  if (!importer.enter(T("class")))
    return false;
  setName(importer.getStringAttribute(T("name")));
  baseType = Type::get(importer.getStringAttribute(T("base"), T("???")), importer.getCallback());
  if (!baseType)
    return false;

  bool res = true;
  forEachXmlChildElementWithTagName(*importer.getCurrentElement(), elt, T("variable"))
  {
    importer.enter(elt);
    TypePtr type = importer.loadType();
    if (type)
    {
      String name = elt->getStringAttribute(T("name"), T("???"));
      variables.push_back(std::make_pair(type, name));
    }
    else
      res = false;
    importer.leave();
  }

  importer.leave();
  return res;*/
  return true;
}
