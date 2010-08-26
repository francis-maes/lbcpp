/*-----------------------------------------.---------------------------------.
| Filename: DynamicObject.cpp              | Dynamic Object                  |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/DynamicObject.h>
#include <lbcpp/Data/XmlSerialisation.h>
using namespace lbcpp;

/*
** DynamicObject
*/
DynamicObject::DynamicObject(TypePtr thisType)
  : Object(thisType) {}

DynamicObject::~DynamicObject()
{
  for (size_t i = 0; i < variableValues.size(); ++i)
    thisClass->getObjectVariableType(i)->destroy(variableValues[i]);
}

VariableValue& DynamicObject::operator[](size_t index)
{
  jassert(index < thisClass->getObjectNumVariables());
  if (variableValues.size() <= index)
  {
    size_t i = variableValues.size();
    variableValues.resize(index + 1);
    while (i < variableValues.size())
    {
      variableValues[i] = thisClass->getObjectVariableType(i)->getMissingValue();
      ++i;
    }
  }
  return variableValues[index];
}

/*
** DynamicClass
*/
VariableValue DynamicClass::create() const
  {return new DynamicObject(refCountedPointerFromThis(this));}

Variable DynamicClass::getObjectVariable(const VariableValue& value, size_t index) const
{
  DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
  jassert(object);
  return Variable::copyFrom(getObjectVariableType(index), (*object)[index]);
}

void DynamicClass::setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const
{
  jassert(subValue.getType()->inheritsFrom(getObjectVariableType(index)));
  DynamicObjectPtr object = value.getObjectAndCast<DynamicObject>();
  jassert(object);
  subValue.copyTo((*object)[index]);
}

void DynamicClass::saveToXml(XmlExporter& exporter) const
{
  ScopedLock _(variablesLock);

  exporter.enter(T("class"));

  exporter.setAttribute(T("name"), getName());
  exporter.setAttribute(T("base"), getBaseType()->getName().replaceCharacters(T("<>"), T("[]")));
  for (size_t i = 0; i < variables.size(); ++i)
  {
    TypePtr type = variables[i].first;
    exporter.enter(T("variable"));
    exporter.setAttribute(T("name"), variables[i].second);
    if (type.dynamicCast<DynamicClass>())
      exporter.saveVariable(T("type"), type);
    else
      exporter.setAttribute(T("type"), type->getName().replaceCharacters(T("<>"), T("[]")));
    exporter.leave();
  }
  exporter.leave();
}

bool DynamicClass::loadFromXml(XmlElement* xml, MessageCallback& callback)
{
  ScopedLock _(variablesLock);
  variables.clear();
  XmlElement* classXml = xml->getChildByName(T("class"));
  if (!classXml)
  {
    callback.errorMessage(T("DynamicClass::loadFromXml"), T("Could not find class element"));
    return false;
  }
  setName(classXml->getStringAttribute(T("className")));
  baseType = Type::get(classXml->getStringAttribute(T("classBase"), T("???")));
  if (!baseType)
    return false;

  forEachXmlChildElementWithTagName(*classXml, elt, T("variable"))
  {
    TypePtr type = Type::get(elt->getStringAttribute(T("type"), T("???")).replaceCharacters(T("[]"), T("<>")), callback);
    if (!type)
      return false;
    String name = elt->getStringAttribute(T("name"), T("???"));
    variables.push_back(std::make_pair(type, name));
  }
  return false;
}
